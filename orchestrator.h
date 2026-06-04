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

class Orchestrator {
private:
    BitcoinClient& bitcoinClient;
    BlockParser parser;
    BlockHasher hasher;
    uint32_t nonceStart_;
    uint32_t nonceEnd_;
    uint32_t nonceStep_;
    std::vector<int> resultArray;
    int maxLag;

    void interactiveInputBlockHeight() {

        uint32_t height;
        std::cout << "Enter block number: ";
        std::cin >> height;

        try {
            auto rawBlock = bitcoinClient.fetchBlockByHeight(height);
            parser.parseFromJson(rawBlock);
            std::cout << "Loaded block #" << height << std::endl;
            std::cout << "Original nonce = " << parser.getNonce() << " (0x"
                << std::hex << parser.getNonce() << std::dec << ")" << std::endl;
            std::cout << "Hash = " << "0x" << std::hex << parser.getBlockId() << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error loading block: " << e.what() << std::endl;
            throw;
        }
    }

    void interactiveInputNonceRange() {

        std::cout << "\nEnter nonce range:" << std::endl;
        std::cout << "Start value: ";
        std::cin >> nonceStart_;
        std::cout << "End value: ";
        std::cin >> nonceEnd_;
        std::cout << "Step value: ";
        std::cin >> nonceStep_;

        if (nonceStep_ == 0) {
            throw std::runtime_error("Step cannot be zero");
        }
        if (nonceStart_ > nonceEnd_) {
            throw std::runtime_error("Start value cannot be greater than end value");
        }
    }

    void processNonceRange() {
        resultArray.clear();

        size_t totalSteps = ((nonceEnd_ - nonceStart_) / nonceStep_) + 1;
        size_t progressStep = std::max<size_t>(1, totalSteps / 100);

        std::cout << "\nProcessing " << totalSteps << " nonce values..." << std::endl;

        uint32_t currentNonce = nonceStart_;
        size_t processed = 0;

        while (currentNonce <= nonceEnd_) {
            parser.setNonce(currentNonce);
            auto header = parser.getRawHeader();
            hasher.computeHashWithSteps(header);
            resultArray.push_back(hasher.getZeroCount());
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

        std::cout << "Processing complete. " << resultArray.size() << " values collected." << std::endl;
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
        for (size_t i = 0; i < resultArray.size(); i++) {
            file << currentNonce << " " << resultArray[i] << std::endl;
            if (currentNonce <= nonceEnd_ - nonceStep_) {
                currentNonce += nonceStep_;
            }
        }

        file << "\n# Autocorrelation results" << std::endl;
        auto autocorr = Autocorrelator::compute(resultArray, maxLag);
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

    void run() {
        try {
            interactiveInputBlockHeight();
            interactiveInputNonceRange();
            processNonceRange();

            if (resultArray.empty()) {
                std::cerr << "Error: No results generated" << std::endl;
                return;
            }

            auto autocorr = Autocorrelator::compute(resultArray, maxLag);
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
