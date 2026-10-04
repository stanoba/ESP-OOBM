# ESP32 KEY V1.0 - Hardware Specifications

Vector Schematic: [`schematic.svg`](schematic.svg) • High-Res Image: [`schematic.png`](schematic.png) • Pinout: [`PINOUT.md`](PINOUT.md)

## Hardware Photos

| Enclosure View | PCB Top (Components) | PCB Bottom (Silkscreen) |
| :---: | :---: | :---: |
| ![Dongle in Enclosure](ESP32_dongle_01.jpg) | ![Bare PCB Top](ESP32_dongle_02.jpg) | ![Bare PCB Bottom](ESP32_dongle_03.jpg) |

---

## Schematic Diagram

[![Schematic Diagram](schematic.png)](schematic.svg)

---

## Component Summary

| Component | Part / IC | Details |
| :--- | :--- | :--- |
| **SoC / SiP** | Espressif **ESP32-PICO-D4** | Dual-core 240 MHz Xtensa LX6, 4 MB embedded SPI flash, QFN-48 (7×7 mm) |
| **USB-UART Bridge** | WCH **CH343P** | High-speed USB-to-UART bridge controller (up to 6 Mbps baud rate) |
| **Power LDO** | MicrOne **ME6211C33M5G** | 5V &rarr; 3.3V / 500 mA Low Dropout Regulator, SOT-23-5 |
| **Auto-Reset** | 2x **S8050** NPN | `Q1`, `Q2` driven by `DTR` & `RTS` lines (hands-free `esptool` flashing) |
| **Status LED** | Blue 0603 SMD | `D3` connected to `GPIO10` (Active-LOW with 10k&Omega; pull-up to 3.3V) |
| **Host Plug** | USB Type-A Male | Direct insertion into router/switch/server USB host port |
| **Antenna** | 2.4 GHz SMD Ceramic Chip | 2.4 GHz 802.11 b/g/n Wi-Fi + Bluetooth |

---

## Operating Principle

```mermaid
flowchart LR
    subgraph Host["Host Device (Router / Switch / Server)"]
        direction TB
        OS["Target OS Console<br/>(MikroTik / Linux / FreeBSD)"]
        USB["USB Host Port<br/>(5V DC + USB CDC)"]
        OS <-->|"TTY / Serial Console"| USB
    end

    subgraph Dongle["ESP32 KEY V1.0 (ESP-OOBM Dongle)"]
        direction TB
        CH["WCH CH343P<br/>USB-to-UART Bridge"]
        
        subgraph MCU["ESP32-PICO-D4 SoC"]
            UART["UART0 Driver<br/>(GPIO1 TX / GPIO3 RX)"]
            SRV["OOB Firmware Engine<br/>• WebTerminal (WS :81)<br/>• Telnet Daemon (Port 23)<br/>• Web Management UI (Port 80)"]
            UART <--> SRV
        end
        
        CH <-->|"TTL UART (115200 baud)"| UART
    end

    subgraph Client["Administrator Device"]
        direction TB
        UI["Web Browser / PuTTY / Telnet<br/>(Emergency Wireless Access)"]
    end

    USB <==>|"USB-A Connector"| CH
    SRV <==>|"Wi-Fi (AP / Station Mode)<br/>802.11 b/g/n"| UI
```

