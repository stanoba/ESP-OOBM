# Software & Firmware Architecture Guide

This document provides a comprehensive breakdown of the firmware architecture, FreeRTOS tasks, C++ modules, memory partitions, and network services of the **ESP-OOBM** wireless console bridge.

---

## 1. System Architecture

```mermaid
flowchart TD
    subgraph Hardware["Hardware Layer"]
        UART0["ESP32 UART0 Controller<br/>GPIO1 (TXD) / GPIO3 (RXD)"]
        WIFI_RADIO["2.4 GHz Wi-Fi Radio<br/>AP + STA Concurrent Mode"]
        NVS_FLASH["SPI Flash Memory<br/>NVS (Preferences) + Dual OTA"]
    end

    subgraph Core["Core Firmware Modules"]
        SB["SerialBridge<br/>Hardware FIFO & Ring Buffer"]
        SYS["SystemStats<br/>CPU, Heap & RSSI Metrics"]
        LOG["ConsoleLogger<br/>In-Memory Event Ring Buffer"]
    end

    subgraph Daemons["Network Daemons & Protocols"]
        WS["WebTerminal Server<br/>WebSocket (Port 81)"]
        TEL["Telnet Server<br/>RFC 854 (Port 23)"]
        WEB["Async WebPortal & REST API<br/>HTTP (Port 80)"]
        PROM["Prometheus Exporter<br/>/metrics (Port 80)"]
        MNDP["MikroTik MNDP Engine<br/>UDP Broadcast (Port 5678)"]
        DNS["Captive Portal DNS<br/>UDP (Port 53)"]
        OTA["ArduinoOTA & WebOTA<br/>Port 3232 / /update"]
    end

    UART0 <-->|"115200 baud Stream"| SB
    SB <-->|"Bidirectional Console"| WS
    SB <-->|"Bidirectional Console"| TEL
    SYS -->|"Telemetry"| PROM
    SYS -->|"Telemetry"| WEB
    LOG -->|"System Events"| WEB
    NVS_FLASH <-->|"Persistent Config"| WEB
    WIFI_RADIO <--> Daemons
```

---

## 2. Firmware Modules & Source Files

All firmware source code is located in the [`software/`](../software/) directory.

| Module | Header | Implementation | Description |
| :--- | :--- | :--- | :--- |
| **SerialBridge** | [`SerialBridge.h`](../software/include/SerialBridge.h) | [`SerialBridge.cpp`](../software/src/SerialBridge.cpp) | Manages non-blocking hardware UART0 communication, FIFO drain, baud rate configuration, parity, stop bits, and TX/RX byte accounting. |
| **WebTerminal** | [`WebTerminal.h`](../software/include/WebTerminal.h) | [`WebTerminal.cpp`](../software/src/WebTerminal.cpp) | WebSocket server on port 81 providing full-duplex ANSI terminal bridging with platform quick-command macro expansion and session history. |
| **TelnetServer** | [`TelnetServer.h`](../software/include/TelnetServer.h) | [`TelnetServer.cpp`](../software/src/TelnetServer.cpp) | Standalone RFC 854 Telnet daemon on port 23 supporting multiple concurrent clients, password authentication, and NVT control negotiation. |
| **WebPortal** | [`WebPortal.h`](../software/include/WebPortal.h) | [`WebPortal.cpp`](../software/src/WebPortal.cpp) | HTTP web server serving the responsive dark/light management UI, Captive Portal DNS redirection, and JSON REST API (`/api/*`). |
| **PrometheusExporter** | [`PrometheusExporter.h`](../software/include/PrometheusExporter.h) | [`PrometheusExporter.cpp`](../software/src/PrometheusExporter.cpp) | Standard OpenMetrics HTTP exporter endpoint on `/metrics` with uptime, RAM, CPU load, Wi-Fi RSSI, and serial I/O telemetry. |
| **MndpDiscovery** | [`MndpDiscovery.h`](../software/include/MndpDiscovery.h) | [`MndpDiscovery.cpp`](../software/src/MndpDiscovery.cpp) | Broadcasts MikroTik Neighbor Discovery Protocol (MNDP) UDP packets every 60s for automatic device discovery in MikroTik Winbox. |
| **ConsoleLogger** | [`ConsoleLogger.h`](../software/include/ConsoleLogger.h) | [`ConsoleLogger.cpp`](../software/src/ConsoleLogger.cpp) | In-memory circular log buffer holding the last 100 system events with ISO 8601 timestamps and severity levels (INFO, WARN, ERROR). |
| **SystemStats** | [`SystemStats.h`](../software/include/SystemStats.h) | [`SystemStats.cpp`](../software/src/SystemStats.cpp) | Calculates real-time CPU load estimations, heap fragmentation index, minimum free heap, uptime, and Wi-Fi signal quality. |

