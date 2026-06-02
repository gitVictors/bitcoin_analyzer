//====================================================================================================================
// Создание 02.06.2026
// 
// autocorrelator.h
//
//====================================================================================================================

#pragma once

#define NOMINMAX

#include <vector>
#include <cmath>
#include <stdexcept>



class Autocorrelator {
public:
    static std::vector<double> compute(const std::vector<int>& data, int maxLag) {
        if (data.empty()) {
            throw std::runtime_error("Empty data array");
        }

        size_t n = data.size();

        // Calculate mean
        double mean = 0.0;
        for (int val : data) {
            mean += val;
        }
        mean /= n;

        // Calculate total variance
        double variance = 0.0;
        for (int val : data) {
            double diff = val - mean;
            variance += diff * diff;
        }

        if (variance == 0.0) {
            throw std::runtime_error("Zero variance in data (all values identical)");
        }

        // Limit maxLag
        size_t actualMaxLag = std::min(static_cast<size_t>(maxLag), n / 2);
        std::vector<double> result(actualMaxLag + 1, 0.0);

        // Compute autocorrelation for each lag
        for (size_t k = 0; k <= actualMaxLag; k++) {
            double sum = 0.0;
            for (size_t i = 0; i < n - k; i++) {
                sum += (data[i] - mean) * (data[i + k] - mean);
            }
            result[k] = sum / variance;
        }

        return result;
    }
};


