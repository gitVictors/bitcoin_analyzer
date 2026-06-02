#pragma once


#include <string>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp> // Требуется библиотека nlohmann/json

using json = nlohmann::json;

class Config {
private:
    std::string bitcoinApiUrl;
    int networkTimeoutSeconds;
    int zeroCountingMode;
    int maxLagForAutocorrelation;

public:
    Config()
        : bitcoinApiUrl("https://blockchain.info/rawblock/")
        , networkTimeoutSeconds(10)
        , zeroCountingMode(1)
        , maxLagForAutocorrelation(50) {
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
                config.bitcoinApiUrl = j["bitcoin_api_url"];
            if (j.contains("network_timeout_seconds"))
                config.networkTimeoutSeconds = j["network_timeout_seconds"];
            if (j.contains("zero_counting_mode"))
                config.zeroCountingMode = j["zero_counting_mode"];
            if (j.contains("max_lag_for_autocorrelation"))
                config.maxLagForAutocorrelation = j["max_lag_for_autocorrelation"];
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing config: " << e.what() << ". Using defaults." << std::endl;
        }

        return config;
    }

    // Getters
    std::string getBitcoinApiUrl() const { return bitcoinApiUrl; }
    int getNetworkTimeoutSeconds() const { return networkTimeoutSeconds; }
    int getZeroCountingMode() const { return zeroCountingMode; }
    int getMaxLagForAutocorrelation() const { return maxLagForAutocorrelation; }
};
