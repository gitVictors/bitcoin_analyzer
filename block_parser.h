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

        // Note: rawHeader_ and rawBlock_ remain empty when parsing from JSON
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
        // Update header bytes 68-71 with little-endian representation
        if (!rawHeader_.empty()) {
            rawHeader_[68] = nonce_ & 0xFF;
            rawHeader_[69] = (nonce_ >> 8) & 0xFF;
            rawHeader_[70] = (nonce_ >> 16) & 0xFF;
            rawHeader_[71] = (nonce_ >> 24) & 0xFF;
        }
    }

    bool hasRawBlock() const {
        return !rawBlock_.empty();
    }
};