//====================================================================================================================
// Создан: 02.06.2026
// 
//
//====================================================================================================================

#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <nlohmann/json.hpp>

#define BITCOIN_BLOCK  80
#define SHA256_BYTE    32

using json = nlohmann::json;

class BlockParser {
private:
    std::vector<uint8_t> rawHeader_; //заголовок блока Биткойна в бинарном виде (ровно 80 байт) !TODO - их нет
    std::vector<uint8_t> rawBlock_; //полный сырой блок Биткойна (заголовок + все транзакции)
    uint32_t nonce_;
    uint32_t height_;
    std::string blockId_;  //это  хеш

    void extractNonce() {
        if (rawHeader_.size() < 72) {
            throw std::runtime_error("Header too short to extract nonce");
        }
        // Nonce is at bytes 68-71 (little-endian)
        nonce_ = 0;
        nonce_ |= rawHeader_[68];
        nonce_ |= (rawHeader_[69] << 8);
        nonce_ |= (rawHeader_[70] << 16);
        nonce_ |= (rawHeader_[71] << 24);
    }

    // Вспомогательная функция: конвертирует hex-строку в байты и разворачивает их (Big-Endian -> Little-Endian)
    std::vector<uint8_t> hexToLittleEndianBytes(const std::string& hex) {
        std::vector<uint8_t> bytes;
        bytes.reserve(hex.length() / 2);

        // Читаем по два символа
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
            bytes.push_back(byte);
        }

        // Разворачиваем массив байт для Little-Endian
        std::reverse(bytes.begin(), bytes.end());
        return bytes;
    }

    // Вспомогательная функция: копирует число в вектор байт в формате Little-Endian
    template<typename T>
    void appendLittleEndian(std::vector<uint8_t>& buffer, T value) {
        for (size_t i = 0; i < sizeof(T); ++i) {
            buffer.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
        }
    }

    // Основная функция компиляции 80-байтового заголовка
    std::vector<uint8_t> getRawHandler(const json& blockJson) {
        std::vector<uint8_t> header;
        header.reserve(80); // Заголовок всегда равен 80 байтам

        // 1. Version (4 байта)
        int32_t version = blockJson.at("version").get<int32_t>();
        appendLittleEndian(header, version);

        // 2. Previous Block Hash (32 байта)
        std::string prevHash = blockJson.at("previousblockhash").get<std::string>();
        std::vector<uint8_t> prevHashBytes = hexToLittleEndianBytes(prevHash);
        header.insert(header.end(), prevHashBytes.begin(), prevHashBytes.end());

        // 3. Merkle Root (32 байта)
        std::string merkleRoot = blockJson.at("merkle_root").get<std::string>();
        std::vector<uint8_t> merkleRootBytes = hexToLittleEndianBytes(merkleRoot);
        header.insert(header.end(), merkleRootBytes.begin(), merkleRootBytes.end());

        // 4. Timestamp (4 байта)
        uint32_t timestamp = blockJson.at("timestamp").get<uint32_t>();
        appendLittleEndian(header, timestamp);

        // 5. Bits (4 байта)
        uint32_t bits = blockJson.at("bits").get<uint32_t>();
        appendLittleEndian(header, bits);

        // 6. Nonce (4 байта)
        uint32_t nonce = blockJson.at("nonce").get<uint32_t>();
        appendLittleEndian(header, nonce);

        return header;
    }

public:
    void parseFromRawBlock(const std::vector<uint8_t>& rawBlock) {
        rawBlock_ = rawBlock;

        if (rawBlock.size() < SHA256_BYTE) {
            throw std::runtime_error("Block too short to contain header");
        }

        rawHeader_.assign(rawBlock.begin(), rawBlock.begin() + 80);
        extractNonce();
    }

    void parseFromJson(const json& blockJson) {

        // Extract nonce from JSON (as uint32_t)
        if (blockJson.contains("nonce")) {
            // Handle case where nonce might be stored as string or number
            if (blockJson["nonce"].is_number()) {
                nonce_ = blockJson["nonce"].get<uint32_t>();
            }
            else if (blockJson["nonce"].is_string()) {
                nonce_ = static_cast<uint32_t>(std::stoul(blockJson["nonce"].get<std::string>()));
            }
            else {
                throw std::runtime_error("Invalid nonce format in JSON");
            }
        }
        else {
            throw std::runtime_error("JSON does not contain nonce field");
        }

        // Extract height for additional context
        if (blockJson.contains("height")) {
            if (blockJson["height"].is_number()) {
                height_ = blockJson["height"].get<uint32_t>();
            }
            else if (blockJson["height"].is_string()) {
                height_ = static_cast<uint32_t>(std::stoul(blockJson["height"].get<std::string>()));
            }
        }

        // Extract block ID for additional context
        if (blockJson.contains("id")) {
            blockId_ = blockJson["id"].get<std::string>();
        }

        rawHeader_ = getRawHandler(blockJson);

        // Note: rawBlock_ remain empty when parsing from JSON
        // because we don't have the raw binary block data
    }

    uint32_t getNonce() const {
        return nonce_;
    }

    uint32_t getHeight() const {
        return height_;
    }

    std::string getBlockId() const {
        return blockId_;
    }

    const std::vector<uint8_t>& getRawHeader() const {

        if (rawHeader_.empty()) {
            throw std::runtime_error("Raw header not available (parsed from JSON)");
        }
        return rawHeader_;
    }

    const std::vector<uint8_t>& getRawBlock() const {
        if (rawBlock_.empty()) {
            throw std::runtime_error("Raw block not available (parsed from JSON)");
        }
        return rawBlock_;
    }

    void setNonce(uint32_t newNonce) {
        nonce_ = newNonce;

        // Проверяем, что заголовок корректного размера (80 байт)
        if (rawHeader_.size() >= 80) {
            // Поле Nonce находится на позициях 76-79 в формате Little-Endian
            rawHeader_[76] = nonce_ & 0xFF;
            rawHeader_[77] = (nonce_ >> 8) & 0xFF;
            rawHeader_[78] = (nonce_ >> 16) & 0xFF;
            rawHeader_[79] = (nonce_ >> 24) & 0xFF;
        }
    }

    bool hasRawBlock() const {
        return !rawBlock_.empty();
    }
};