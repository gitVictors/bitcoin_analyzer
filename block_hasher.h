//====================================================================================================================
// Создание 02.06.2026
// 
//
//
//====================================================================================================================

#pragma once


#include "sha256_step_recorder.h"
#include <vector>
#include <cstdint>

class BlockHasher {
private:
    Sha256StepRecorder stepRecorder;
    std::vector<uint8_t> allStepsFlattened;
    int zeroCount_;
    int zeroCountingMode;

    void flattenAndCountZeros() {

        allStepsFlattened.clear();
        const auto& steps = stepRecorder.getStepStates();

        // Flatten all steps
        for (const auto& step : steps) {
            allStepsFlattened.insert(allStepsFlattened.end(), step.begin(), step.end());
        }

        // Count zeros based on mode
        zeroCount_ = 0;

        if (zeroCountingMode == 1) { // Byte mode
            for (uint8_t byte : allStepsFlattened) {
                if (byte == 0) {
                    zeroCount_++;
                }
            }
        }
        else if (zeroCountingMode == 2) { // 32-bit word mode
            size_t wordCount = allStepsFlattened.size() / 4;
            for (size_t i = 0; i < wordCount; i++) {
                uint32_t word = 0;
                word |= allStepsFlattened[i * 4];
                word |= (allStepsFlattened[i * 4 + 1] << 8);
                word |= (allStepsFlattened[i * 4 + 2] << 16);
                word |= (allStepsFlattened[i * 4 + 3] << 24);
                if (word == 0) {
                    zeroCount_++;
                }
            }
        }
    }

public:
    BlockHasher(int mode) : zeroCountingMode(mode), zeroCount_(0) {}

    void computeHashWithSteps(const std::vector<uint8_t>& header) {

        stepRecorder.reset();

        // First SHA-256 pass
        uint8_t firstHash[32];
        stepRecorder.update(header.data(), header.size());
        stepRecorder.finalize(firstHash);

        // Second SHA-256 pass
        stepRecorder.reset();
        stepRecorder.update(firstHash, 32);
        uint8_t finalHash[32];
        stepRecorder.finalize(finalHash);

        // Flatten and count zeros
        flattenAndCountZeros();
    }

    int getZeroCount() const {
        return zeroCount_;
    }

    void reset() {
        stepRecorder.reset();
        allStepsFlattened.clear();
        zeroCount_ = 0;
    }
};
