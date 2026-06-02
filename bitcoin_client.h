#pragma once


#include <string>
#include <vector>
#include <cstdint>
#include <curl/curl.h>
#include <sstream>

class BitcoinClient {
private:
    std::string baseUrl; //comment вава ыы
    int timeoutSeconds;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t totalSize = size * nmemb;
        userp->append((char*)contents, totalSize);
        return totalSize;
    }

public:
    BitcoinClient(const std::string& url, int timeout)
        : baseUrl(url), timeoutSeconds(timeout) {
    }

    std::vector<uint8_t> fetchBlockByHeight(uint32_t height) {
        std::string url = baseUrl + std::to_string(height);
        std::string response;
         
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutSeconds); 

        CURLcode res = curl_easy_perform(curl); //ывава
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            throw std::runtime_error("Failed to fetch block: " + std::string(curl_easy_strerror(res)));
        }

        // Convert hex string to bytes
        std::vector<uint8_t> result;
        for (size_t i = 0; i < response.length(); i += 2) {
            std::string byteString = response.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
            result.push_back(byte);
        }

        return result;
    }
};

