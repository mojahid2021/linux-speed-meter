#ifndef PING_TEST_H
#define PING_TEST_H

#include "speed_test.h"
#include <vector>

// Latency and jitter test results
struct PingResults {
    double minMs;
    double maxMs;
    double avgMs;
    double jitterMs;
    int successCount;
    int totalCount;
    std::vector<double> samples;
    
    PingResults() : minMs(0), maxMs(0), avgMs(0), jitterMs(0), 
                    successCount(0), totalCount(0) {}
};

// Ping/latency test class
class PingTest {
public:
    PingTest();
    ~PingTest();
    
    // Run ping test
    // Uses TCP connection timing as fallback to ICMP
    PingResults run(const std::string& host, int port = 80, int count = 10);
    
    // Single ping measurement
    double singlePing(const std::string& host, int port = 80);
    
    // Calculate statistics from raw ping samples
    static PingResults calculateStats(const std::vector<double>& samples);
    
    // Calculate jitter (variation in latency)
    static double calculateJitter(const std::vector<double>& samples);
    
private:
    int timeout_;  // milliseconds
    
    // TCP-based ping (HTTP HEAD request or socket connect)
    double tcpPing(const std::string& host, int port);
    
    // HTTP-based ping using curl
    double httpPing(const std::string& url);
};

#endif // PING_TEST_H
