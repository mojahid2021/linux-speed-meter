#ifndef UPLOAD_TEST_H
#define UPLOAD_TEST_H

#include "speed_test.h"
#include <atomic>
#include <thread>
#include <mutex>

// Upload speed test with parallel connections
class UploadTest {
public:
    UploadTest();
    ~UploadTest();
    
    // Run upload test with multiple parallel threads
    // Returns speed in Mbps
    double run(const std::string& url, int parallelConnections = 4,
               int durationSeconds = 10, int warmupSeconds = 2,
               ProgressCallback callback = nullptr);
    
    // Stop ongoing test
    void stop();
    
    // Get progressive results during test
    double getCurrentSpeed() const { return currentSpeedMbps_.load(); }
    uint64_t getTotalBytesUploaded() const { return totalBytes_.load(); }
    
private:
    std::atomic<bool> running_;
    std::atomic<uint64_t> totalBytes_;
    std::atomic<double> currentSpeedMbps_;
    std::vector<std::thread> threads_;
    std::mutex mutex_;
    
    // Per-thread upload worker
    void uploadWorker(const std::string& url, int threadId);
    
    // Calculate speed from bytes and time
    static double calculateMbps(uint64_t bytes, double seconds);
    
    // Generate random data for upload
    static std::vector<char> generateUploadData(size_t size);
};

#endif // UPLOAD_TEST_H
