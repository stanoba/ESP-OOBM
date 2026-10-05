#include "ConsoleLogger.h"
#include <time.h>
#include <stdarg.h>
#include <stdio.h>

ConsoleLogger logger;
static portMUX_TYPE s_loggerMux = portMUX_INITIALIZER_UNLOCKED;

ConsoleLogger::ConsoleLogger() : _head(0), _count(0) {
    memset(_entries, 0, sizeof(_entries));
}

void ConsoleLogger::log(LogLevel level, const char *fmt, ...) {
    time_t now = time(nullptr);
    uint32_t ts = (now > 1577836800) ? (uint32_t)now : (millis() / 1000);
    LogEntry entry = {};
    entry.timestamp = ts;
    entry.level = (uint8_t)level;

    va_list args;
    va_start(args, fmt);
    vsnprintf(entry.msg, sizeof(entry.msg), fmt, args);
    va_end(args);

    portENTER_CRITICAL(&s_loggerMux);
    _entries[_head] = entry;
    _head = (_head + 1) % MAX_LOG_ENTRIES;
    if (_count < MAX_LOG_ENTRIES) {
        _count++;
    }
    portEXIT_CRITICAL(&s_loggerMux);
}

void ConsoleLogger::logInfo(const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    log(LOG_LVL_INFO, "%s", buf);
}

void ConsoleLogger::logWarn(const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    log(LOG_LVL_WARN, "%s", buf);
}

void ConsoleLogger::logError(const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    log(LOG_LVL_ERROR, "%s", buf);
}

size_t ConsoleLogger::getCount() const {
    portENTER_CRITICAL(&s_loggerMux);
    size_t count = _count;
    portEXIT_CRITICAL(&s_loggerMux);
    return count;
}

LogEntry ConsoleLogger::getEntry(size_t index) const {
    LogEntry entry = {};
    portENTER_CRITICAL(&s_loggerMux);
    if (_count == 0) {
        portEXIT_CRITICAL(&s_loggerMux);
        return entry;
    }
    if (index >= _count) index = _count - 1;
    size_t start = (_head >= _count) ? (_head - _count) : (MAX_LOG_ENTRIES + _head - _count);
    size_t actualIdx = (start + index) % MAX_LOG_ENTRIES;
    entry = _entries[actualIdx];
    portEXIT_CRITICAL(&s_loggerMux);
    return entry;
}
