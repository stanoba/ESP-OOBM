# ESP32 KEY V1.0 - Hardware Resources

This directory contains hardware design assets, pinout mappings, and schematic generation tools for the **ESP32 KEY V1.0** USB dongle with **ESP32-PICO-D4** SoC and **WCH CH343P** USB-to-UART bridge.

> [!TIP]
> For the full hardware specification, component BOM, GPIO truth tables, and auto-download circuit operation, see the complete guide:
> 
> 📖 **[Hardware Specifications & Pinout Guide](../docs/hardware.md)**

---

## Directory Contents

| File / Resource | Description |
| :--- | :--- |
| [`generate_schematic.py`](generate_schematic.py) | Python script to generate vector SVG and raster PNG schematics |
| [`PINOUT.md`](PINOUT.md) | Quick GPIO and CH343P pinout reference sheet |
| [`schematic.svg`](../assets/schematic.svg) | Vector schematic diagram |
| [`schematic.png`](../assets/schematic.png) | High-resolution schematic rendering |

---

## Hardware Photos

| Enclosure View | PCB Top (Components) | PCB Bottom (Silkscreen) |
| :---: | :---: | :---: |
| ![Dongle in Enclosure](../assets/ESP32_dongle_01.jpg) | ![Bare PCB Top](../assets/ESP32_dongle_02.jpg) | ![Bare PCB Bottom](../assets/ESP32_dongle_03.jpg) |
