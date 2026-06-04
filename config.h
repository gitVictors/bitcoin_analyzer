#pragma once


#include <string>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp> // Требуется библиотека nlohmann/json

using json = nlohmann::json;

class Config {
private:
    std::string bitcoinApiUrl_;
    int networkTimeoutSeconds_;
    int zeroCountingMode_;
    int maxLagForAutocorrelation_;

public:
    Config()
        : bitcoinApiUrl_("https://blockchain.info/rawblock/")
        , networkTimeoutSeconds_(10)
        , zeroCountingMode_(1)
        , maxLagForAutocorrelation_(50) {
    }

    static Config loadFromFile(const std::string& filename) {

        Config config;

        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Warning: Could not open config file. Using defaults." << std::endl;
            return config;
        }

        try {
            json j;
            file >> j;

            if (j.contains("bitcoin_api_url"))
                config.bitcoinApiUrl_ = j["bitcoin_api_url"];
            if (j.contains("network_timeout_seconds"))
                config.networkTimeoutSeconds_ = j["network_timeout_seconds"];
            if (j.contains("zero_counting_mode"))
                config.zeroCountingMode_ = j["zero_counting_mode"];
            if (j.contains("max_lag_for_autocorrelation"))
                config.maxLagForAutocorrelation_ = j["max_lag_for_autocorrelation"];
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing config: " << e.what() << ". Using defaults." << std::endl;
        }

        return config;
    }

    // Getters
    std::string getBitcoinApiUrl() const { return bitcoinApiUrl_; }
    int getNetworkTimeoutSeconds() const { return networkTimeoutSeconds_; }
    int getZeroCountingMode() const { return zeroCountingMode_; }
    int getMaxLagForAutocorrelation() const { return maxLagForAutocorrelation_; }
};
