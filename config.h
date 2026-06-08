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
    uint32_t nonce_;
    uint32_t nonceStart_;
    uint32_t nonceEnd_;
    uint32_t step_;

public:
    Config()
        : bitcoinApiUrl_("https://blockchain.info/rawblock/")
        , networkTimeoutSeconds_(10)
        , zeroCountingMode_(1)
        , maxLagForAutocorrelation_(50)
        , nonce_(0)
        , nonceStart_ (0)
        , nonceEnd_ (0)
        , step_ (1)
    {
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
            if (j.contains("nonce"))
                config.nonce_ = j["nonce"];
            if (j.contains("nonceStart"))
                config.nonceStart_ = j["nonceStart"];
            if (j.contains("nonceEnd"))
                config.nonceEnd_ = j["nonceEnd"];
            if (j.contains("nonceEnd"))
                config.nonceEnd_ = j["nonceEnd"];
            if (j.contains("step"))
                config.step_ = j["step"];
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
    uint32_t getNonce()const { return nonce_; }
    uint32_t getNonceStart() const { return nonceStart_; }
    uint32_t getNonceEnd() const { return nonceEnd_; }
    uint32_t getStep() const { return  step_; }

};
