#pragma once


#include <string>
#include <vector>
#include <cstdint>
#include <curl/curl.h>
#include <sstream>
#include <nlohmann/json.hpp> 

using json = nlohmann::json;

class BitcoinClient {
private:
    std::string baseUrl_; 
    int timeoutSeconds_;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t totalSize = size * nmemb;
        userp->append((char*)contents, totalSize);
        return totalSize;
    }

    std::string performHttpGet(const std::string& url) {
        std::string response;

        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSeconds_);

        // Set user-agent to avoid being blocked
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            throw std::runtime_error("Failed to fetch URL: " + url + ", error: " + std::string(curl_easy_strerror(res)));
        }

        return response;
    }

public:
    BitcoinClient(const std::string& url, int timeout)
        : baseUrl_(url), timeoutSeconds_(timeout) {
    }

    std::vector<uint8_t> fetchHashByHeight(uint32_t height) {

        std::string url = baseUrl_ + "api/block-height/" + std::to_string(height);
        std::string response = performHttpGet(url);

        // Convert hex string to bytes
        std::vector<uint8_t> result;
        for (size_t i = 0; i < response.length(); i += 2) {
            std::string byteString = response.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
            result.push_back(byte);
        }

        return result;
    }

    json fetchBlockByHeight (uint32_t height) {

        // Step 1: Get block hash by height
        std::string hashUrl = baseUrl_ + "api/block-height/" + std::to_string(height);
        std::string blockHash = performHttpGet(hashUrl);

        // Remove any whitespace or newlines from the hash
        blockHash.erase(remove_if(blockHash.begin(), blockHash.end(), ::isspace), blockHash.end());

        // Step 2: Get block data by hash
        std::string blockUrl = baseUrl_ + "api/block/" + blockHash;
        std::string blockData = performHttpGet(blockUrl);

        // Parse JSON response
        json blockJson = json::parse(blockData);

        return blockJson;
    }

    

};

