# ESP-OOBM: Wireless Out-of-Band Management Dongle

[![PlatformIO Build](https://img.shields.io/badge/PlatformIO-ESP32--PICO--D4-orange.svg)](https://platformio.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Firmware Version](https://img.shields.io/badge/Version-v1.0.0-emerald.svg)](software/include/Config.h)
[![Form Factor](https://img.shields.io/badge/Hardware-ESP32%20KEY%20V1.0-teal.svg)](hardware/README.md)

**ESP-OOBM** is an open-source, high-performance, wireless **Out-of-Band (OOB) Management** console firmware designed for the **ESP32-PICO-D4 USB Key (ESP32 KEY V1.0)** with **CH343P USB-to-UART bridge**.

When plugged into the USB port of a router, switch, firewall, or server, the dongle acts as a secure wireless bridge to the host's serial console. It enables network administrators to perform emergency recovery, initial configuration, and live terminal debugging without requiring wired console cables or physical access.

---

## Key Features

- **⚡ Full ANSI/VT100 Web Terminal**:
  - High-speed bidirectional console streaming over WebSockets.
  - Complete 256-color palette, cursor movement, and screen clearing support.
  - Mobile touch keypad for critical non-printable keys (`Esc`, `Tab`, `Ctrl+C`, `Ctrl+Z`, `Ctrl+D`, `↑`, `↓`, `←`, `→`, `Enter`, `Clear`).
  - Customizable quick macro buttons for rapid diagnostic commands.

- **🔌 Non-Blocking Telnet Server (Port 23)**:
  - RFC 854 compliant Telnet daemon for native command-line access via PuTTY, OpenSSH (`telnet`), or Minicom.
  - Optional password challenge prompt before granting serial pass-through.

- **📶 Dual Wi-Fi & Captive Portal**:
  - **Access Point (AP) Mode**: Creates an isolated `ESP-OOBM-XXXXXX` network with DNS Captive Portal interception (instant pop-up configuration on Android, iOS, Windows, macOS).
  - **Station (Client) Mode**: Connects to existing infrastructure Wi-Fi with automatic background AP scanning and DHCP/Static IP support.

- **🔒 Enterprise-Grade Security**:
  - Password-protected Web Management with HTTP Basic Authentication.
  - Protected REST/AJAX API endpoints (`401 Unauthorized` enforcement).
  - Authenticated WebSocket terminal handshake and Telnet access control.

- **⏱️ SNTP Time Synchronization & Worldwide POSIX Timezones**:
  - Built-in SNTP client synchronized to network time servers.
  - Full POSIX timezone database with automatic daylight saving time (DST) adjustments.

- **📊 Prometheus `/metrics` Exporter**:
  - Dedicated `/metrics` endpoint exporting real-time CPU, RAM, heap fragmentation %, WiFi RSSI, UART RX/TX byte counters, and session metrics.

- **🔄 Over-The-Air (OTA) Firmware Updates**:
  - Browser-based multipart `.bin` upload page with real-time progress bar, MD5 integrity check, dual-partition fail-safe swap, and auto-restart.
  - PlatformIO ArduinoOTA network flashing support (Port 3232).

- **🧠 Zero-Heap Fragmentation Architecture**:
  - 100% of HTML, CSS, JavaScript, and SVG vector graphics are stored in Flash ROM (`PROGMEM`).
  - Pre-allocated static ring buffers for UART RX/TX and system event logs.
  - Chunked streaming HTTP delivery to prevent dynamic memory allocation churn.

---

## Compatibility & Verification

The firmware is completely vendor-neutral and communicates over standard serial data protocols. It has been tested and verified on:
- **MikroTik RouterOS v7.x** (e.g. **RB5009UG+S+**, CCR series) via USB host port (`/port/print` -> `usb1`).
- **Linux Network Appliances & Servers** via USB CDC/CH343 serial ports (`/dev/ttyCH343USB0` or `/dev/ttyUSB0`).
- **Cisco / Juniper / Generic Network Switches** with USB console or RS-232 adapter interfaces.

---

## Project Structure

```
WiFi-Out-of-Band-Management/
├── README.md                      # Main project documentation (English)
├── LICENSE                        # MIT open-source license
│
├── hardware/                      # Hardware documentation & specs
│   ├── README.md                  # Hardware overview & technical specifications
│   ├── PINOUT.md                  # Detailed pinout & schematic wiring notes
│   └── images/                    # Hardware photos & component diagrams
│
└── software/                      # PlatformIO firmware source code
    ├── platformio.ini             # PlatformIO build configuration
    ├── partitions_custom.csv      # 4MB dual OTA partition scheme (2x 1.92MB)
    ├── include/                   # Header files
    │   ├── Config.h               # Pin definitions, defaults, NVS keys, version
    │   ├── Timezones.h            # POSIX timezone definitions
    │   ├── VectorGraphics.h       # Custom SVG vector logo & favicon (PROGMEM)
    │   ├── SystemStats.h          # System telemetry & fragmentation tracker
    │   ├── SerialBridge.h         # Static ring buffer & UART management
    │   ├── WebTerminal.h          # WebSocket terminal session engine
    │   ├── TelnetServer.h         # RFC 854 Telnet server & auth
    │   ├── PrometheusExporter.h   # Prometheus /metrics endpoint exporter
    │   ├── MndpDiscovery.h        # Neighbor Discovery Protocol broadcaster
    │   ├── WebPortal.h            # Web server routes, HTML templates, OTA
    │   └── ConsoleLogger.h        # Circular in-memory event logger
    └── src/                       # Source implementation files
        ├── main.cpp               # System orchestration & main loop
        ├── SystemStats.cpp        # Telemetry metrics collection
        ├── SerialBridge.cpp       # UART FIFO buffer streaming
        ├── WebTerminal.cpp        # WebSocket streaming implementation
        ├── TelnetServer.cpp       # Telnet socket server implementation
        ├── PrometheusExporter.cpp # Prometheus metric formatting
        ├── MndpDiscovery.cpp      # UDP 5678 discovery broadcast implementation
        ├── WebPortal.cpp          # Web GUI, REST handlers & OTA upload
        └── ConsoleLogger.cpp      # Event logger implementation
```

---

## Web User Interface

The web interface features a clean, responsive design with light and dark mode support:

1. **Dashboard** (`/`): Real-time telemetry cards displaying system performance, memory heap fragmentation, serial byte counters, active sessions, Wi-Fi RSSI signal quality, and NTP time.
2. **Terminal** (`/terminal`): Full-screen interactive ANSI console with touch toolbar for mobile devices.
3. **Settings** (`/settings`): Consolidated configuration page with top quick-jump anchors:
   - `[Serial Settings]`: Baud rate (300 to 921,600 bps; default 115,200), Parity, Stop bits, Local Echo, Telnet port.
   - `[WiFi & Network]`: Auto-scanned AP list with live signal bars, Station mode credentials, AP settings, Captive portal.
   - `[Security]`: Web Admin password protection, Telnet password challenge, Hostname / mDNS identity.
   - `[System & Maintenance]`: NTP Server, POSIX Timezone selector, Device Reboot, Factory Reset.
   - `[OTA Firmware Update]`: Drag-and-drop `.bin` file upload with live progress bar.
   - `[System Event Logs]`: Rolling in-memory log buffer for connection and system events.

---

## Getting Started & Flashing

### 1. Requirements
- [PlatformIO Core](https://platformio.org/) or [VS Code with PlatformIO IDE Extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide).
- USB cable / direct USB-A port connection to the ESP32 KEY V1.0 dongle.

### 2. Compilation & Flashing
Navigate to the `software/` directory and compile the firmware:

```powershell
cd software
pio run -e esp32_pico_d4
```

Upload the firmware to the ESP32 dongle over USB:

```powershell
pio run -e esp32_pico_d4 -t upload
```

### 3. Initial Wi-Fi Connection
1. Insert the dongle into a USB port.
2. Connect to the Wi-Fi Access Point: `ESP-OOBM-XXXXXX` (Open by default).
3. The Captive Portal will automatically open `http://192.168.4.1/`.
4. Open the **Terminal** tab to begin console access, or configure your local Wi-Fi under **Settings**.

---

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.
