#include "../include/upload_test.h"
#include <curl/curl.h>
#include <iostream>
#include <chrono>
#include <random>

UploadTest::UploadTest()
    : running_(false), totalBytes_(0), currentSpeedMbps_(0.0) {
}

UploadTest::~UploadTest() {
    stop();
}

void UploadTest::stop() {
    running_ = false;
    
    // Wait for all threads to finish
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    threads_.clear();
}

double UploadTest::run(const std::string& url, int parallelConnections,
                       int durationSeconds, int warmupSeconds,
                       ProgressCallback callback) {
    
    running_ = true;
    totalBytes_ = 0;
    currentSpeedMbps_ = 0.0;
    threads_.clear();
    
    auto startTime = std::chrono::steady_clock::now();
    auto warmupEnd = startTime + std::chrono::seconds(warmupSeconds);
    auto testEnd = startTime + std::chrono::seconds(durationSeconds);
    
    // Start parallel upload threads
    for (int i = 0; i < parallelConnections && running_; ++i) {
        threads_.emplace_back(&UploadTest::uploadWorker, this, url, i);
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
        }
        
        // Calculate current speed (after warmup)
        if (warmupComplete) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - warmupEnd).count() / 1000.0;
            if (elapsed > 0) {
                uint64_t measuredBytes = totalBytes_.load() - bytesAtWarmupEnd;
                double mbps = calculateMbps(measuredBytes, elapsed);
                currentSpeedMbps_ = mbps;
                
                if (callback) {
                    double progress = elapsed / (durationSeconds - warmupSeconds);
                    callback("Uploading...", progress, mbps);
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
    auto totalElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(finalTime - warmupEnd).count() / 1000.0;
    
    if (totalElapsed > 0) {
        uint64_t measuredBytes = totalBytes_.load() - bytesAtWarmupEnd;
        return calculateMbps(measuredBytes, totalElapsed);
    }
    
    return 0.0;
}

void UploadTest::uploadWorker(const std::string& url, int threadId) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl for upload thread " << threadId << std::endl;
        return;
    }
    
    // Generate random data for upload (1MB chunk)
    std::vector<char> uploadData = generateUploadData(1024 * 1024);
    uint64_t threadBytes = 0;
    
    // Configure curl for POST upload
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, uploadData.size());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, uploadData.data());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    
    // Discard response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, 
        [](void* contents, size_t size, size_t nmemb, void* userp) -> size_t {
            return size * nmemb;  // Discard all data
        });
    
    // Keep uploading while test is running
    while (running_) {
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            threadBytes += uploadData.size();
            totalBytes_ += uploadData.size();
        } else if (running_) {
            std::cerr << "Upload error in thread " << threadId 
                      << ": " << curl_easy_strerror(res) << std::endl;
            // Small delay before retry
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    
    curl_easy_cleanup(curl);
}

double UploadTest::calculateMbps(uint64_t bytes, double seconds) {
    if (seconds <= 0) return 0.0;
    
    // Convert bytes to megabits
    double bits = bytes * 8.0;
    double megabits = bits / 1000000.0;
    
    return megabits / seconds;
}

std::vector<char> UploadTest::generateUploadData(size_t size) {
    std::vector<char> data(size);
    
    // Fill with random-ish data (doesn't need to be cryptographically secure)
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (size_t i = 0; i < size; ++i) {
        data[i] = static_cast<char>(dis(gen));
    }
    
    return data;
}
