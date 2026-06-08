//====================================================================================================================
// Создание 02.06.2026
//     dd
// bitcoin_analyzer.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
//====================================================================================================================

#define NOMINMAX

#include "orchestrator.h"
#include "config.h"
#include <iostream>
#include <curl/curl.h>


int main(int argc, char* argv[]) {

    // Initialize CURL globally
    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::cout << "=== Bitcoin Block Analyzer ===" << std::endl;
    std::cout << "Analyzing zero patterns in SHA-256 steps" << std::endl;
    std::cout << std::endl;

    // Load configuration
    Config config;
    if (argc > 1) {
        config = Config::loadFromFile(argv[1]);
    }
    else {
        config = Config::loadFromFile("config.json");
    }

    
    std::cout << "Configuration:" << std::endl;
    std::cout << "  API URL: " << config.getBitcoinApiUrl() << std::endl;
    std::cout << "  Zero counting mode: " << config.getZeroCountingMode()
        << (config.getZeroCountingMode() == 1 ? " (byte mode)" : " (word mode)") << std::endl;
    std::cout << "  Max autocorrelation lag: " << config.getMaxLagForAutocorrelation() << std::endl;
    std::cout << std::endl;
    
    

    try {
        BitcoinClient client(config.getBitcoinApiUrl(), config.getNetworkTimeoutSeconds());
        Orchestrator orchestrator(client,
            config.getMaxLagForAutocorrelation(),
            config.getZeroCountingMode());
        orchestrator.run(config);
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        curl_global_cleanup();
        return 1;
    }

    curl_global_cleanup();
    std::cout << "\nProgram finished." << std::endl;
    return 0;
}



//// test debug ---------------------------------------------
//       Sha256StepRecorder record;
//       std::string empty("abc");
//       std::vector<uint8_t>  input = record.stringToBytes(empty);
//       std::vector<uint8_t> res = record.customSha256(input);

//       std::vector<uint8_t> expected = record.hexToBytes(
//           "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
//       );

//       assert(res.size() == 32);
//       assert(record.compareVectors(expected, res));
////-----------------------------------------------------------
