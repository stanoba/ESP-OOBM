#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>

enum LogLevel : uint8_t {
    LOG_LVL_INFO = 0,
    LOG_LVL_WARN = 1,
    LOG_LVL_ERROR = 2,
    LOG_LVL_DEBUG = 3
};

struct LogEntry {
    uint32_t timestamp;  // Epoch seconds if NTP synced, else uptime seconds
    uint8_t  level;
    char     msg[128];
};

class ConsoleLogger {
public:
    static const size_t MAX_LOG_ENTRIES = 80;

    ConsoleLogger();
    void log(LogLevel level, const char *fmt, ...);
    void logInfo(const char *fmt, ...);
    void logWarn(const char *fmt, ...);
    void logError(const char *fmt, ...);

    size_t getCount() const;
    LogEntry getEntry(size_t index) const; // 0 is oldest, count-1 is newest

private:
    LogEntry _entries[MAX_LOG_ENTRIES];
    size_t   _head;
    size_t   _count;
};

extern ConsoleLogger logger;
