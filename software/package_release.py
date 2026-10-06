#!/usr/bin/env python3
"""
ESP-OOBM Firmware Packaging Script
Builds and packages firmware binaries for releases:
1. esp-oobm-v<VERSION>-firmware.bin (OTA update)
2. esp-oobm-v<VERSION>-factory-0x0.bin (Complete factory flash from 0x0000)
3. esp-oobm-v<VERSION>-checksums.txt (SHA-256 hashes)
"""

import sys
import os
import glob
import shutil
import hashlib
import subprocess

def get_firmware_version():
    config_path = os.path.join(os.path.dirname(__file__), "include", "Config.h")
    if os.path.exists(config_path):
        with open(config_path, "r", encoding="utf-8") as f:
            for line in f:
                if "#define FIRMWARE_VERSION" in line:
                    parts = line.split()
                    if len(parts) >= 3:
                        return parts[2].strip('"')
    return "1.0.1"

def find_boot_app0():
    # Look in common PlatformIO locations
    home_dir = os.path.expanduser("~")
    candidates = glob.glob(os.path.join(home_dir, ".platformio", "packages", "**", "boot_app0.bin"), recursive=True)
    if candidates:
        return candidates[0]
    return None

def sha256_file(filepath):
    h = hashlib.sha256()
    with open(filepath, "rb") as f:
        while chunk := f.read(65536):
            h.update(chunk)
    return h.hexdigest()

def main():
    version = sys.argv[1] if len(sys.argv) > 1 else get_firmware_version()
    if version.startswith("v"):
        version = version[1:]

    script_dir = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.join(script_dir, ".pio", "build", "esp32_pico_d4")
    dist_dir = os.path.join(script_dir, "dist")

    os.makedirs(dist_dir, exist_ok=True)

    bootloader = os.path.join(build_dir, "bootloader.bin")
    partitions = os.path.join(build_dir, "partitions.bin")
    firmware = os.path.join(build_dir, "firmware.bin")
    boot_app0 = find_boot_app0()

    if not os.path.exists(firmware):
        print(f"Error: {firmware} not found. Run 'pio run -e esp32_pico_d4' first.")
        sys.exit(1)

    # 1. Output file paths
    ota_dest = os.path.join(dist_dir, f"esp-oobm-v{version}-firmware.bin")
    factory_dest = os.path.join(dist_dir, f"esp-oobm-v{version}-factory-0x0.bin")
    checksums_dest = os.path.join(dist_dir, f"esp-oobm-v{version}-checksums.txt")

    # 2. Copy OTA firmware binary
    shutil.copyfile(firmware, ota_dest)
    print(f"Created OTA binary: {os.path.basename(ota_dest)}")

    # 3. Merge factory binary
    if os.path.exists(bootloader) and os.path.exists(partitions) and boot_app0 and os.path.exists(boot_app0):
        merge_cmd = [
            sys.executable, "-m", "esptool",
            "--chip", "esp32",
            "merge-bin",
            "-o", factory_dest,
            "--flash-mode", "dio",
            "--flash-freq", "40m",
            "--flash-size", "4MB",
            "0x1000", bootloader,
            "0x8000", partitions,
            "0xe000", boot_app0,
            "0x10000", firmware
        ]
        print("Merging factory binary with esptool...")
        res = subprocess.run(merge_cmd, capture_output=True, text=True)
        if res.returncode == 0:
            print(f"Created Factory binary: {os.path.basename(factory_dest)}")
        else:
            print(f"Warning: merge-bin failed: {res.stderr}")
    else:
        print("Warning: Missing bootloader, partitions, or boot_app0.bin. Skipping factory merge.")

    # 4. Generate SHA256 Checksums
    checksums = []
    for f in [ota_dest, factory_dest]:
        if os.path.exists(f):
            fname = os.path.basename(f)
            chash = sha256_file(f)
            checksums.append(f"{chash}  {fname}")

    with open(checksums_dest, "w", encoding="utf-8") as f:
        f.write("\n".join(checksums) + "\n")

    print(f"Created SHA256 checksums: {os.path.basename(checksums_dest)}")
    print(f"\nAll release artifacts ready in: {dist_dir}")

if __name__ == "__main__":
    main()
