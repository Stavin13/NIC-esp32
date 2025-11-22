# EasyEDA Schematic Guide

## Quick Start Checklist

### Components to Search in EasyEDA Library

1. **ESP32-S3-WROOM-1** or **ESP32-WROOM-32**
2. **W5500** (search: "W5500-QFN48")
3. **HR911105A** (RJ45 with magnetics)
4. **USB-C-16P** or **TYPE-C-31-M-12**
5. **AMS1117-3.3** (voltage regulator)
6. **Capacitors**: C0805 (10uF, 1uF, 100nF)
7. **Resistors**: R0805 (10K, 330R)
8. **LEDs**: LED-0805
9. **Crystal**: 25MHz (for W5500)

## Schematic Sheets Breakdown

### Sheet 1: Power Management

**Components:**
- J1: USB-C connector (TYPE-C-16P)
- J2: PCIe edge connector (search: "PCIe-x1-edge")
- U1: AMS1117-3.3 voltage regulator
- C1, C2: 10uF capacitors (input/output filtering)
- C3, C4: 100nF bypass capacitors
- D1, D2: Schottky diodes (power selection)

**Connections:**
```
USB-C VBUS (pins 4,9) → D1 → VIN
PCIe 3.3V (pin A3) → D2 → VIN
VIN → U1 input
U1 output → VCC_3V3 (power rail)
All GND pins → GND (ground plane)
```

**Net Labels to Add:**
- VCC_3V3 (main 3.3V rail)
- USB_5V (USB power input)
- PCIe_3V3 (PCIe power input)
- GND (ground)

### Sheet 2: ESP32 Core

**Components:**
- U2: ESP32-S3-WROOM-1 or ESP32-WROOM-32
- SW1: Reset button (tactile switch)
- R1, R2: 10kΩ pull-up resistors
- C5, C6: 100nF, 10uF decoupling capacitors

**Connections:**
```
Power:
VCC_3V3 → ESP32 pin 2 (VDD)
GND → ESP32 pins 1,15,38 (GND)

Reset Circuit:
ESP32 EN → R1 → VCC_3V3
ESP32 EN → SW1 → GND

Boot Mode:
ESP32 GPIO0 → R2 → VCC_3V3
```

**GPIO Assignments (add net labels):**
```
GPIO36 → SPI_SCK
GPIO37 → SPI_MISO
GPIO35 → SPI_MOSI
GPIO34 → W5500_CS
GPIO33 → W5500_RST
GPIO2 → PCIE_PRESENT
GPIO15 → USB_CC_DETECT
GPIO12 → LED_USB
GPIO13 → LED_PCIE
GPIO14 → LED_NET
```

### Sheet 3: Ethernet Interface

**Components:**
- U3: W5500 (QFN48 package)
- J3: HR911105A (RJ45 with magnetics)
- Y1: 25MHz crystal
- C7, C8: 22pF load capacitors
- C9-C12: 100nF bypass capacitors (one per power pin)

**W5500 Power Connections:**
```
VCC_3V3 → W5500 pins:
  - AVDD (pin 1)
  - DVDD (pin 9)
  - PVDD (pin 17)
  
GND → W5500 pins:
  - AGND (pin 8)
  - DGND (pin 16)
  - PGND (pin 24)

Add 100nF cap near each power pin
```

**W5500 SPI Connections:**
```
SPI_SCK → W5500 SCLK (pin 30)
SPI_MISO → W5500 MISO (pin 31)
SPI_MOSI → W5500 MOSI (pin 32)
W5500_CS → W5500 SCSn (pin 33)
W5500_RST → W5500 RSTn (pin 34)
```

**Crystal Connections:**
```
Y1 pin 1 → W5500 XI (pin 11)
Y1 pin 2 → W5500 XO (pin 12)
C7 (22pF) from XI to GND
C8 (22pF) from XO to GND
```

**Ethernet PHY Connections:**
```
W5500 → HR911105A:
TX+ (pin 21) → RJ45 pin 1
TX- (pin 22) → RJ45 pin 2
RX+ (pin 24) → RJ45 pin 3
RX- (pin 25) → RJ45 pin 6

Note: HR911105A has integrated magnetics,
so direct connection is OK
```

