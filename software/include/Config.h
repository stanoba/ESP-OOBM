#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

// =============================================================================
// Firmware Version & Identity Metadata
// =============================================================================
#define FIRMWARE_NAME               "ESP-OOBM"
#define FIRMWARE_DESCRIPTION        "Wireless Out-of-Band Management Dongle"
#define FIRMWARE_VERSION            "1.0.1"
#define FIRMWARE_BUILD_DATE         __DATE__
#define FIRMWARE_BUILD_TIME         __TIME__
#define DEFAULT_HOSTNAME            "esp-oobm"

// =============================================================================
// Hardware Pin Definitions (ESP32-PICO-D4 + CH343P Dongle)
// =============================================================================
#define PIN_UART_TX                 1    // GPIO1 (U0TXD -> CH343P RXD)
#define PIN_UART_RX                 3    // GPIO3 (U0RXD <- CH343P TXD)
#define PIN_LED_STATUS              10   // GPIO10 (On-board Blue LED D3, Active-LOW)
#define LED_ACTIVE_LEVEL            LOW  // LOW = ON, HIGH = OFF
// Note: EN & GPIO0 are controlled via CH343P DTR/RTS auto-download transistor circuit (Q1/Q2 S8050)

void setLedWifi(bool on);

// =============================================================================
// Serial Bridge Buffer & Performance Defaults
// =============================================================================
#define DEFAULT_BAUD_RATE           115200
#define DEFAULT_DATA_BITS           8
#define DEFAULT_PARITY              0    // 0 = None, 1 = Odd, 2 = Even
#define DEFAULT_STOP_BITS           1

#define SERIAL_RX_RING_BUFFER_SIZE  8192
#define SERIAL_TX_RING_BUFFER_SIZE  2048
#define SERIAL_FLUSH_INTERVAL_MS    5    // Coalescing debounce for network packets

// =============================================================================
// Network & Protocol Ports
// =============================================================================
#define HTTP_PORT                   80
#define WEBSOCKET_PORT              81
#define TELNET_PORT                 23
#define DNS_PORT                    53
#define MNDP_PORT                   5678
#define OTA_PORT                    3232
#define DASHBOARD_AJAX_REFRESH_MS   5000

// =============================================================================
// Access Point & Captive Portal Defaults
// =============================================================================
#define AP_SSID_PREFIX              "ESP-OOBM-"
#define AP_DEFAULT_PASSWORD         "oobmadm123"  // Default WPA2-PSK password
#define AP_DEFAULT_CHANNEL          1
#define AP_IP_ADDRESS               192, 168, 4, 1
#define AP_NETMASK                  255, 255, 255, 0

// =============================================================================
// Security & Authentication Defaults
// =============================================================================
#define DEFAULT_AUTH_ENABLED        true
#define DEFAULT_AUTH_USER           "admin"
#define DEFAULT_AUTH_PASS           "oobmadm123"
#define DEFAULT_TELNET_AUTH         true
#define DEFAULT_TELNET_PASS         "oobmadm123"

// =============================================================================
// SNTP Time Synchronization Defaults
// =============================================================================
#define NTP_DEFAULT_SERVER          "pool.ntp.org"
#define DEFAULT_TZ_POSIX            "CET-1CEST,M3.5.0,M10.5.0/3" // Europe/Bratislava
#define DEFAULT_TZ_CITY             "Europe/Bratislava (UTC+1, CEST)"
#define NTP_SYNC_INTERVAL_SEC       3600

// =============================================================================
// NVS Storage Keys (Namespace: "oobm_cfg")
// =============================================================================
#define NVS_NAMESPACE               "oobm_cfg"

// System & Device
#define NVS_KEY_HOSTNAME            "hostname"
#define NVS_KEY_MNDP_EN             "mndp_en"
#define NVS_KEY_CLI_PLATFORM        "cli_plat"

// Authentication
#define NVS_KEY_AUTH_EN             "auth_en"
#define NVS_KEY_AUTH_USER           "auth_user"
#define NVS_KEY_AUTH_PASS           "auth_pass"

// Serial Bridge
#define NVS_KEY_SER_BAUD            "ser_baud"
#define NVS_KEY_SER_DBITS           "ser_dbits"
#define NVS_KEY_SER_PARITY          "ser_parity"
#define NVS_KEY_SER_SBITS           "ser_sbits"
#define NVS_KEY_SER_ECHO            "ser_echo"
#define NVS_KEY_SER_BANNER          "ser_banner"

// Telnet Server
#define NVS_KEY_TELNET_EN           "tel_en"
#define NVS_KEY_TELNET_PORT         "tel_port"
#define NVS_KEY_TELNET_AUTH         "tel_auth"
#define NVS_KEY_TELNET_PASS         "tel_pass"

// Wi-Fi Station Mode
#define NVS_KEY_WIFI_SSID           "sta_ssid"
#define NVS_KEY_WIFI_PASS           "sta_pass"
#define NVS_KEY_WIFI_DHCP           "sta_dhcp"
#define NVS_KEY_WIFI_IP             "sta_ip"
#define NVS_KEY_WIFI_GW             "sta_gw"
#define NVS_KEY_WIFI_SN             "sta_sn"
#define NVS_KEY_WIFI_DNS            "sta_dns"

// Wi-Fi Access Point Mode
#define NVS_KEY_AP_SSID             "ap_ssid"
#define NVS_KEY_AP_PASS             "ap_pass"
#define NVS_KEY_AP_CHAN             "ap_chan"
#define NVS_KEY_AP_HIDDEN           "ap_hidden"
#define NVS_KEY_AP_CAPTIVE          "ap_captive"

// NTP Time
#define NVS_KEY_NTP_ENABLED         "ntp_en"
#define NVS_KEY_NTP_SERVER          "ntp_srv"
#define NVS_KEY_NTP_TZ_POSIX        "ntp_tz"
#define NVS_KEY_NTP_TZ_CITY         "ntp_city"
#define NVS_KEY_TIME_FORMAT_24H     "time_24h"

// =============================================================================
// Hostname Generation & Retrieval
// =============================================================================
inline String getDefaultHostname() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String suffix = (mac.length() >= 6) ? mac.substring(mac.length() - 6) : "000000";
    suffix.toLowerCase();
    return "esp-oobm-" + suffix;
}

inline String getDeviceHostname(Preferences &prefs) {
    String host = prefs.getString(NVS_KEY_HOSTNAME, "");
    host.trim();
    if (host.length() == 0 || host.equalsIgnoreCase(DEFAULT_HOSTNAME)) {
        return getDefaultHostname();
    }
    host.toLowerCase();
    return host;
}
