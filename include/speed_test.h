#ifndef SPEED_TEST_H
#define SPEED_TEST_H

#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <functional>

// Result structure for speed tests
struct SpeedTestResult {
    double downloadSpeedMbps;
    double uploadSpeedMbps;
    double pingMs;
    double jitterMs;
    std::string serverName;
    std::string serverUrl;
    std::chrono::system_clock::time_point timestamp;
    bool success;
    std::string errorMessage;
    
    SpeedTestResult() 
        : downloadSpeedMbps(0.0), uploadSpeedMbps(0.0), 
          pingMs(0.0), jitterMs(0.0), success(false) {}
};

// Server information
struct TestServer {
    std::string name;
    std::string host;
    std::string downloadUrl;
    std::string uploadUrl;
    std::string country;
    double distance;  // km from user (can be calculated or configured)
    
    TestServer(const std::string& n, const std::string& h, 
               const std::string& dl, const std::string& ul)
        : name(n), host(h), downloadUrl(dl), uploadUrl(ul), 
          country(""), distance(0.0) {}
};

// Progress callback for UI updates
using ProgressCallback = std::function<void(const std::string& stage, double progress, double currentSpeed)>;

// Base class for all speed tests
class SpeedTest {
public:
    SpeedTest();
    virtual ~SpeedTest();
    
    // Run complete speed test
    SpeedTestResult runCompleteTest(const TestServer& server, ProgressCallback callback = nullptr);
    
    // Individual test methods
    double testDownloadSpeed(const TestServer& server, ProgressCallback callback = nullptr);
    double testUploadSpeed(const TestServer& server, ProgressCallback callback = nullptr);
    double testPing(const TestServer& server, int count = 10);
    double calculateJitter(const std::vector<double>& pingResults);
    
    // Configuration
    void setParallelConnections(int count) { parallelConnections_ = count; }
    void setTestDuration(int seconds) { testDuration_ = seconds; }
    void setWarmupTime(int seconds) { warmupTime_ = seconds; }
    void setTimeout(int seconds) { timeout_ = seconds; }
    
    int getParallelConnections() const { return parallelConnections_; }
    int getTestDuration() const { return testDuration_; }
    int getWarmupTime() const { return warmupTime_; }
    
    // Server management
    static std::vector<TestServer> getDefaultServers();
    static TestServer findClosestServer(const std::vector<TestServer>& servers);
    
protected:
    int parallelConnections_;
    int testDuration_;      // seconds
    int warmupTime_;        // seconds to discard for TCP warm-up
    int timeout_;           // seconds for individual operations
    
    bool running_;
    
    // Helper methods for subclasses
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static size_t readCallback(void* ptr, size_t size, size_t nmemb, void* userp);
};

#endif // SPEED_TEST_H