### Sheet 4: Status LEDs

**Components:**
- LED1, LED2, LED3: 0805 LEDs
- R3, R4, R5: 330Ω current limiting resistors

**Connections:**
```
LED_USB → R3 → LED1 anode
LED1 cathode → GND

LED_PCIE → R4 → LED2 anode
LED2 cathode → GND

LED_NET → R5 → LED3 anode
LED3 cathode → GND
```

## PCB Layout Tips

### Layer Stack (2-layer board)
```
Top Layer:
├── Signal traces
├── Components (mostly top side)
└── Ground pour (where possible)

Bottom Layer:
├── Ground plane (solid)
├── Power traces (VCC_3V3)
└── Some signal routing if needed
```

### Component Placement Strategy

```
Board Layout (60mm x 40mm):

┌─────────────────────────────────────┐
│  [USB-C]                    [LEDs]  │
│                                     │
│  [Power]  [ESP32-S3]  [W5500]      │
│   (U1)      (U2)        (U3)       │
│                                     │
│                        [RJ45]       │
│                         (J3)        │
│                                     │
│  [PCIe Edge Connector]              │
└─────────────────────────────────────┘
```

### Critical Routing Rules

1. **SPI Traces (ESP32 ↔ W5500)**
   - Keep traces short (< 50mm)
   - Route on same layer if possible
   - Match lengths within 5mm
   - Use 0.2mm trace width minimum

2. **Power Traces**
   - VCC_3V3: 0.5mm width minimum
   - Add multiple vias to ground plane
   - Place decoupling caps close to ICs

3. **USB-C Differential Pairs**
   - D+ and D- must be matched length
   - 90Ω differential impedance
   - Keep away from other signals

4. **Ethernet Differential Pairs**
   - TX+/TX- and RX+/RX- matched
   - 100Ω differential impedance
   - Route away from digital signals

5. **Crystal Placement**
   - Place 25MHz crystal close to W5500
   - Keep traces short
   - Ground plane under crystal
   - No traces under crystal

### Design Rule Check Settings

```
Minimum Trace Width: 0.15mm (6mil)
Minimum Trace Spacing: 0.15mm (6mil)
Minimum Via Size: 0.3mm drill, 0.6mm pad
Minimum Hole Size: 0.3mm

For JLCPCB:
- Use their DRC rules (built into EasyEDA)
- 2-layer board
- 1.6mm thickness
- HASL surface finish
```

## Export for Manufacturing

### Gerber Files
1. Click "Fabrication Output" → "Gerber"
2. Select all layers
3. Export as ZIP file
4. Upload directly to JLCPCB

### BOM (Bill of Materials)
1. Click "Fabrication Output" → "BOM"
2. Export as CSV
3. Add LCSC part numbers for assembly

### Pick and Place
1. Click "Fabrication Output" → "Pick and Place"
2. Export as CSV
3. Verify component positions

## Testing Points to Add

Add test points (1mm pads) for:
- VCC_3V3
- GND (multiple points)
- SPI_SCK, SPI_MISO, SPI_MOSI
- W5500_CS
- USB_DP, USB_DM
- PCIE_PRESENT

## Common Mistakes to Avoid

1. ❌ Forgetting decoupling capacitors
2. ❌ Wrong crystal load capacitors (must be 22pF for 25MHz)
3. ❌ Not matching differential pair lengths
4. ❌ Poor ground plane connectivity
5. ❌ USB-C pins swapped
6. ❌ Wrong footprint for components
7. ❌ No test points for debugging
8. ❌ Traces too thin for power rails

## Verification Checklist

Before ordering PCB:
- [ ] All components have footprints assigned
- [ ] Power connections verified (VCC, GND)
- [ ] SPI connections correct
- [ ] USB-C pinout correct
- [ ] Ethernet pinout correct
- [ ] Crystal and load caps correct
- [ ] All net labels match across sheets
- [ ] Design rule check passes
- [ ] Board outline defined
- [ ] Mounting holes added (if needed)
- [ ] Silkscreen readable
- [ ] Test points added
