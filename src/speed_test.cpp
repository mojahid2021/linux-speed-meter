#include "../include/speed_test.h"
#include <curl/curl.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstring>

SpeedTest::SpeedTest()
    : parallelConnections_(4), testDuration_(10), warmupTime_(2), 
      timeout_(30), running_(false) {
    // Initialize curl globally
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

SpeedTest::~SpeedTest() {
    curl_global_cleanup();
}

SpeedTestResult SpeedTest::runCompleteTest(const TestServer& server, ProgressCallback callback) {
    SpeedTestResult result;
    result.timestamp = std::chrono::system_clock::now();
    result.serverName = server.name;
    result.serverUrl = server.host;
    
    try {
        // Step 1: Ping test
        if (callback) callback("Testing latency...", 0.0, 0.0);
        result.pingMs = testPing(server);
        
        // Step 2: Download test
        if (callback) callback("Testing download speed...", 0.33, 0.0);
        result.downloadSpeedMbps = testDownloadSpeed(server, callback);
        
        // Step 3: Upload test
        if (callback) callback("Testing upload speed...", 0.66, 0.0);
        result.uploadSpeedMbps = testUploadSpeed(server, callback);
        
        if (callback) callback("Test complete!", 1.0, 0.0);
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = e.what();
    }
    
    return result;
}

double SpeedTest::testDownloadSpeed(const TestServer& server, ProgressCallback callback) {
    // Implementation delegated to DownloadTest class
    return 0.0;
}

double SpeedTest::testUploadSpeed(const TestServer& server, ProgressCallback callback) {
    // Implementation delegated to UploadTest class
    return 0.0;
}

double SpeedTest::testPing(const TestServer& server, int count) {
    // Implementation delegated to PingTest class
    return 0.0;
}

double SpeedTest::calculateJitter(const std::vector<double>& pingResults) {
    if (pingResults.size() < 2) return 0.0;
    
    double sumSquaredDiff = 0.0;
    for (size_t i = 1; i < pingResults.size(); ++i) {
        double diff = pingResults[i] - pingResults[i-1];
        sumSquaredDiff += diff * diff;
    }
    
    return std::sqrt(sumSquaredDiff / (pingResults.size() - 1));
}

// Default test servers (can be configured)
std::vector<TestServer> SpeedTest::getDefaultServers() {
    std::vector<TestServer> servers;
    
    // Example servers - in production, these would be real speed test servers
    // Using common fast download URLs for testing
    servers.push_back(TestServer(
        "Cloudflare",
        "speed.cloudflare.com",
        "https://speed.cloudflare.com/__down?bytes=25000000",  // 25MB
        "https://speed.cloudflare.com/__up"
    ));
    
    servers.push_back(TestServer(
        "Fast.com (Netflix)",
        "fast.com",
        "https://api.fast.com/netflix/speedtest",
        "https://api.fast.com/netflix/speedtest"
    ));
    
    // LibreSpeed compatible servers
    servers.push_back(TestServer(
        "LibreSpeed Demo",
        "librespeed.org",
        "http://librespeed.org/garbage.php?ckSize=25",
        "http://librespeed.org/empty.php"
    ));
    
    return servers;
}

TestServer SpeedTest::findClosestServer(const std::vector<TestServer>& servers) {
    // For now, return first server
    // In production, would ping all servers and select fastest
    if (servers.empty()) {
        throw std::runtime_error("No servers available");
    }
    return servers[0];
}

// Callback for writing downloaded data
size_t SpeedTest::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    // Just count bytes, don't store data
    uint64_t* counter = static_cast<uint64_t*>(userp);
    *counter += realsize;
    return realsize;
}

// Callback for reading upload data
size_t SpeedTest::readCallback(void* ptr, size_t size, size_t nmemb, void* userp) {
    size_t maxBytes = size * nmemb;
    std::vector<char>* data = static_cast<std::vector<char>*>(userp);
    
    if (data->empty()) return 0;
    
    size_t bytesToCopy = std::min(maxBytes, data->size());
    memcpy(ptr, data->data(), bytesToCopy);
    
    // Remove copied bytes from buffer (or refill for continuous upload)
    // For continuous upload, we'll just keep sending the same data
    return bytesToCopy;
}
