# ESP-OOBM - Firmware & Software Project

This directory contains the PlatformIO firmware project and source code for the **ESP-OOBM** wireless Out-of-Band console bridge.

> [!TIP]
> For the complete software architecture, FreeRTOS tasks, module lifecycles, memory partition maps, and NVS configuration keys, see the full guide:
> 
> 📖 **[Software & Firmware Architecture Guide](../docs/software.md)**

---

## Directory Contents & Modules

| Module / Path | Description |
| :--- | :--- |
| [`src/main.cpp`](src/main.cpp) | Main application entry point, Wi-Fi initialization, and daemon event loop |
| [`include/Config.h`](include/Config.h) | Hardware pinouts, default credentials, network ports, and NVS key mappings |
| [`include/`](include/) & [`src/`](src/) | C++ core modules (`SerialBridge`, `WebTerminal`, `TelnetServer`, `WebPortal`, `PrometheusExporter`, `MndpDiscovery`, `ConsoleLogger`, `SystemStats`) |
| [`platformio.ini`](platformio.ini) | PlatformIO build configuration for ESP32-PICO-D4 |
| [`partitions_custom.csv`](partitions_custom.csv) | 4 MB Flash dual-OTA partition table definition |

---

## Quick Build & Upload

```powershell
# Build firmware binary
pio run -e esp32_pico_d4

# Flash to dongle via USB
pio run -e esp32_pico_d4 -t upload
```

### Factory Reset (Clear All Settings)
```powershell
# Erase all flash memory and reset to defaults
pio run -e esp32_pico_d4 -t erase
pio run -e esp32_pico_d4 -t upload
```

---

## Related Documentation

* 📖 **[Software & Firmware Architecture Guide](../docs/software.md)**
* 🌐 **[REST & WebSocket API Specification](../docs/api.md)**
* 📊 **[Prometheus Exporter & Grafana Guide](../docs/prometheus.md)**
* 💻 **[Web Interface & Terminal Guide](../docs/ui-guide.md)**
* 🔌 **[Hardware Specifications & Pinout Guide](../docs/hardware.md)**
