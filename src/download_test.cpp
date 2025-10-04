#include "../include/download_test.h"
#include <curl/curl.h>
#include <iostream>
#include <chrono>
#include <cstring>
#include <algorithm>

DownloadTest::DownloadTest() 
    : running_(false), totalBytes_(0), currentSpeedMbps_(0.0) {
}

DownloadTest::~DownloadTest() {
    stop();
}

void DownloadTest::stop() {
    running_ = false;
    
    // Wait for all threads to finish
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    threads_.clear();
}

double DownloadTest::run(const std::string& url, int parallelConnections,
                         int durationSeconds, int warmupSeconds,
                         ProgressCallback callback) {

    if (durationSeconds <= 0) {
        durationSeconds = 1;
    }
    if (warmupSeconds < 0) {
        warmupSeconds = 0;
    }

    int totalDuration = std::max(durationSeconds, warmupSeconds + 1);
    int effectiveWarmup = std::min(warmupSeconds, totalDuration - 1);
    double measurementWindow = static_cast<double>(totalDuration - effectiveWarmup);
    if (measurementWindow <= 0.0) {
        measurementWindow = 1.0;
    }

    running_ = true;
    totalBytes_ = 0;
    currentSpeedMbps_ = 0.0;
    threads_.clear();
    
    auto startTime = std::chrono::steady_clock::now();
    auto warmupEnd = startTime + std::chrono::seconds(effectiveWarmup);
    auto testEnd = startTime + std::chrono::seconds(totalDuration);
    auto measurementStart = (effectiveWarmup == 0) ? startTime : warmupEnd;
    
    // Start parallel download threads
    for (int i = 0; i < parallelConnections && running_; ++i) {
        threads_.emplace_back(&DownloadTest::downloadWorker, this, url, i);
    }
    
    // Monitor progress
    uint64_t bytesAtWarmupEnd = 0;
    bool warmupComplete = false;
    
    while (running_ && std::chrono::steady_clock::now() < testEnd) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        auto now = std::chrono::steady_clock::now();
        
        // Track warmup period
        if (!warmupComplete && now >= warmupEnd) {
            bytesAtWarmupEnd = totalBytes_.load();
            warmupComplete = true;
            measurementStart = now;
        }
        
        // Calculate current speed (after warmup)
        if (warmupComplete) {
            auto elapsed = std::chrono::duration<double>(now - measurementStart).count();
            if (elapsed > 0) {
                uint64_t measuredBytes = totalBytes_.load() - bytesAtWarmupEnd;
                double mbps = calculateMbps(measuredBytes, elapsed);
                currentSpeedMbps_ = mbps;
                
                if (callback) {
                    double progress = elapsed / measurementWindow;
                    if (progress < 0.0) progress = 0.0;
                    if (progress > 1.0) progress = 1.0;
                    callback("Downloading...", progress, mbps);
                }
            }
        }
    }
    
    // Stop all threads
    running_ = false;
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    threads_.clear();
    
    // Calculate final speed (excluding warmup period)
    auto finalTime = std::chrono::steady_clock::now();
    double totalElapsed = std::chrono::duration<double>(finalTime - measurementStart).count();
    if (totalElapsed <= 0.0) {
        totalElapsed = std::chrono::duration<double>(finalTime - startTime).count();
    }

    if (totalElapsed > 0.0) {
        uint64_t measuredBytes = totalBytes_.load() - (warmupComplete ? bytesAtWarmupEnd : 0);
        return calculateMbps(measuredBytes, totalElapsed);
    }

    return 0.0;
}

void DownloadTest::downloadWorker(const std::string& url, int threadId) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl for thread " << threadId << std::endl;
        return;
    }
    
    uint64_t threadBytes = 0;
    
    // Configure curl
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, 
        [](void* contents, size_t size, size_t nmemb, void* userp) -> size_t {
            size_t realsize = size * nmemb;
            uint64_t* counter = static_cast<uint64_t*>(userp);
            *counter += realsize;
            return realsize;
        });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &threadBytes);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    
    // Keep downloading while test is running
    while (running_) {
        threadBytes = 0;
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            totalBytes_ += threadBytes;
        } else if (running_) {
            std::cerr << "Download error in thread " << threadId 
                      << ": " << curl_easy_strerror(res) << std::endl;
        }
        
        // Small delay before next iteration
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    curl_easy_cleanup(curl);
}

double DownloadTest::calculateMbps(uint64_t bytes, double seconds) {
    if (seconds <= 0) return 0.0;
    
    // Convert bytes to megabits
    double bits = bytes * 8.0;
    double megabits = bits / 1000000.0;
    
    return megabits / seconds;
}
