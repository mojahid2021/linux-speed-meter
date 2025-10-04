#include "../include/ping_test.h"
#include "../include/curl_wrapper.h"
#include <curl/curl.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <algorithm>
#include <cmath>

PingTest::PingTest() : timeout_(5000) {  // 5 second timeout
}

PingTest::~PingTest() {
}

PingResults PingTest::run(const std::string& host, int port, int count) {
    std::vector<double> samples;
    samples.reserve(count);
    
    std::cout << "Running ping test to " << host << ":" << port 
              << " (" << count << " samples)..." << std::endl;
    
    for (int i = 0; i < count; ++i) {
        double pingTime = singlePing(host, port);
        
        if (pingTime > 0) {
            samples.push_back(pingTime);
            std::cout << "Ping " << (i + 1) << ": " << pingTime << " ms" << std::endl;
        } else {
            std::cout << "Ping " << (i + 1) << ": failed" << std::endl;
        }
        
        // Small delay between pings
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return calculateStats(samples);
}

double PingTest::singlePing(const std::string& host, int port) {
    // Try HTTP ping first (more reliable cross-platform)
    std::string url = "http://" + host;
    if (port != 80) {
        url += ":" + std::to_string(port);
    }
    
    double httpTime = httpPing(url);
    if (httpTime > 0) {
        return httpTime;
    }
    
    // Fallback to TCP ping
    return tcpPing(host, port);
}

double PingTest::httpPing(const std::string& url) {
    CurlHandle curl;
    if (!curl) {
        return -1.0;
    }
    
    // Configure for HEAD request (minimal data transfer)
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_NOBODY, 1L);  // HEAD request
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, timeout_);
    curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);  // Skip SSL verification for speed testing
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 0L);
    
    // Discard any response
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, 
        [](void* contents, size_t size, size_t nmemb, void* userp) -> size_t {
            return size * nmemb;
        });
    
    auto start = std::chrono::high_resolution_clock::now();
    CURLcode res = curl_easy_perform(curl.get());
    auto end = std::chrono::high_resolution_clock::now();
    
    if (res != CURLE_OK) {
        return -1.0;
    }
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return duration / 1000.0;  // Convert to milliseconds
    // RAII wrapper automatically cleans up CURL handle
}

double PingTest::tcpPing(const std::string& host, int port) {
    // TCP connection timing as fallback
    // This would require platform-specific socket code
    // For now, return -1 to indicate not implemented
    // In production, would use raw sockets or boost::asio
    return -1.0;
}

PingResults PingTest::calculateStats(const std::vector<double>& samples) {
    PingResults results;
    results.samples = samples;
    results.totalCount = samples.size();
    results.successCount = samples.size();
    
    if (samples.empty()) {
        return results;
    }
    
    // Calculate min, max, avg
    results.minMs = *std::min_element(samples.begin(), samples.end());
    results.maxMs = *std::max_element(samples.begin(), samples.end());
    
    double sum = 0.0;
    for (double sample : samples) {
        sum += sample;
    }
    results.avgMs = sum / samples.size();
    
    // Calculate jitter
    results.jitterMs = calculateJitter(samples);
    
    return results;
}

double PingTest::calculateJitter(const std::vector<double>& samples) {
    if (samples.size() < 2) {
        return 0.0;
    }
    
    // Calculate standard deviation of consecutive differences
    std::vector<double> differences;
    for (size_t i = 1; i < samples.size(); ++i) {
        differences.push_back(std::abs(samples[i] - samples[i-1]));
    }
    
    double sum = 0.0;
    for (double diff : differences) {
        sum += diff;
    }
    double avgDiff = sum / differences.size();
    
    // Calculate standard deviation
    double sumSquares = 0.0;
    for (double diff : differences) {
        double deviation = diff - avgDiff;
        sumSquares += deviation * deviation;
    }
    
    return std::sqrt(sumSquares / differences.size());
}
