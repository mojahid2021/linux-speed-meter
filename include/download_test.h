#ifndef DOWNLOAD_TEST_H
#define DOWNLOAD_TEST_H

#include "speed_test.h"
#include <atomic>
#include <thread>
#include <mutex>

// Download speed test with parallel connections
class DownloadTest {
public:
    DownloadTest();
    ~DownloadTest();
    
    // Run download test with multiple parallel threads
    // Returns speed in Mbps
    double run(const std::string& url, int parallelConnections = 4, 
               int durationSeconds = 10, int warmupSeconds = 2,
               ProgressCallback callback = nullptr);
    
    // Stop ongoing test
    void stop();
    
    // Get progressive results during test
    double getCurrentSpeed() const { return currentSpeedMbps_.load(); }
    uint64_t getTotalBytesDownloaded() const { return totalBytes_.load(); }
    
private:
    std::atomic<bool> running_;
    std::atomic<uint64_t> totalBytes_;
    std::atomic<double> currentSpeedMbps_;
    std::vector<std::thread> threads_;
    std::mutex mutex_;
    
    // Per-thread download worker
    void downloadWorker(const std::string& url, int threadId);
    
    // Calculate speed from bytes and time
    static double calculateMbps(uint64_t bytes, double seconds);
};

#endif // DOWNLOAD_TEST_H
