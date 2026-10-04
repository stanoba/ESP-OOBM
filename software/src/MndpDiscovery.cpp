#include "MndpDiscovery.h"
#include "ConsoleLogger.h"

MndpDiscovery mndpDiscovery;

MndpDiscovery::MndpDiscovery() 
    : _enabled(true),
      _lastBroadcastMillis(0) {
    strncpy(_hostname, DEFAULT_HOSTNAME, sizeof(_hostname) - 1);
    _hostname[sizeof(_hostname) - 1] = '\0';
}

void MndpDiscovery::begin(Preferences &prefs) {
    _enabled = prefs.getBool(NVS_KEY_MNDP_EN, true);
    String host = prefs.getString(NVS_KEY_HOSTNAME, DEFAULT_HOSTNAME);
    strncpy(_hostname, host.c_str(), sizeof(_hostname) - 1);
    _hostname[sizeof(_hostname) - 1] = '\0';

    if (_enabled) {
        _udp.begin(MNDP_PORT);
        logger.logInfo("MNDP Neighbor Discovery active (Broadcast UDP :5678)");
        sendAnnouncement();
    }
}

void MndpDiscovery::setHostname(const char *name) {
    if (name && strlen(name) > 0) {
        strncpy(_hostname, name, sizeof(_hostname) - 1);
        _hostname[sizeof(_hostname) - 1] = '\0';
    }
}

void MndpDiscovery::encodeTLV(uint8_t *buf, size_t &offset, uint16_t type, const uint8_t *val, uint16_t len) {
    buf[offset++] = (uint8_t)(type >> 8);
    buf[offset++] = (uint8_t)(type & 0xFF);
    buf[offset++] = (uint8_t)(len >> 8);
    buf[offset++] = (uint8_t)(len & 0xFF);
    if (val && len > 0) {
        memcpy(buf + offset, val, len);
        offset += len;
    }
}

void MndpDiscovery::encodeStringTLV(uint8_t *buf, size_t &offset, uint16_t type, const char *str) {
    if (!str) return;
    uint16_t len = (uint16_t)strlen(str);
    encodeTLV(buf, offset, type, (const uint8_t*)str, len);
}

void MndpDiscovery::sendAnnouncement() {
    if (!_enabled) return;
    if (WiFi.status() != WL_CONNECTED && !(WiFi.getMode() & WIFI_MODE_AP)) return;

    uint8_t packet[384];
    memset(packet, 0, sizeof(packet));
    size_t offset = 0;

    // MNDP Header: 4 zero bytes
    packet[offset++] = 0x00;
    packet[offset++] = 0x00;
    packet[offset++] = 0x00;
    packet[offset++] = 0x00;

    // TLV 1: MAC Address (6 bytes)
    uint8_t mac[6];
    WiFi.macAddress(mac);
    encodeTLV(packet, offset, 0x0001, mac, 6);

    // TLV 5: Identity / Hostname
    encodeStringTLV(packet, offset, 0x0005, _hostname);

    // TLV 7: Version
    char verBuf[48];
    snprintf(verBuf, sizeof(verBuf), "%s v%s", FIRMWARE_NAME, FIRMWARE_VERSION);
    encodeStringTLV(packet, offset, 0x0007, verBuf);

    // TLV 8: Platform / Architecture
    encodeStringTLV(packet, offset, 0x0008, "ESP32-PICO-D4");

    // TLV 10: Uptime (4 bytes seconds, Little-Endian)
    uint32_t uptimeSec = millis() / 1000;
    uint8_t uptimeBytes[4];
    uptimeBytes[0] = (uint8_t)(uptimeSec & 0xFF);
    uptimeBytes[1] = (uint8_t)((uptimeSec >> 8) & 0xFF);
    uptimeBytes[2] = (uint8_t)((uptimeSec >> 16) & 0xFF);
    uptimeBytes[3] = (uint8_t)((uptimeSec >> 24) & 0xFF);
    encodeTLV(packet, offset, 0x000A, uptimeBytes, 4);

    // TLV 12: Hardware Board Name
    encodeStringTLV(packet, offset, 0x000C, "ESP32 KEY V1.0");

    // TLV 16: Interface Name
    encodeStringTLV(packet, offset, 0x0010, "wlan0");

    // Broadcast packet to 255.255.255.255:5678
    _udp.beginPacket(IPAddress(255, 255, 255, 255), MNDP_PORT);
    _udp.write(packet, offset);
    _udp.endPacket();

    _lastBroadcastMillis = millis();
}

void MndpDiscovery::loop() {
    if (!_enabled) return;

    // Send announcement every 60 seconds
    if (millis() - _lastBroadcastMillis >= 60000UL) {
        sendAnnouncement();
    }
}
