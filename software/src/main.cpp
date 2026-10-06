#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <esp_sntp.h>
#include <esp_log.h>
#include <stdarg.h>
#include <stdio.h>

#include "Config.h"
#include "ConsoleLogger.h"
#include "SystemStats.h"
#include "SerialBridge.h"
#include "TelnetServer.h"
#include "WebTerminal.h"
#include "MndpDiscovery.h"
#include "WebPortal.h"

// =============================================================================
// Global State & Instances
// =============================================================================
WebServer   server(HTTP_PORT);
DNSServer   dnsServer;
Preferences preferences;
WebPortal   portal(server, dnsServer, preferences);

time_t g_lastNtpSyncTimestamp = 0;
bool   g_ntpSynced = false;

static uint32_t s_lastLedBlinkMillis = 0;
static bool     s_ledState = false;
static uint32_t s_lastNtpSyncTrigger = 0;
static bool     s_staWasConnected = false;

// UART0 is also the serial bridge to the managed device (GPIO1/3). Keep
// ESP-IDF diagnostics out of that byte stream and retain warnings/errors in
// the dashboard's System Event Log instead.
static int captureEspIdfLog(const char *format, va_list args) {
    char line[128];
    int length = vsnprintf(line, sizeof(line), format, args);
    if (length > 0) {
        // ESP-IDF's formatted log text already ends in a newline. Store a
        // single-line message because the event-log renderers add separators.
        size_t lineLength = strlen(line);
        while (lineLength > 0 &&
               (line[lineLength - 1] == '\r' || line[lineLength - 1] == '\n')) {
            line[--lineLength] = '\0';
        }

        const char *severity = line;
        while (*severity == '\033') {
            while (*severity && *severity != 'm') ++severity;
            if (*severity) ++severity;
        }
        if (*severity == 'E') {
            logger.logError("%s", line);
        } else if (*severity == 'W') {
            logger.logWarn("%s", line);
        }
    }
    return length;
}

// =============================================================================
// SNTP Notification Callback & Trigger Helper
// =============================================================================
void timeSyncCallback(struct timeval *tv) {
    g_lastNtpSyncTimestamp = time(nullptr);
    g_ntpSynced = true;
    logger.logInfo("NTP Time synchronized successfully.");
}

void triggerNtpSync() {
    bool ntpEn = preferences.getBool(NVS_KEY_NTP_ENABLED, true);
    if (!ntpEn) {
        if (sntp_enabled()) {
            sntp_stop();
        }
        logger.logInfo("NTP is disabled in configuration.");
        return;
    }
    String tzPosix = preferences.getString(NVS_KEY_NTP_TZ_POSIX, DEFAULT_TZ_POSIX);
    String ntpSrv = preferences.getString(NVS_KEY_NTP_SERVER, NTP_DEFAULT_SERVER);

    if (sntp_enabled()) {
        sntp_stop();
    }
    sntp_set_time_sync_notification_cb(timeSyncCallback);
    configTzTime(tzPosix.c_str(), ntpSrv.c_str(), "time.google.com", "time.cloudflare.com");
    logger.logInfo("NTP client started: %s, time.google.com, time.cloudflare.com (TZ: %s)", ntpSrv.c_str(), tzPosix.c_str());
}

// =============================================================================
// Wi-Fi Configuration Helper
// =============================================================================
void setupWiFi(Preferences &prefs) {
    String staSsid = prefs.getString(NVS_KEY_WIFI_SSID, "");
    String staPass = prefs.getString(NVS_KEY_WIFI_PASS, "");
    bool   staDhcp = prefs.getBool(NVS_KEY_WIFI_DHCP, true);

    String apSsid = prefs.getString(NVS_KEY_AP_SSID, "");
    String apPass = prefs.getString(NVS_KEY_AP_PASS, AP_DEFAULT_PASSWORD);
    uint8_t apChan = prefs.getUChar(NVS_KEY_AP_CHAN, AP_DEFAULT_CHANNEL);
    bool   apHidden = prefs.getBool(NVS_KEY_AP_HIDDEN, false);

    // If AP SSID not set, generate default "ESP-OOBM-XXXXXX" from MAC
    if (apSsid.length() == 0) {
        uint8_t mac[6];
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
        char suffix[8];
        snprintf(suffix, sizeof(suffix), "%02X%02X%02X", mac[3], mac[4], mac[5]);
        apSsid = String(AP_SSID_PREFIX) + suffix;
    }

    bool staConfigured = (staSsid.length() > 0);

    if (staConfigured) {
        WiFi.mode(WIFI_AP_STA);
        WiFi.setSleep(false);

        if (!staDhcp) {
            IPAddress ip, gw, sn, dns;
            ip.fromString(prefs.getString(NVS_KEY_WIFI_IP, "192.168.1.50"));
            gw.fromString(prefs.getString(NVS_KEY_WIFI_GW, "192.168.1.1"));
            sn.fromString(prefs.getString(NVS_KEY_WIFI_SN, "255.255.255.0"));
            dns.fromString(prefs.getString(NVS_KEY_WIFI_DNS, "1.1.1.1"));
            WiFi.config(ip, gw, sn, dns);
        }

        logger.logInfo("Connecting to Station Wi-Fi: %s ...", staSsid.c_str());
        WiFi.begin(staSsid.c_str(), staPass.c_str());

        // Wait up to 5s for STA connection
        uint32_t startMs = millis();
        while (WiFi.status() != WL_CONNECTED && (millis() - startMs < 5000)) {
            delay(100);
            yield();
        }

        if (WiFi.status() == WL_CONNECTED) {
            s_staWasConnected = true;
            logger.logInfo("Station Wi-Fi connected! IP: %s, RSSI: %d dBm", 
                           WiFi.localIP().toString().c_str(), WiFi.RSSI());
            triggerNtpSync();
        } else {
            logger.logWarn("Station Wi-Fi connection timed out. Starting AP mode.");
        }
    } else {
        WiFi.mode(WIFI_AP);
        WiFi.setSleep(false);
    }

    // Always start AP
    WiFi.softAPConfig(IPAddress(AP_IP_ADDRESS), IPAddress(AP_IP_ADDRESS), IPAddress(AP_NETMASK));
    const char *pass = (apPass.length() >= 8) ? apPass.c_str() : nullptr;
    WiFi.softAP(apSsid.c_str(), pass, apChan, apHidden);
    logger.logInfo("Access Point started! SSID: %s, IP: %s (Channel: %u)", apSsid.c_str(), WiFi.softAPIP().toString().c_str(), apChan);
}

