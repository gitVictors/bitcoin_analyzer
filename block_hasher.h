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
#include <iostream>
#include <ios>
#include <iomanip>


class BlockHasher {

private:

    Sha256StepRecorder stepRecorder;
    //std::vector<uint8_t> allStepsFlattened;
    //int zeroCount_;
    int zeroCountingMode;



public:
    BlockHasher(int mode) : zeroCountingMode(mode) {}

    void computeHashWithSteps(const std::vector<uint8_t>& header) {

        stepRecorder.reset();
        
        std::vector<uint8_t> firstHash = stepRecorder.customSha256(header);
        std::vector<uint8_t> secontHash = stepRecorder.customSha256(firstHash);
     
    }

    uint32_t getZeroCount() const {
        return stepRecorder.getZerrotDump();
    }

    void reset() {
        stepRecorder.reset();
   
    }
};
