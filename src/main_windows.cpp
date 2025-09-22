#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>
#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <memory>
#include "../include/data_manager.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

static std::atomic<bool> running(true);

void signal_handler(int signal) {
    running = false;
}

// Windows-specific speed monitoring
class WindowsSpeedMonitor {
private:
    MIB_IFTABLE* ifTable = nullptr;
    DWORD lastUpdate = 0;
    double currentDownloadSpeed = 0.0;
    double currentUploadSpeed = 0.0;
    uint64_t totalDownloadBytes = 0;
    uint64_t totalUploadBytes = 0;

public:
    WindowsSpeedMonitor() {
        ifTable = (MIB_IFTABLE*)malloc(sizeof(MIB_IFTABLE));
        update();
    }

    ~WindowsSpeedMonitor() {
        if (ifTable) {
            free(ifTable);
        }
    }

    void update() {
        DWORD dwRetVal = 0;
        DWORD dwSize = sizeof(MIB_IFTABLE);

        // Get interface table
        dwRetVal = GetIfTable(ifTable, &dwSize, FALSE);
        if (dwRetVal != NO_ERROR) {
            std::cerr << "GetIfTable failed with error: " << dwRetVal << std::endl;
            return;
        }

        DWORD currentTime = GetTickCount();
        DWORD timeDiff = currentTime - lastUpdate;

        if (lastUpdate > 0 && timeDiff > 0) {
            // Find active interface with most traffic
            PMIB_IFROW bestInterface = nullptr;
            uint64_t maxTraffic = 0;

            for (DWORD i = 0; i < ifTable->dwNumEntries; i++) {
                PMIB_IFROW row = &ifTable->table[i];

                // Skip loopback and inactive interfaces
                if (row->dwType == MIB_IF_TYPE_LOOPBACK ||
                    !(row->dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL)) {
                    continue;
                }

                uint64_t traffic = row->dwInOctets + row->dwOutOctets;
                if (traffic > maxTraffic) {
                    maxTraffic = traffic;
                    bestInterface = row;
                }
            }

            if (bestInterface) {
                // Calculate speeds
                currentDownloadSpeed = ((double)(bestInterface->dwInOctets - totalDownloadBytes) * 1000.0) / timeDiff;
                currentUploadSpeed = ((double)(bestInterface->dwOutOctets - totalUploadBytes) * 1000.0) / timeDiff;

                totalDownloadBytes = bestInterface->dwInOctets;
                totalUploadBytes = bestInterface->dwOutOctets;
            }
        } else {
            // First update - just store values
            for (DWORD i = 0; i < ifTable->dwNumEntries; i++) {
                PMIB_IFROW row = &ifTable->table[i];
                if (row->dwType != MIB_IF_TYPE_LOOPBACK &&
                    row->dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL) {
                    totalDownloadBytes = row->dwInOctets;
                    totalUploadBytes = row->dwOutOctets;
                    break;
                }
            }
        }

        lastUpdate = currentTime;
    }

    double get_current_download_speed() const { return currentDownloadSpeed; }
    double get_current_upload_speed() const { return currentUploadSpeed; }
    double get_total_rx_mb() const { return totalDownloadBytes / (1024.0 * 1024.0); }
    double get_total_tx_mb() const { return totalUploadBytes / (1024.0 * 1024.0); }
};

std::string formatSpeed(double speed);

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "Linux Speed Meter - Windows Console Version" << std::endl;
    std::cout << "==========================================" << std::endl;

    try {
        auto speedMonitor = std::make_unique<WindowsSpeedMonitor>();
        auto dataManager = std::make_unique<DataManager>();

        std::cout << "Monitoring network speed... Press Ctrl+C to exit." << std::endl;
        std::cout << "Download | Upload | Total Down | Total Up" << std::endl;
        std::cout << "---------|--------|------------|----------" << std::endl;

        int update_counter = 0;
        uint64_t prev_total_download_bytes = 0;
        uint64_t prev_total_upload_bytes = 0;

        while (running) {
            // Update speed monitor
            speedMonitor->update();

            // Display current speeds
            double download_speed = speedMonitor->get_current_download_speed();
            double upload_speed = speedMonitor->get_current_upload_speed();
            double total_download = speedMonitor->get_total_rx_mb();
            double total_upload = speedMonitor->get_total_tx_mb();

            std::cout << formatSpeed(download_speed) << " | "
                      << formatSpeed(upload_speed) << " | "
                      << std::to_string(total_download).substr(0, 8) << " MB | "
                      << std::to_string(total_upload).substr(0, 8) << " MB" << std::endl;

            // Save data every minute
            update_counter++;
            if (update_counter >= 60) {
                uint64_t current_download_bytes = total_download * 1024 * 1024;
                uint64_t current_upload_bytes = total_upload * 1024 * 1024;
                
                // Calculate incremental bytes since last save
                uint64_t incremental_download = current_download_bytes - prev_total_download_bytes;
                uint64_t incremental_upload = current_upload_bytes - prev_total_upload_bytes;
                
                // Update previous totals
                prev_total_download_bytes = current_download_bytes;
                prev_total_upload_bytes = current_upload_bytes;
                
                dataManager->updateDailyStats(
                    incremental_download,    // Incremental download bytes
                    incremental_upload,      // Incremental upload bytes
                    download_speed,
                    upload_speed,
                    std::chrono::seconds(update_counter)
                );
                update_counter = 0;
                std::cout << "Data saved to persistent storage." << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << std::endl << "Shutting down..." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}