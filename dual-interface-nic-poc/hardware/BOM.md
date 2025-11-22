# Bill of Materials (BOM)

## Phase 1: USB-C Only Prototype

### Core Components

| Item | Description | Quantity | Unit Price | Total | Source |
|------|-------------|----------|------------|-------|--------|
| ESP32-S3 DevKit | ESP32-S3-WROOM-1 with USB-C | 1 | $8.00 | $8.00 | AliExpress, Amazon |
| W5500 Module | Ethernet controller with RJ45 | 1 | $4.00 | $4.00 | AliExpress, Amazon |
| Jumper Wires | Female-to-female, 20cm | 10 | $0.10 | $1.00 | AliExpress, Amazon |
| **Subtotal** | | | | **$13.00** | |

### Optional Components

| Item | Description | Quantity | Unit Price | Total |
|------|-------------|----------|------------|-------|
| LEDs | 3mm or 5mm, assorted colors | 3 | $0.10 | $0.30 |
| Resistors | 330Ω, 1/4W | 3 | $0.05 | $0.15 |
| Breadboard | 400 tie-points | 1 | $2.00 | $2.00 |
| USB-C Cable | For programming | 1 | $3.00 | $3.00 |
| Ethernet Cable | Cat5e or Cat6, 1m | 1 | $2.00 | $2.00 |
| **Optional Total** | | | | **$7.45** |

## Phase 2: Full Dual Interface (USB-C + PCIe)

### Additional Components

| Item | Description | Quantity | Unit Price | Total | Source |
|------|-------------|----------|------------|-------|--------|
| PCIe-USB Adapter | ASM1042A based, x1 slot | 1 | $15.00 | $15.00 | AliExpress |
| USB-C Breakout | With PD support | 1 | $5.00 | $5.00 | AliExpress, Adafruit |
| Level Shifter | 74LVC245 or similar | 1 | $1.00 | $1.00 | AliExpress, Digikey |
| Resistors | 10kΩ, 1/4W (pull-ups) | 5 | $0.05 | $0.25 | AliExpress, Digikey |
| Capacitors | 100nF ceramic, 0805 | 10 | $0.10 | $1.00 | AliExpress, Digikey |
| Wire | 30AWG, various colors | 1m | $5.00 | $5.00 | Amazon |
| **Phase 2 Subtotal** | | | | **$27.25** | |

### Power Components

| Item | Description | Quantity | Unit Price | Total |
|------|-------------|----------|------------|-------|
| LDO Regulator | AMS1117-3.3 | 1 | $0.50 | $0.50 |
| Capacitor | 10µF electrolytic | 2 | $0.15 | $0.30 |
| Capacitor | 100nF ceramic | 4 | $0.10 | $0.40 |
| Schottky Diode | 1N5819 or similar | 2 | $0.20 | $0.40 |
| **Power Total** | | | | **$1.60** |

## Phase 3: Custom PCB (Optional)

### PCB Manufacturing

| Item | Description | Quantity | Unit Price | Total | Source |
|------|-------------|----------|------------|-------|--------|
| PCB | 2-layer, 60x40mm | 5 | $2.00 | $10.00 | JLCPCB, PCBWay |
| Assembly | SMT assembly service | 1 | $15.00 | $15.00 | JLCPCB |
| Stencil | For solder paste | 1 | $8.00 | $8.00 | JLCPCB |
| **PCB Total** | | | | **$33.00** | |

### Enclosure

| Item | Description | Quantity | Unit Price | Total |
|------|-------------|----------|------------|-------|
| 3D Print | PLA filament, ~50g | 1 | $5.00 | $5.00 |
| Screws | M3 x 6mm | 4 | $0.10 | $0.40 |
| Standoffs | M3 x 10mm | 4 | $0.25 | $1.00 |
| **Enclosure Total** | | | | **$6.40** |

## Total Cost Summary

| Phase | Description | Cost |
|-------|-------------|------|
| Phase 1 | USB-C Only (Minimal) | $13.00 |
| Phase 1 | USB-C Only (With Optional) | $20.45 |
| Phase 2 | Full Dual Interface | $47.70 |
| Phase 3 | Custom PCB + Enclosure | $87.10 |

## Recommended Suppliers

### Budget Options (Longer Shipping)
- **AliExpress**: Cheapest prices, 2-4 week shipping
- **Banggood**: Similar to AliExpress
- **LCSC**: Good for bulk components

### Fast Shipping Options
- **Amazon**: 1-2 day shipping, higher prices
- **Adafruit**: Quality components, US-based
- **SparkFun**: Educational focus, good documentation
- **Digikey**: Professional supplier, huge selection
- **Mouser**: Similar to Digikey

### PCB Manufacturing
- **JLCPCB**: $2 for 5 PCBs, fast turnaround
- **PCBWay**: Slightly more expensive, good quality
- **OSH Park**: US-based, purple PCBs
- **Seeed Studio**: Good for prototypes

## Alternative Component Options

### ESP32 Alternatives

| Component | Price | Pros | Cons |
|-----------|-------|------|------|
| ESP32-S3 DevKit | $8 | USB-C, native USB OTG | Larger size |
| ESP32-C3 SuperMini | $3 | Tiny, USB-C | Less powerful |
| ESP32-S2 | $4 | USB device mode | No Bluetooth |
| Raspberry Pi Pico W | $6 | Better USB stack | No Ethernet MAC |

### Ethernet Controller Alternatives

| Component | Price | Pros | Cons |
|-----------|-------|------|------|
| W5500 | $4 | Hardware TCP/IP, proven | 100Mbps only |
| W6100 | $6 | IPv6 support, newer | Less common |
| ENC28J60 | $2 | Very cheap | Software stack, slower |
| LAN8720 | $3 | Gigabit capable | Needs external MAC |

## Tools Required

### Basic Tools (Phase 1)
- Wire strippers
- Multimeter
- Computer with USB port

### Advanced Tools (Phase 2+)
- Soldering iron (temperature controlled)
- Solder (lead-free recommended)
- Flux
- Helping hands/PCB holder
- Hot air station (for SMD work)
- Oscilloscope (for debugging)

### Estimated Tool Cost
- Basic: $20-50
- Advanced: $100-300
- Professional: $500+

## Notes

1. Prices are approximate and vary by supplier/location
2. Bulk discounts available for quantities >10
3. Shipping costs not included
4. Some components may require minimum order quantities
5. Consider buying extras for mistakes/testing