---

## 3. Flash Memory & Partition Layout

The ESP32-PICO-D4 includes 4 MB (4096 KB) embedded SPI flash. The partitioning scheme is configured in [`partitions_custom.csv`](../software/partitions_custom.csv):

```
# ESP32 Custom 4MB Partition Table with Dual OTA
# Name,   Type, SubType,  Offset,    Size,     Flags
nvs,      data, nvs,      0x9000,    0x5000,   
otadata,  data, ota,      0xe000,    0x2000,   
app0,     app,  ota_0,    0x10000,   0x1E0000, 
app1,     app,  ota_1,    0x1F0000,  0x1E0000, 
spiffs,   data, spiffs,   0x3D0000,  0x30000,  
```

### Partition Map Breakdown

| Partition | Offset | Size | Purpose |
| :--- | :---: | :---: | :--- |
| **`nvs`** | `0x009000` | 20 KB | Non-Volatile Storage for runtime preferences (SSID, passwords, baud rate, hostname) |
| **`otadata`** | `0x00E000` | 8 KB | OTA boot selector tracking active vs. pending update partitions |
| **`app0`** | `0x010000` | 1920 KB | Primary firmware slot (Active / Rollback) |
| **`app1`** | `0x1F0000` | 1920 KB | Secondary firmware slot (Seamless OTA target) |
| **`spiffs`** | `0x3D0000` | 192 KB | Static storage / SPIFFS filesystem |

---

## 4. Network Daemons & Services

| Service | Protocol / Transport | Port | Authentication | Notes |
| :--- | :--- | :---: | :--- | :--- |
| **Web Dashboard** | HTTP (1.1) | `80` | Form Login / Session Cookie | Responsive dashboard, logs, and configuration portal |
| **WebTerminal** | WebSocket (Binary / Text) | `81` | Inherits Web Session Cookie | Full-duplex bidirectional ANSI serial console stream |
| **Telnet Daemon** | RFC 854 (TCP) | `23` | Password Challenge Prompt | Standalone terminal client access (PuTTY, Linux `telnet`) |
| **Prometheus Exporter** | HTTP (`GET /metrics`) | `80` | HTTP Basic Auth | OpenMetrics format telemetry scraper endpoint |
| **Captive Portal DNS** | DNS (UDP) | `53` | None | Wildcard DNS answering `192.168.4.1` for standalone AP mode |
| **MikroTik MNDP** | UDP Broadcast | `5678` | None | Winbox discovery broadcasts on `255.255.255.255` |
| **ArduinoOTA** | TCP / Custom | `3232` | Digest Hash | Wireless flash uploading directly from PlatformIO CLI |

---

## 5. Non-Volatile Storage (NVS) Configuration Keys

All settings are persisted across reboots in ESP32 NVS using the Arduino `Preferences` library (`NVS_NAMESPACE = "oobm"`):

