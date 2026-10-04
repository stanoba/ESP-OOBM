# ESP32 KEY V1.0 - Hardware Specifications

The **ESP32 KEY V1.0** is an ultra-compact USB-A dongle development board based on the **Espressif ESP32-PICO-D4** System-in-Package (SiP) and the **WCH CH343P** high-speed USB-to-UART bridge controller.

---

## Technical Specifications

| Parameter | Specification |
| :--- | :--- |
| **Microcontroller** | Espressif ESP32-PICO-D4 (Dual-core Xtensa® 32-bit LX6 up to 240 MHz) |
| **Embedded Flash** | 4 MB SPI Flash (integrated directly inside the PICO-D4 SiP) |
| **SRAM** | 520 KB internal SRAM + 448 KB ROM |
| **USB-to-UART Bridge** | WCH CH343P (Full-speed USB 2.0, hardware baud rates up to 6 Mbps) |
| **Host Interface** | Standard USB-A Male (VBUS 5V, D-, D+, GND) |
| **Wireless Interface** | 2.4 GHz Wi-Fi (802.11 b/g/n) + Bluetooth v4.2 BR/EDR and BLE |
| **Antenna** | Embedded 2.4 GHz High-Efficiency Ceramic Chip Antenna |
| **Operating Voltage** | 5.0 V DC (supplied via USB host port, internal 3.3V LDO regulator) |
| **Controls** | 1x Tactile Function Pushbutton (connected to `GPIO0`) |
| **Indicators** | On-board SMD LEDs (Power, Serial RX/TX, WiFi Status) |
| **Enclosure** | Transparent snap-fit protective ABS/polycarbonate casing with lanyard hole |

---

## Block Diagram & Operating Principle

```
+-------------------------------------------------------------------------+
| ESP32 KEY V1.0 USB Dongle                                               |
|                                                                         |
|  +-------------+       +-------------------+       +-----------------+  |
|  | USB-A Male  | <===> | WCH CH343P        | <===> | ESP32-PICO-D4   |  |
|  | (5V, D-, D+) |       | USB-to-UART       |       | (Dual-Core LX6) |  |
|  +-------------+       | (Virtual COM)     |       | (4MB Flash)     |  |
|                        +-------------------+       +-----------------+  |
|                                                            ^            |
|                                                            |            |
|                                                   +-----------------+   |
|                                                   | 2.4GHz Ceramic  |   |
|                                                   | Antenna         |   |
|                                                   +-----------------+   |
+-------------------------------------------------------------------------+
```

When plugged into a USB host port on a router, switch, or server:
1. **Power Supply**: 5V VBUS from the host port powers the internal 3.3V low-dropout regulator (LDO) on the dongle.
2. **USB Enumeration**: The host OS (Linux, RouterOS, Windows, macOS) detects the CH343P bridge as a standard serial device (e.g. `/dev/ttyACM0`, `/dev/ttyUSB0`, or `/port usb1`).
3. **Serial Communication**: Hardware `UART0` (`GPIO1 TXD`, `GPIO3 RXD`) on the ESP32 communicates bidirectionally with the host serial console.
4. **Wireless Bridge**: The ESP32 creates an independent Wi-Fi access point / connects to a management network to expose the serial console via WebSockets (Web Terminal) and Telnet (Port 23).

---

## Function Button & LED Indicators

- **Function Button (`GPIO0`)**:
  - **Short Press (< 2 seconds)**: Broadcasts a network neighbor discovery announcement.
  - **Long Press (> 5 seconds)**: Initiates a Factory Reset, clearing stored NVS configuration and rebooting into default AP setup mode.
  - **Boot Mode**: Holding the button while inserting into USB puts the ESP32 into ROM bootloader download mode for wired firmware flashing.

- **LED Indicators**:
  - **Blue / Green LED**: WiFi connection and AP status (blinking = AP active, solid = Station connected).
  - **Amber / Red LED**: Serial traffic activity (flashes on RX/TX packet transit).
