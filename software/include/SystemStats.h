#pragma once
#include <Arduino.h>
#include <WiFi.h>

struct SystemStatsData {
    uint32_t uptimeSeconds;
    uint32_t cpuFreqMhz;
    float    cpuLoadPercent;
    float    temperatureCelsius;
    uint8_t  resetReasonCode;
    char     resetReasonStr[24];
    uint32_t freeHeapBytes;
    uint32_t minFreeHeapBytes;
    uint32_t maxAllocHeapBytes;
    uint8_t  heapFragPercent;
    uint32_t flashSizeBytes;
    uint32_t sketchSizeBytes;
    uint32_t freeSketchSpaceBytes;
    int8_t   wifiRssi;
    bool     wifiStaConnected;
    bool     wifiApActive;
    uint8_t  wifiApClients;
    char     ipAddress[20];
    char     macAddress[20];
    char     ssid[34];
    uint32_t baudRate;
    uint32_t serialRxBytes;
    uint32_t serialTxBytes;
    uint32_t serialRxOverflow;
    uint16_t activeWsClients;
    uint16_t activeTelnetClients;
    uint16_t activeTlsSockets;
    bool     ntpSynced;
    time_t   currentTime;
};

class SystemStats {
public:
    static void update(SystemStatsData &stats);
    static void recordLoopActivity(uint32_t busyMicros);
    static void formatUptime(uint32_t totalSec, char *buf, size_t bufSize);
    static void formatBytes(uint32_t bytes, char *buf, size_t bufSize);
};
