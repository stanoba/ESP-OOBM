#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <esp_sntp.h>

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
static uint32_t s_buttonPressStart = 0;
static bool     s_buttonPressed = false;
static uint32_t s_lastNtpSyncTrigger = 0;
static bool     s_staWasConnected = false;

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
        String mac = WiFi.macAddress();
        mac.replace(":", "");
        apSsid = String(AP_SSID_PREFIX) + mac.substring(mac.length() - 6);
    }

    bool staConfigured = (staSsid.length() > 0);

    if (staConfigured) {
        WiFi.mode(WIFI_AP_STA);
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

        // Wait up to 8s for STA connection
        uint32_t startMs = millis();
        while (WiFi.status() != WL_CONNECTED && (millis() - startMs < 8000)) {
            delay(200);
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
    }

    // Always start AP if not connected or if AP mode configured
    WiFi.softAPConfig(IPAddress(AP_IP_ADDRESS), IPAddress(AP_IP_ADDRESS), IPAddress(AP_NETMASK));
    WiFi.softAP(apSsid.c_str(), apPass.length() > 0 ? apPass.c_str() : nullptr, apChan, apHidden);
    logger.logInfo("Access Point started! SSID: %s, IP: %s", apSsid.c_str(), WiFi.softAPIP().toString().c_str());
}

// =============================================================================
// Hardware Button & LED Helper
// =============================================================================
void handleButton() {
    bool pressed = (digitalRead(PIN_BUTTON) == LOW);

    if (pressed && !s_buttonPressed) {
        s_buttonPressed = true;
        s_buttonPressStart = millis();
    } else if (!pressed && s_buttonPressed) {
        uint32_t duration = millis() - s_buttonPressStart;
        s_buttonPressed = false;

        if (duration >= 5000) {
            // Long Press > 5s -> Factory Reset
            logger.logWarn("Button Long Press detected (>5s)! Performing Factory Reset...");
            preferences.clear();
            delay(500);
            ESP.restart();
        } else if (duration >= 100) {
            // Short Press -> Broadcast MNDP discovery announcement
            logger.logInfo("Button Short Press: Sending MNDP announcement.");
            mndpDiscovery.sendAnnouncement();
        }
    }
}

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
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_LED_STATUS, OUTPUT);
    digitalWrite(PIN_LED_STATUS, !LED_ACTIVE_LEVEL);

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

    // Initialize Web Portal & HTTP Routes
    portal.begin();

    logger.logInfo("System ready. Listening for incoming connections.");
}

// =============================================================================
// Main Loop
// =============================================================================
void loop() {
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

    // 5. Handle Hardware Button & Activity LED
    handleButton();
    updateLed();

    yield();
}
