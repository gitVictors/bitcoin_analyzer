//====================================================================================================================
// Создание 02.06.2026
// 
//
//====================================================================================================================

#pragma once


#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>

class BlockParser {
private:
    std::vector<uint8_t> rawHeader;
    uint32_t nonce;

    void extractNonce() {
        if (rawHeader.size() < 72) {
            throw std::runtime_error("Header too short to extract nonce");
        }
        // Nonce is at bytes 68-71 (little-endian)
        nonce = 0;
        nonce |= rawHeader[68];
        nonce |= (rawHeader[69] << 8);
        nonce |= (rawHeader[70] << 16);
        nonce |= (rawHeader[71] << 24);
    }

public:
    void parseFromRawBlock(const std::vector<uint8_t>& rawBlock) {
        if (rawBlock.size() < 80) {
            throw std::runtime_error("Block too short to contain header");
        }

        rawHeader.assign(rawBlock.begin(), rawBlock.begin() + 80);
        extractNonce();
    }

    uint32_t getNonce() const {
        return nonce;
    }

    void setNonce(uint32_t newNonce) {
        nonce = newNonce;
        // Update header bytes 68-71 with little-endian representation
        rawHeader[68] = nonce & 0xFF;
        rawHeader[69] = (nonce >> 8) & 0xFF;
        rawHeader[70] = (nonce >> 16) & 0xFF;
        rawHeader[71] = (nonce >> 24) & 0xFF;
    }

    const std::vector<uint8_t>& getRawHeader() const {
        return rawHeader;
    }
};

