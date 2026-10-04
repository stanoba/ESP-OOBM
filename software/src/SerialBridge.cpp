#include "SerialBridge.h"
#include "ConsoleLogger.h"

SerialBridge serialBridge;

uint32_t g_serialRxBytes = 0;
uint32_t g_serialTxBytes = 0;
uint32_t g_serialRxOverflow = 0;
uint32_t g_serialBaudRate = DEFAULT_BAUD_RATE;

SerialBridge::SerialBridge() 
    : _baudRate(DEFAULT_BAUD_RATE),
      _dataBits(DEFAULT_DATA_BITS),
      _parity(DEFAULT_PARITY),
      _stopBits(DEFAULT_STOP_BITS),
      _forceEcho(false),
      _greetingBanner(true),
      _rxHead(0),
      _rxTail(0),
      _rxCount(0) {
    memset(_rxRing, 0, sizeof(_rxRing));
}

uint32_t SerialBridge::getSerialConfigCode(uint8_t dataBits, uint8_t parity, uint8_t stopBits) const {
    // Arduino ESP32 Serial config flags: SERIAL_5N1 to SERIAL_8E2
    if (dataBits == 5) {
        if (parity == 1) return (stopBits == 2) ? SERIAL_5O2 : SERIAL_5O1;
        if (parity == 2) return (stopBits == 2) ? SERIAL_5E2 : SERIAL_5E1;
        return (stopBits == 2) ? SERIAL_5N2 : SERIAL_5N1;
    } else if (dataBits == 6) {
        if (parity == 1) return (stopBits == 2) ? SERIAL_6O2 : SERIAL_6O1;
        if (parity == 2) return (stopBits == 2) ? SERIAL_6E2 : SERIAL_6E1;
        return (stopBits == 2) ? SERIAL_6N2 : SERIAL_6N1;
    } else if (dataBits == 7) {
        if (parity == 1) return (stopBits == 2) ? SERIAL_7O2 : SERIAL_7O1;
        if (parity == 2) return (stopBits == 2) ? SERIAL_7E2 : SERIAL_7E1;
        return (stopBits == 2) ? SERIAL_7N2 : SERIAL_7N1;
    } else { // 8 data bits
        if (parity == 1) return (stopBits == 2) ? SERIAL_8O2 : SERIAL_8O1;
        if (parity == 2) return (stopBits == 2) ? SERIAL_8E2 : SERIAL_8E1;
        return (stopBits == 2) ? SERIAL_8N2 : SERIAL_8N1;
    }
}

void SerialBridge::begin(Preferences &prefs) {
    _baudRate = prefs.getUInt(NVS_KEY_SER_BAUD, DEFAULT_BAUD_RATE);
    _dataBits = prefs.getUChar(NVS_KEY_SER_DBITS, DEFAULT_DATA_BITS);
    _parity   = prefs.getUChar(NVS_KEY_SER_PARITY, DEFAULT_PARITY);
    _stopBits = prefs.getUChar(NVS_KEY_SER_SBITS, DEFAULT_STOP_BITS);
    _forceEcho = prefs.getBool(NVS_KEY_SER_ECHO, false);
    _greetingBanner = prefs.getBool(NVS_KEY_SER_BANNER, true);

    g_serialBaudRate = _baudRate;

    uint32_t config = getSerialConfigCode(_dataBits, _parity, _stopBits);
    Serial.end();
    Serial.setRxBufferSize(4096);
    Serial.begin(_baudRate, config, PIN_UART_RX, PIN_UART_TX);

    logger.logInfo("Serial initialized at %u baud (%u%c%u)", 
                   _baudRate, _dataBits, (_parity == 1 ? 'O' : (_parity == 2 ? 'E' : 'N')), _stopBits);
}

bool SerialBridge::updateConfig(uint32_t baud, uint8_t dataBits, uint8_t parity, uint8_t stopBits) {
    if (baud < 300 || baud > 921600) baud = DEFAULT_BAUD_RATE;
    if (dataBits < 5 || dataBits > 8) dataBits = 8;
    if (parity > 2) parity = 0;
    if (stopBits < 1 || stopBits > 2) stopBits = 1;

    _baudRate = baud;
    _dataBits = dataBits;
    _parity   = parity;
    _stopBits = stopBits;
    g_serialBaudRate = baud;

    uint32_t config = getSerialConfigCode(_dataBits, _parity, _stopBits);
    Serial.end();
    Serial.setRxBufferSize(4096);
    Serial.begin(_baudRate, config, PIN_UART_RX, PIN_UART_TX);

    logger.logInfo("Serial reconfigured to %u baud (%u%c%u)", 
                   _baudRate, _dataBits, (_parity == 1 ? 'O' : (_parity == 2 ? 'E' : 'N')), _stopBits);
    return true;
}

void SerialBridge::loop() {
    // Rapidly drain hardware UART buffer into static ring buffer
    while (Serial.available() > 0) {
        int byte = Serial.read();
        if (byte < 0) break;

        if (_rxCount < SERIAL_RX_RING_BUFFER_SIZE) {
            _rxRing[_rxHead] = (uint8_t)byte;
            _rxHead = (_rxHead + 1) % SERIAL_RX_RING_BUFFER_SIZE;
            _rxCount++;
            g_serialRxBytes++;
        } else {
            g_serialRxOverflow++;
            // Ring buffer overflow - drop oldest or current byte
            _rxTail = (_rxTail + 1) % SERIAL_RX_RING_BUFFER_SIZE;
            _rxRing[_rxHead] = (uint8_t)byte;
            _rxHead = (_rxHead + 1) % SERIAL_RX_RING_BUFFER_SIZE;
        }
    }
}

size_t SerialBridge::write(const uint8_t *data, size_t len) {
    if (!data || len == 0) return 0;
    size_t written = Serial.write(data, len);
    g_serialTxBytes += written;
    return written;
}

size_t SerialBridge::write(uint8_t byte) {
    size_t written = Serial.write(byte);
    g_serialTxBytes += written;
    return written;
}

size_t SerialBridge::readBytes(uint8_t *buffer, size_t maxLen) {
    if (!buffer || maxLen == 0 || _rxCount == 0) return 0;
    size_t bytesRead = 0;

    while (bytesRead < maxLen && _rxCount > 0) {
        buffer[bytesRead++] = _rxRing[_rxTail];
        _rxTail = (_rxTail + 1) % SERIAL_RX_RING_BUFFER_SIZE;
        _rxCount--;
    }
    return bytesRead;
}

size_t SerialBridge::available() const {
    return _rxCount;
}

void SerialBridge::clearRx() {
    _rxHead = 0;
    _rxTail = 0;
    _rxCount = 0;
}
