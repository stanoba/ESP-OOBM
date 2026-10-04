# ESP32 KEY V1.0 - Hardware Pinout & Wiring

This document details the internal electrical connections between the **ESP32-PICO-D4** System-in-Package, the **WCH CH343P** USB bridge, the tactile button, and status LEDs on the **ESP32 KEY V1.0** dongle.

---

## Interconnect Wiring Table

| ESP32-PICO-D4 Pin | Function | Peripheral / Connection | Description |
| :--- | :--- | :--- | :--- |
| **GPIO1** | `U0TXD` | CH343P `RXD` | Hardware Serial TX (Data sent from ESP32 to Host USB) |
| **GPIO3** | `U0RXD` | CH343P `TXD` | Hardware Serial RX (Data received by ESP32 from Host USB) |
| **GPIO0** | `BOOT` / `IO0` | Tactile Button (SW1) | Function Button (Active-LOW, pull-up to 3.3V) |
| **GPIO10** | `IO10` | Status Blue LED (D3) | Wi-Fi / Status LED (Active-LOW: 3.3V &rarr; R3 10k&Omega; &rarr; D3 Blue LED &rarr; GPIO10) |
| **EN** (`CHIP_PU`) | Reset | RC Circuit + CH343P `DTR/RTS` | Auto-reset circuit for USB flashing |
| **VDD33** | Power | 3.3V LDO Output | Core supply voltage |
| **GND** | Ground | Common Ground Plane | Common reference ground |

---

## USB Interface Pinout (Standard USB-A Male)

| USB Pin | Name | Wire / Signal | Connection |
| :--- | :--- | :--- | :--- |
| **Pin 1** | `VBUS` | +5.0 V DC | Connected to 3.3V LDO input |
| **Pin 2** | `D-` | USB Data Negative | Connected to CH343P `UD-` pin |
| **Pin 3** | `D+` | USB Data Positive | Connected to CH343P `UD+` pin |
| **Pin 4** | `GND` | Ground | Connected to common board GND |
| **Shell** | `SHIELD` | Chassis Ground | Connected to USB plug shield |

---

## ESP32-PICO-D4 Internal Architecture

The ESP32-PICO-D4 integrates all required peripheral passives inside a compact 7×7 mm QFN package:
- 40 MHz Crystal Oscillator
- 4 MB SPI Flash Memory connected internally to `GPIO6`, `GPIO7`, `GPIO8`, `GPIO9`, `GPIO10`, `GPIO11` (Do not use these GPIOs externally).
- Decoupling capacitors and RF matching network directly connected to the ceramic antenna pad.

---

## Host Operating System Driver Information

The WCH CH343P is an updated USB-to-UART bridge controller that is natively supported across modern operating systems:
- **RouterOS v7.x**: Automatically detected upon insertion (`/port print` displays `usb1` or `serial0`).
- **Linux (Kernel >= 5.12)**: Native `ch341`/`ch343` driver in kernel tree creates `/dev/ttyCH343USB0` or `/dev/ttyUSB0`.
- **macOS (>= 11 Big Sur)**: Supported via native Apple USB CDC-ACM driver or official WCH macOS driver.
- **Windows (10 / 11)**: Supported via Windows Update or WCH CH343 CDC driver (`COMx`).
