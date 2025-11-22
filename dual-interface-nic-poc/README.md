# Dual Interface NIC - Proof of Concept

A cross-platform (x86 + ARM) network interface card using ESP32 with both USB-C and PCIe support.

## Hardware Requirements

### Minimal POC (USB-C Only - $12)
- ESP32-S3 DevKit with USB-C ($8)
- W5500 Ethernet Module ($4)
- Jumper wires

### Full POC (USB-C + PCIe - $35)
- ESP32-S3 DevKit with USB-C ($8)
- W5500 Ethernet Module ($4)
- PCIe-to-USB Adapter Board ($15)
- USB-C breakout board ($5)
- Jumper wires ($3)

## Architecture

```
Host Computer (x86/ARM)
    ↓
USB-C or PCIe Interface
    ↓
ESP32-S3 (Bridge Controller)
    ↓
W5500 Ethernet Controller
    ↓
Network
```

## Pin Connections

### ESP32-S3 to W5500 (SPI)
```
ESP32-S3    W5500
GPIO36   →  SCK
GPIO37   →  MISO
GPIO35   →  MOSI
GPIO34   →  CS
GPIO33   →  RST
3.3V     →  VCC
GND      →  GND
```

### Interface Detection
```
GPIO2  →  PCIe Present Detection (pulled low when PCIe active)
GPIO15 →  USB-C CC1 Detection
GPIO12 →  Status LED (USB Active)
GPIO13 →  Status LED (PCIe Active)
```

## Features

- ✅ Cross-platform: Works on x86 and ARM hosts
- ✅ Dual interface: USB-C and PCIe support
- ✅ Auto-detection: Automatically selects active interface
- ✅ Standard drivers: Uses CDC-ECM (no custom drivers needed)
- ✅ Hardware Ethernet: W5500 handles TCP/IP stack
- ✅ Hot-plug support: USB-C hot-pluggable

## Supported Host Systems

- Linux (x86_64, ARM64, RISC-V)
- Windows 10/11 (x86_64, ARM64)
- macOS (Intel, Apple Silicon)
- Embedded Linux (Raspberry Pi, etc.)

## Build Instructions

See `docs/BUILD.md` for detailed assembly instructions.

## Firmware

See `firmware/` directory for Arduino/ESP-IDF code.

## Testing

See `docs/TESTING.md` for cross-platform testing procedures.
