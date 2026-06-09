//====================================================================================================================
// Создание 02.06.2026
// 
//
//====================================================================================================================

#pragma once


#include "bitcoin_client.h"
#include "block_parser.h"
#include "block_hasher.h"
#include "autocorrelator.h"
#include "config.h"
#include <iostream>
#include <iomanip>
#include <fstream>

#define ON  1
#define OFF 0
#define ONLY_CONF  ON

class Orchestrator {

private:

    BitcoinClient& bitcoinClient;
    BlockParser parser;
    BlockHasher hasher;
    uint32_t nonceStart_;
    uint32_t nonceEnd_;
    uint32_t nonceStep_;
    std::vector<uint32_t> resultArray_;
    //std::unordered_map<uint32_t, uint64_t > resultArray_;
    int maxLag;

    void interactiveInputBlockHeight(Config config) {
        uint32_t height;

#if ONLY_CONF == 0
        std::cout << "Enter block number: ";
        std::cin >> height;
#else
        std::cout << "Enter block number: ";
        std::cout << config.getNumBlock();
        height = config.getNumBlock();
#endif

        try {
            auto rawBlock = bitcoinClient.fetchBlockByHeight(height);
            parser.parseFromJson(rawBlock);
            std::cout << "Loaded block #" << height << std::endl;
            std::cout << "Original nonce = " << parser.getNonce() << " (0x"
                << std::hex << parser.getNonce() << std::dec << ")" << std::endl;
            std::cout << "Hash = " << "0x" << std::hex << parser.getBlockId() << std::endl;
            parser.outRawHeader();   
        }
        catch (const std::exception& e) {
            std::cerr << "Error loading block: " << e.what() << std::endl;
            throw;
        }
    }

    void interactiveInputNonceRange(Config config) {

#if ONLY_CONF == 0
        std::cout << "\nEnter nonce range:" << std::endl;
        std::cout << "Start value ";
        std::cin >> nonceStart_;
        std::cout << "End value: ";
        std::cin >> nonceEnd_;
        std::cout << "Step value: ";
        std::cin >> nonceStep_;
#else
        std::cout << "\nEnter nonce range:" << std::endl;
        nonceStart_ = config.getNonce();
        std::cout << "Start value " << nonceStart_;
        nonceEnd_ = config.getNonceEnd();
        std::cout << "End value: " << nonceEnd_;
        nonceStep_ = config.getNonceStart();
        std::cout << "Step value: " << nonceStep_;
        
#endif


        if (nonceStep_ == 0) {
            throw std::runtime_error("Step cannot be zero");
        }
        if (nonceStart_ > nonceEnd_) {
            throw std::runtime_error("Start value cannot be greater than end value");
        }
    }

    void processNonceRange() {

        resultArray_.clear();

        size_t totalSteps = ((nonceEnd_ - nonceStart_) / nonceStep_) + 1;
        size_t progressStep = std::max<size_t>(1, totalSteps / 100);

        std::cout << "\nProcessing " << totalSteps << " nonce values..." << std::endl;

        uint32_t currentNonce = nonceStart_;
        size_t processed = 0;

        while (currentNonce <= nonceEnd_) {

           
            parser.setNonce(currentNonce);
            auto header = parser.getRawHeader();
            
            hasher.computeHashWithSteps(header);
            resultArray_.push_back(hasher.getZeroCount());
            hasher.reset();

            processed++;
            if (processed % progressStep == 0) {
                std::cout << "Progress: " << (processed * 100 / totalSteps) << "%\r" << std::flush;
            }

            if (currentNonce > nonceEnd_ - nonceStep_) {
                break;
            }
            currentNonce += nonceStep_;
        }

        std::cout << "Processing complete. " << resultArray_.size() << " values collected." << std::endl;
    }

    void outputResults(const std::vector<double>& autocorr) {
        std::cout << "\n=== Autocorrelation Results ===" << std::endl;
        std::cout << std::fixed << std::setprecision(6);

        for (size_t i = 0; i < autocorr.size(); i++) {
            std::cout << "lag = " << std::setw(4) << i << " : " << autocorr[i] << std::endl;
        }
    }

    void saveResultsToFile(const std::string& filename) {

        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Warning: Could not open file " << filename << " for writing" << std::endl;
            return;
        }

        file << std::fixed << std::setprecision(6);
        file << "# Zero counts for each nonce" << std::endl;
        file << "# Format: nonce_value zero_count" << std::endl;

        uint32_t currentNonce = nonceStart_;
        for (size_t i = 0; i < resultArray_.size(); i++) {
            file << currentNonce << " " << resultArray_[i] << std::endl;
            if (currentNonce <= nonceEnd_ - nonceStep_) {
                currentNonce += nonceStep_;
            }
        }

        file << "\n# Autocorrelation results" << std::endl;
        auto autocorr = Autocorrelator::compute(resultArray_, maxLag);
        for (size_t i = 0; i < autocorr.size(); i++) {
            file << i << " " << autocorr[i] << std::endl;
        }

        std::cout << "Results saved to " << filename << std::endl;
    }

public:
    Orchestrator(BitcoinClient& client, int maxLagConfig, int zeroCountMode)
        : bitcoinClient(client), hasher(zeroCountMode), maxLag(maxLagConfig)
        , nonceStart_(0)
        , nonceEnd_(0)
        , nonceStep_(0)
    {
    }

    void run(const Config config) {

        try {

            interactiveInputBlockHeight(config);
            interactiveInputNonceRange(config);
            processNonceRange();

            if (resultArray_.empty()) {
                std::cerr << "Error: No results generated" << std::endl;
                return;
            }

            auto autocorr = Autocorrelator::compute(resultArray_, maxLag);
            outputResults(autocorr);

            char saveChoice;
            std::cout << "\nSave results to file? (y/n): ";
            std::cin >> saveChoice;
            if (saveChoice == 'y' || saveChoice == 'Y') {
                saveResultsToFile("bitcoin_analysis_results.txt");
            }

        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
};
