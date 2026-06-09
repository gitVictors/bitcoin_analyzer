//====================================================================================================================
// Создание 02.06.2026
// 
//
//====================================================================================================================


#pragma once



#include <vector>
#include <array>
#include <cstdint>
#include <cstring>
#include <bit>
#include <iostream>

class Sha256StepRecorder {

private:
    // SHA-256 constants
    static constexpr uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    static constexpr unsigned int HEADER_SIZE = 80;
    static constexpr uint32_t SECOND_HASH = 32;

    uint32_t h_[8];
    uint8_t buffer_[HEADER_SIZE];
    uint64_t bitCount_;
    size_t bufferIndex_;
    std::vector<std::array<uint8_t, 32>> stepStates_;
    uint32_t zeroDump_;

    void transform(const uint8_t* chunk) {

        uint32_t w[64];
        uint32_t work[8];

        // Prepare message schedule
        for (int i = 0; i < 16; i++) {
            w[i] = (chunk[i * 4] << 24) | (chunk[i * 4 + 1] << 16) | (chunk[i * 4 + 2] << 8) | chunk[i * 4 + 3];
        }
        for (int i = 16; i < 64; i++) {
            uint32_t s0 = (std::rotr(w[i - 15], 7) ^ std::rotr(w[i - 15], 18) ^ (w[i - 15] >> 3));
            uint32_t s1 = (std::rotr(w[i - 2], 17) ^ std::rotr(w[i - 2], 19) ^ (w[i - 2] >> 10));
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        // Initialize working variables
        for (int i = 0; i < 8; i++) {
            work[i] = h_[i];
        }

        // Main compression loop
        for (int i = 0; i < 64; i++) {
            uint32_t S1 = std::rotr(work[4], 6) ^ std::rotr(work[4], 11) ^ std::rotr(work[4], 25);
            uint32_t ch = (work[4] & work[5]) ^ (~work[4] & work[6]);
            uint32_t temp1 = work[7] + S1 + ch + K[i] + w[i];
            uint32_t S0 = std::rotr(work[0], 2) ^ std::rotr(work[0], 13) ^ std::rotr(work[0], 22);
            uint32_t maj = (work[0] & work[1]) ^ (work[0] & work[2]) ^ (work[1] & work[2]);
            uint32_t temp2 = S0 + maj;

            work[7] = work[6];
            work[6] = work[5];
            work[5] = work[4];
            work[4] = work[3] + temp1;
            work[3] = work[2];
            work[2] = work[1];
            work[1] = work[0];
            work[0] = temp1 + temp2;
            //std::cout << std::hex << work[0] << " " << work[1] << " " << work[2] << " " << work[3]
            //    << " " << work[4] << " " << work[5] << " " << work[6] << " " << work[7] << std::endl;
        }

        // Add compressed chunk to hash
        for (int i = 0; i < 8; i++) {
            h_[i] += work[i];
        }
        //std::cout << std::hex << work[0] << " " << work[1] << " " << work[2] << " " << work[3]
        //    << " " << work[4] << " " << work[5] << " " << work[6] << " " << work[7] << std::endl;
    }

    //вспомогательная функция
    // если 0x0f , то ++
    // если 0xf0, то ++
    // если 0x00, то + 2
    void updateZeroDump(uint32_t var) {

        if (var == 0) {
            zeroDump_ += 2;
            return;
        }
        
        if ( (var & 0x0f) == 0)
            zeroDump_++;

        if ( (var & 0xf0) == 0 )
            zeroDump_++;
    }


public:

    Sha256StepRecorder():
         buffer_(0)
        , bitCount_(0)
        , bufferIndex_(0)
        , zeroDump_(0)
    {}

    uint32_t getZerrotDump() const {
        return zeroDump_;
    }

    // Helper function to convert hex string to bytes
    std::vector<uint8_t> hexToBytes(const std::string& hex) {
        std::vector<uint8_t> bytes;
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }

    //Helper
    std::vector<uint8_t> stringToBytes(std::string& str) {
        return std::vector<uint8_t>(str.begin(), str.end());
    }

    // Helper function to compare vectors
    bool compareVectors(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) {
        if (a.size() != b.size()) return false;
        return memcmp(a.data(), b.data(), a.size()) == 0;
    }

    void reset() {
        // Initialize hash values
        h_[0] = 0x6a09e667;
        h_[1] = 0xbb67ae85;
        h_[2] = 0x3c6ef372;
        h_[3] = 0xa54ff53a;
        h_[4] = 0x510e527f;
        h_[5] = 0x9b05688c;
        h_[6] = 0x1f83d9ab;
        h_[7] = 0x5be0cd19;

        bitCount_ = 0;
        bufferIndex_ = 0;
        stepStates_.clear();
        memset(buffer_, 0, 64);
        zeroDump_ = 0;
    }

 

    const std::vector<std::array<uint8_t, 32>>& getStepStates() const {
        return stepStates_;
    }

    //-------------
    
    std::vector<uint8_t> customSha256(const std::vector<uint8_t> input) {

        constexpr uint32_t k[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        uint32_t h[8] = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        };

        uint64_t original_bit_len = input.size() * 8;

        
        std::vector<uint8_t> data = input;
        data.push_back(static_cast<char> (0x80));

        //Добавляем нули до тех пор, пока (длина данных % 64) != 56
        while ((data.size() % 64) != 56) {
            data.push_back(0x00);
        }
       
        // Добавляем длину в битах в big-endian
        for (int i = 7; i >= 0; --i) {
            data.push_back(static_cast<uint8_t>((original_bit_len >> (i * 8)) & 0xFF));
        }

        for (int i = 0; i < data.size(); i += 64) {

            uint32_t w[64] = { 0 };

            for (int t = 0; t < 16; ++t) {
                w[t] = (static_cast<uint32_t>(static_cast<unsigned char>(data[i + t * 4 + 0])) << 24) |
                    (static_cast<uint32_t>(static_cast<unsigned char>(data[i + t * 4 + 1])) << 16) |
                    (static_cast<uint32_t>(static_cast<unsigned char>(data[i + t * 4 + 2])) << 8) |
                    (static_cast<uint32_t>(static_cast<unsigned char>(data[i + t * 4 + 3])) << 0);
            }

            for (int t = 16; t < 64; ++t) {
                uint32_t s0 = (w[t - 15] >> 7 | w[t - 15] << (32 - 7)) ^ (w[t - 15] >> 18 | w[t - 15] << (32 - 18)) ^ (w[t - 15] >> 3);
                uint32_t s1 = (w[t - 2] >> 17 | w[t - 2] << (32 - 17)) ^ (w[t - 2] >> 19 | w[t - 2] << (32 - 19)) ^ (w[t - 2] >> 10);
                w[t] = w[t - 16] + s0 + w[t - 7] + s1;
            }

            uint32_t a = h[0], b = h[1], c = h[2], d = h[3],
                e = h[4], f = h[5], g = h[6], hh = h[7];

            for (int t = 0; t < 64; ++t) {
                uint32_t S1 = (e >> 6 | e << (32 - 6)) ^ (e >> 11 | e << (32 - 11)) ^ (e >> 25 | e << (32 - 25));
                uint32_t ch = (e & f) ^ ((~e) & g);
                uint32_t temp1 = hh + S1 + ch + k[t] + w[t];
                uint32_t S0 = (a >> 2 | a << (32 - 2)) ^ (a >> 13 | a << (32 - 13)) ^ (a >> 22 | a << (32 - 22));
                uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                uint32_t temp2 = S0 + maj;

                hh = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;

                updateZeroDump(a);
                updateZeroDump(b);
                updateZeroDump(c);
                updateZeroDump(d);
                updateZeroDump(e);
                updateZeroDump(f);
                updateZeroDump(hh);

                
                std::cout << std::hex << a << " " << b << " " << c << " " << d << " " << e << " " << f << " " << g << " " << hh << " ";
                std::cout << std::dec << zeroDump_ << std::endl;
                
               
            }

            h[0] += a; h[1] += b; h[2] += c; h[3] += d;
            h[4] += e; h[5] += f; h[6] += g; h[7] += hh;

            for (int i = 0; i < 8; ++i) {
                updateZeroDump(h[i]);
            }
            for (int i = 0; i < 7; ++i) {
                std::cout << std::hex << h[i] << " ";
            }
            std::cout << std::endl;
        }

        std::vector<uint8_t> result;
        for (uint32_t val : h) {
            result.push_back(static_cast<uint8_t>((val >> 24) & 0xFF));
            result.push_back(static_cast<uint8_t>((val >> 16) & 0xFF));
            result.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
            result.push_back(static_cast<uint8_t>((val >> 0) & 0xFF));
        }

        return result;
    }


    //-------------

};


