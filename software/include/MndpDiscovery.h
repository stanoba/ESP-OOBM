#pragma once
#include <Arduino.h>
#include <WiFiUdp.h>
#include <Preferences.h>
#include "Config.h"

class MndpDiscovery {
public:
    MndpDiscovery();
    void begin(Preferences &prefs);
    void loop();
    void sendAnnouncement();

    bool isEnabled() const { return _enabled; }
    void setEnabled(bool en) { _enabled = en; }
    void setHostname(const char *name);

private:
    bool     _enabled;
    char     _hostname[32];
    WiFiUDP  _udp;
    uint32_t _lastBroadcastMillis;

    void encodeTLV(uint8_t *buf, size_t &offset, uint16_t type, const uint8_t *val, uint16_t len);
    void encodeStringTLV(uint8_t *buf, size_t &offset, uint16_t type, const char *str);
};

extern MndpDiscovery mndpDiscovery;
