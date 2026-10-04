#include "SystemStats.h"
#include <esp_heap_caps.h>
#include <time.h>
#include <stdio.h>

extern uint32_t g_serialRxBytes;
extern uint32_t g_serialTxBytes;
extern uint32_t g_serialRxOverflow;
extern uint32_t g_serialBaudRate;
extern uint16_t g_activeWsClients;
extern uint16_t g_activeTelnetClients;
extern bool     g_ntpSynced;

void SystemStats::update(SystemStatsData &stats) {
    stats.uptimeSeconds = millis() / 1000;
    stats.cpuFreqMhz = getCpuFrequencyMhz();

    stats.freeHeapBytes = ESP.getFreeHeap();
    stats.minFreeHeapBytes = ESP.getMinFreeHeap();
    stats.maxAllocHeapBytes = ESP.getMaxAllocHeap();

    if (stats.freeHeapBytes > 0 && stats.maxAllocHeapBytes > 0 && stats.maxAllocHeapBytes <= stats.freeHeapBytes) {
        stats.heapFragPercent = (uint8_t)(100 - ((stats.maxAllocHeapBytes * 100ULL) / stats.freeHeapBytes));
    } else {
        stats.heapFragPercent = 0;
    }

    stats.flashSizeBytes = ESP.getFlashChipSize();
    stats.sketchSizeBytes = ESP.getSketchSize();
    stats.freeSketchSpaceBytes = ESP.getFreeSketchSpace();

    stats.wifiStaConnected = (WiFi.status() == WL_CONNECTED);
    stats.wifiApActive = (WiFi.getMode() & WIFI_MODE_AP);
    stats.wifiApClients = stats.wifiApActive ? WiFi.softAPgetStationNum() : 0;
    stats.wifiRssi = stats.wifiStaConnected ? WiFi.RSSI() : 0;

    if (stats.wifiStaConnected) {
        snprintf(stats.ipAddress, sizeof(stats.ipAddress), "%s", WiFi.localIP().toString().c_str());
        snprintf(stats.ssid, sizeof(stats.ssid), "%s", WiFi.SSID().c_str());
    } else if (stats.wifiApActive) {
        snprintf(stats.ipAddress, sizeof(stats.ipAddress), "%s", WiFi.softAPIP().toString().c_str());
        snprintf(stats.ssid, sizeof(stats.ssid), "%s", WiFi.softAPSSID().c_str());
    } else {
        snprintf(stats.ipAddress, sizeof(stats.ipAddress), "0.0.0.0");
        stats.ssid[0] = '\0';
    }

    snprintf(stats.macAddress, sizeof(stats.macAddress), "%s", WiFi.macAddress().c_str());

    stats.baudRate = g_serialBaudRate;
    stats.serialRxBytes = g_serialRxBytes;
    stats.serialTxBytes = g_serialTxBytes;
    stats.serialRxOverflow = g_serialRxOverflow;
    stats.activeWsClients = g_activeWsClients;
    stats.activeTelnetClients = g_activeTelnetClients;
    stats.ntpSynced = g_ntpSynced;
    stats.currentTime = time(nullptr);
}

void SystemStats::formatUptime(uint32_t totalSec, char *buf, size_t bufSize) {
    uint32_t days = totalSec / 86400;
    uint32_t hours = (totalSec % 86400) / 3600;
    uint32_t mins = (totalSec % 3600) / 60;
    uint32_t secs = totalSec % 60;

    if (days > 0) {
        snprintf(buf, bufSize, "%ud %uh %um %us", days, hours, mins, secs);
    } else if (hours > 0) {
        snprintf(buf, bufSize, "%uh %um %us", hours, mins, secs);
    } else {
        snprintf(buf, bufSize, "%um %us", mins, secs);
    }
}

void SystemStats::formatBytes(uint32_t bytes, char *buf, size_t bufSize) {
    if (bytes >= (1024 * 1024)) {
        snprintf(buf, bufSize, "%.2f MB", bytes / (1024.0f * 1024.0f));
    } else if (bytes >= 1024) {
        snprintf(buf, bufSize, "%.1f KB", bytes / 1024.0f);
    } else {
        snprintf(buf, bufSize, "%u B", bytes);
    }
}
