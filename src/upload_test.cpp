#include "../include/upload_test.h"
#include "../include/curl_wrapper.h"
#include <curl/curl.h>
#include <iostream>
#include <chrono>
#include <random>
#include <algorithm>

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
    
    // Add maximum timeout (2x the expected duration) to prevent hanging
    auto maxTestEnd = startTime + std::chrono::seconds(totalDuration * 2);
    
    // Start parallel upload threads
    for (int i = 0; i < parallelConnections && running_; ++i) {
        threads_.emplace_back(&UploadTest::uploadWorker, this, url, i);
    }
    
    // Monitor progress
    uint64_t bytesAtWarmupEnd = 0;
    bool warmupComplete = false;
    
    while (running_ && std::chrono::steady_clock::now() < testEnd && std::chrono::steady_clock::now() < maxTestEnd) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));  // More frequent updates
        
        auto now = std::chrono::steady_clock::now();
        
        // Track warmup period
        if (!warmupComplete && now >= warmupEnd) {
            bytesAtWarmupEnd = totalBytes_.load();
            warmupComplete = true;
            measurementStart = now;
        }
        
        // Calculate progress and speed
        auto elapsed = std::chrono::duration<double>(now - startTime).count();
        double overallProgress = elapsed / totalDuration;
        if (overallProgress > 1.0) overallProgress = 1.0;
        
        if (callback) {
            if (!warmupComplete) {
                // During warmup, show progress based on time
                callback("Warming up...", overallProgress * 0.5, 0.0);
            } else {
                // After warmup, show upload progress
                auto measurementElapsed = std::chrono::duration<double>(now - measurementStart).count();
                double measurementProgress = measurementElapsed / measurementWindow;
                if (measurementProgress > 1.0) measurementProgress = 1.0;
                
                uint64_t measuredBytes = totalBytes_.load() - bytesAtWarmupEnd;
                double mbps = calculateMbps(measuredBytes, measurementElapsed);
                currentSpeedMbps_ = mbps;
                
                callback("Uploading...", 0.5 + measurementProgress * 0.5, mbps);
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

void UploadTest::uploadWorker(const std::string& url, int threadId) {
    CurlHandle curl;
    if (!curl) {
        std::cerr << "Failed to initialize curl for upload thread " << threadId << std::endl;
        return;
    }
    
    // Generate random data for upload (1MB chunk)
    std::vector<char> uploadData = generateUploadData(1024 * 1024);
    uint64_t threadBytes = 0;
    int consecutiveErrors = 0;
    const int maxConsecutiveErrors = 3;
    
    // Configure curl for POST upload
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, uploadData.size());
    curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, uploadData.data());
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 10L);  // Shorter timeout for speed testing
    curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);  // For speed testing, skip SSL verification
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 0L);
    
    // Discard response
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, 
        [](void* contents, size_t size, size_t nmemb, void* userp) -> size_t {
            return size * nmemb;  // Discard all data
        });
    
    // Keep uploading while test is running
    while (running_ && consecutiveErrors < maxConsecutiveErrors) {
        CURLcode res = curl_easy_perform(curl.get());
        
        if (res == CURLE_OK) {
            threadBytes += uploadData.size();
            totalBytes_ += uploadData.size();
            consecutiveErrors = 0;  // Reset error counter on success
        } else if (running_) {
            consecutiveErrors++;
            std::cerr << "Upload error in thread " << threadId 
                      << ": " << curl_easy_strerror(res)
                      << " (attempt " << consecutiveErrors << "/" << maxConsecutiveErrors << ")" << std::endl;
            // Small delay before retry
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    // RAII wrapper automatically cleans up CURL handle
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