| NVS Key Constant | Key String | Data Type | Default Value | Description |
| :--- | :--- | :---: | :--- | :--- |
| `NVS_KEY_WIFI_SSID` | `"w_ssid"` | String | `""` | Station Wi-Fi network SSID |
| `NVS_KEY_WIFI_PASS` | `"w_pass"` | String | `""` | Station Wi-Fi network WPA2 password |
| `NVS_KEY_WIFI_DHCP` | `"w_dhcp"` | Bool | `true` | Use DHCP (`true`) or Static IP (`false`) |
| `NVS_KEY_WIFI_IP` | `"w_ip"` | String | `"192.168.1.50"` | Static IP address (when DHCP disabled) |
| `NVS_KEY_WIFI_GW` | `"w_gw"` | String | `"192.168.1.1"` | Gateway IP address |
| `NVS_KEY_WIFI_SN` | `"w_sn"` | String | `"255.255.255.0"`| Subnet mask |
| `NVS_KEY_WIFI_DNS` | `"w_dns"` | String | `"1.1.1.1"` | Primary DNS server |
| `NVS_KEY_AP_SSID` | `"ap_ssid"` | String | `"ESP-OOBM-XXXXXX"`| Fallback Access Point SSID (auto-generated from MAC) |
| `NVS_KEY_AP_PASS` | `"ap_pass"` | String | `"oobmadm123"` | Fallback Access Point WPA2 passphrase |
| `NVS_KEY_AP_CHAN` | `"ap_chan"` | UChar | `1` | Wi-Fi Access Point radio channel (1–13) |
| `NVS_KEY_AP_HIDDEN` | `"ap_hid"` | Bool | `false` | Hide AP SSID beacon broadcast |
| `NVS_KEY_AUTH_ENABLED`| `"auth_en"`| Bool | `true` | Enable Web Portal & API authentication |
| `NVS_KEY_ADMIN_USER` | `"adm_usr"` | String | `"admin"` | Web Portal & REST API username |
| `NVS_KEY_ADMIN_PASS` | `"adm_pwd"` | String | `"oobmadm123"` | Web Portal & REST API password |
| `NVS_KEY_TELNET_EN` | `"tel_en"` | Bool | `true` | Enable standalone Telnet daemon on port 23 |
| `NVS_KEY_TELNET_PASS` | `"tel_pwd"` | String | `"oobmadm123"` | Telnet login challenge password |
| `NVS_KEY_BAUDRATE` | `"uart_br"` | UInt | `115200` | UART0 baud rate (300 to 921600 bps) |
| `NVS_KEY_DATABITS` | `"uart_db"` | UChar | `8` | Data bits (5, 6, 7, 8) |
| `NVS_KEY_PARITY` | `"uart_pr"` | UChar | `0` (None) | Parity (`0`=None, `1`=Odd, `2`=Even) |
| `NVS_KEY_STOPBITS` | `"uart_sb"` | UChar | `1` | Stop bits (1, 2) |
| `NVS_KEY_DEV_NAME` | `"dev_name"`| String | `"ESP-OOBM"` | mDNS and MNDP broadcast device hostname |
| `NVS_KEY_NTP_ENABLED` | `"ntp_en"` | Bool | `true` | Automatic SNTP time synchronization |
| `NVS_KEY_NTP_SERVER` | `"ntp_srv"` | String | `"pool.ntp.org"` | Primary NTP server hostname |
| `NVS_KEY_NTP_TZ_POSIX`| `"ntp_tzp"` | String | `"CET-1CEST,M3.5.0,M10.5.0/3"` | POSIX timezone string (Default: Europe/Bratislava) |

---

## 6. Building & Flashing

### Requirements
* [PlatformIO Core (CLI)](https://docs.platformio.org/en/latest/core/index.html) or PlatformIO IDE (VS Code extension).
* Espressif 32 platform package (`^6.12.0`).

### Compilation & Flash Commands
```powershell
# Navigate to software directory
cd software

# Compile firmware binary
pio run -e esp32_pico_d4

# Flash firmware via USB (CH343P auto-reset handled automatically)
pio run -e esp32_pico_d4 -t upload

# Open serial monitor for initial debug output
pio device monitor -b 115200
```

### Over-The-Air (OTA) Updates
1. **Web OTA**: Navigate to `http://192.168.4.1/update` in your browser and upload the compiled `.pio/build/esp32_pico_d4/firmware.bin`.
2. **ArduinoOTA (CLI)**:
   ```powershell
   pio run -e esp32_pico_d4 -t upload --upload-port 192.168.1.150
   ```