// =============================================================================
// Status LED Helper
// =============================================================================
void setLedWifi(bool on) {
    digitalWrite(PIN_LED_STATUS, on ? LED_ACTIVE_LEVEL : !LED_ACTIVE_LEVEL);
}

void updateLed() {
    if (WiFi.status() == WL_CONNECTED) {
        // Station Connected: Solid ON
        setLedWifi(true);
    } else if (WiFi.getMode() & WIFI_MODE_AP) {
        // Standalone AP Mode: Slow blink (500ms cycle)
        if (millis() - s_lastLedBlinkMillis >= 500) {
            s_lastLedBlinkMillis = millis();
            s_ledState = !s_ledState;
            setLedWifi(s_ledState);
        }
    } else {
        // Connecting / Disconnected: Fast blink (200ms cycle)
        if (millis() - s_lastLedBlinkMillis >= 200) {
            s_lastLedBlinkMillis = millis();
            s_ledState = !s_ledState;
            setLedWifi(s_ledState);
        }
    }
}

// =============================================================================
// Setup Function
// =============================================================================
void setup() {
    pinMode(PIN_LED_STATUS, OUTPUT);
    digitalWrite(PIN_LED_STATUS, !LED_ACTIVE_LEVEL);

    // The default ESP-IDF console is UART0, shared with SerialBridge.
    // Capture runtime warnings/errors in the event log without injecting
    // diagnostic text into the managed device's serial session.
    esp_log_set_vprintf(captureEspIdfLog);

    preferences.begin(NVS_NAMESPACE, false);

    // Initialize Serial Console Bridge
    serialBridge.begin(preferences);
    logger.logInfo("ESP-OOBM Firmware v%s initialized.", FIRMWARE_VERSION);

    // Initialize Wi-Fi
    setupWiFi(preferences);

    // Initialize Hostname & mDNS
    String hostname = getDeviceHostname(preferences);
    if (MDNS.begin(hostname.c_str())) {
        MDNS.addService("http", "tcp", HTTP_PORT);
        MDNS.addService("telnet", "tcp", TELNET_PORT);
        MDNS.addService("oobm", "tcp", WEBSOCKET_PORT);
        logger.logInfo("mDNS responder started: http://%s.local", hostname.c_str());
    }

    // Initialize ArduinoOTA
    ArduinoOTA.setHostname(hostname.c_str());
    ArduinoOTA.setPort(OTA_PORT);
    ArduinoOTA.begin();

    // Initialize SNTP Time Client
    triggerNtpSync();

    // Initialize Telnet Server
    telnetServer.begin(preferences);

    // Initialize WebSocket Terminal
    webTerminal.begin(preferences);

    // Initialize MNDP Discovery
    mndpDiscovery.begin(preferences);

    // Initialize HTTP Web Portal Routes
    portal.begin();

    logger.logInfo("System ready. Listening for incoming connections.");
}

// =============================================================================
// Main Loop
// =============================================================================
void loop() {
    uint64_t loopStartUs = esp_timer_get_time();

    // 1. Drain Hardware UART into static ring buffer
    serialBridge.loop();

    // 2. Broadcast incoming Serial bytes to active WebSocket and Telnet clients
    size_t avail = serialBridge.available();
    if (avail > 0) {
        uint8_t streamBuf[256];
        size_t readLen = serialBridge.readBytes(streamBuf, sizeof(streamBuf));
        if (readLen > 0) {
            webTerminal.broadcast(streamBuf, readLen);
            telnetServer.broadcast(streamBuf, readLen);
        }
    }

    // 3. Process Network Daemons
    webTerminal.loop();
    telnetServer.loop();
    portal.loop();
    mndpDiscovery.loop();
    ArduinoOTA.handle();

    // 4. Handle Wi-Fi status transitions & Periodic NTP re-sync
    bool staConnected = (WiFi.status() == WL_CONNECTED);
    if (staConnected && !s_staWasConnected) {
        s_staWasConnected = true;
        triggerNtpSync();
    } else if (!staConnected && s_staWasConnected) {
        s_staWasConnected = false;
    }

    if (staConnected && (millis() - s_lastNtpSyncTrigger >= 3600000UL)) {
        s_lastNtpSyncTrigger = millis();
        triggerNtpSync();
    }

    // 5. Update Status LED
    updateLed();

    uint64_t loopEndUs = esp_timer_get_time();
    SystemStats::recordLoopActivity((uint32_t)(loopEndUs - loopStartUs));

    delay(1);
}
