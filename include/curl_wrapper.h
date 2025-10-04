#ifndef CURL_WRAPPER_H
#define CURL_WRAPPER_H

#include <curl/curl.h>
#include <memory>

// RAII wrapper for CURL handle to prevent leaks
class CurlHandle {
public:
    CurlHandle() : curl_(curl_easy_init()) {}
    
    ~CurlHandle() {
        if (curl_) {
            curl_easy_cleanup(curl_);
        }
    }
    
    // Delete copy constructor and assignment operator
    CurlHandle(const CurlHandle&) = delete;
    CurlHandle& operator=(const CurlHandle&) = delete;
    
    // Allow move semantics
    CurlHandle(CurlHandle&& other) noexcept : curl_(other.curl_) {
        other.curl_ = nullptr;
    }
    
    CurlHandle& operator=(CurlHandle&& other) noexcept {
        if (this != &other) {
            if (curl_) {
                curl_easy_cleanup(curl_);
            }
            curl_ = other.curl_;
            other.curl_ = nullptr;
        }
        return *this;
    }
    
    // Get the raw CURL pointer
    CURL* get() const { return curl_; }
    
    // Check if the handle is valid
    bool isValid() const { return curl_ != nullptr; }
    
    // Convenience operator for use in if statements
    explicit operator bool() const { return isValid(); }
    
private:
    CURL* curl_;
};

#endif // CURL_WRAPPER_H
