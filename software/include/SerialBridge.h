#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "Config.h"

class SerialBridge {
public:
    SerialBridge();
    void begin(Preferences &prefs);
    void loop();

    // Serial Hardware Configuration
    bool updateConfig(uint32_t baud, uint8_t dataBits, uint8_t parity, uint8_t stopBits);
    
    // Write data received from network to Serial port
    size_t write(const uint8_t *data, size_t len);
    size_t write(uint8_t byte);

    // Read data from RX ring buffer
    size_t readBytes(uint8_t *buffer, size_t maxLen);
    size_t available() const;
    void clearRx();

    // Configuration getters
    uint32_t getBaudRate() const { return _baudRate; }
    uint8_t  getDataBits() const { return _dataBits; }
    uint8_t  getParity()   const { return _parity; }
    uint8_t  getStopBits() const { return _stopBits; }
    bool     getForceEcho() const { return _forceEcho; }
    bool     getGreetingBanner() const { return _greetingBanner; }

    void setForceEcho(bool en) { _forceEcho = en; }
    void setGreetingBanner(bool en) { _greetingBanner = en; }

private:
    uint32_t _baudRate;
    uint8_t  _dataBits;
    uint8_t  _parity;
    uint8_t  _stopBits;
    bool     _forceEcho;
    bool     _greetingBanner;

    // Static Pre-allocated Circular Ring Buffer (Zero dynamic allocation)
    uint8_t  _rxRing[SERIAL_RX_RING_BUFFER_SIZE];
    volatile size_t _rxHead;
    volatile size_t _rxTail;
    volatile size_t _rxCount;

    uint32_t getSerialConfigCode(uint8_t dataBits, uint8_t parity, uint8_t stopBits) const;
};

extern SerialBridge serialBridge;
