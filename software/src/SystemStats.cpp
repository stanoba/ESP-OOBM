#include "SystemStats.h"
#include <esp_heap_caps.h>
#include <esp_system.h>
#include <time.h>
#include <stdio.h>

extern uint32_t g_serialRxBytes;
extern uint32_t g_serialTxBytes;
extern uint32_t g_serialRxOverflow;
extern uint32_t g_serialBaudRate;
extern uint16_t g_activeWsClients;
extern uint16_t g_activeTelnetClients;
extern bool     g_ntpSynced;

static uint64_t s_lastCpuCalcTime = 0;
static uint32_t s_accumulatedBusyUs = 0;
static float    s_lastCpuLoadPercent = 0.0f;

static const char* getResetReasonString(esp_reset_reason_t r) {
    switch (r) {
        case ESP_RST_POWERON:   return "Power-On";
        case ESP_RST_EXT:       return "External Pin";
        case ESP_RST_SW:        return "Software Reset";
        case ESP_RST_PANIC:     return "Software Crash/Panic";
        case ESP_RST_INT_WDT:   return "Interrupt Watchdog";
        case ESP_RST_TASK_WDT:  return "Task Watchdog";
        case ESP_RST_WDT:       return "Other Watchdog";
        case ESP_RST_DEEPSLEEP: return "Deep Sleep Wakeup";
        case ESP_RST_BROWNOUT:  return "Brownout (Voltage Drop)";
        case ESP_RST_SDIO:      return "SDIO Reset";
        default:                return "Unknown";
    }
}

void SystemStats::recordLoopActivity(uint32_t busyMicros) {
    s_accumulatedBusyUs += busyMicros;
    uint64_t now = esp_timer_get_time();
    if (s_lastCpuCalcTime == 0) s_lastCpuCalcTime = now;
    if (now - s_lastCpuCalcTime >= 1000000ULL) {
        uint64_t interval = now - s_lastCpuCalcTime;
        if (interval > 0) {
            float load = ((float)s_accumulatedBusyUs / (float)interval) * 100.0f;
            if (load > 100.0f) load = 100.0f;
            s_lastCpuLoadPercent = load;
        }
        s_accumulatedBusyUs = 0;
        s_lastCpuCalcTime = now;
    }
}

void SystemStats::update(SystemStatsData &stats) {
    stats.uptimeSeconds = millis() / 1000;
    stats.cpuFreqMhz = getCpuFrequencyMhz();
    stats.cpuLoadPercent = s_lastCpuLoadPercent;
    stats.temperatureCelsius = temperatureRead();

    esp_reset_reason_t rst = esp_reset_reason();
    stats.resetReasonCode = (uint8_t)rst;
    strncpy(stats.resetReasonStr, getResetReasonString(rst), sizeof(stats.resetReasonStr) - 1);
    stats.resetReasonStr[sizeof(stats.resetReasonStr) - 1] = '\0';

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
