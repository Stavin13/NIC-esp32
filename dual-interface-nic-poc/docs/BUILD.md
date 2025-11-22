# Build Instructions

## Phase 1: USB-C Only Prototype (Recommended Start)

### Components Needed
- ESP32-S3 DevKit with USB-C
- W5500 Ethernet Module
- 7 jumper wires (female-to-female)
- Micro USB cable (for programming)

### Wiring

```
ESP32-S3 Pin    →    W5500 Pin
─────────────────────────────────
GPIO36 (SCK)    →    SCK
GPIO37 (MISO)   →    MISO
GPIO35 (MOSI)   →    MOSI
GPIO34          →    CS
GPIO33          →    RST
3.3V            →    VCC
GND             →    GND
```

### Assembly Steps

1. **Connect Power First**
   - Connect ESP32 3.3V to W5500 VCC
   - Connect ESP32 GND to W5500 GND
   - Double-check polarity!

2. **Connect SPI Signals**
   - Connect all 5 SPI wires as shown above
   - Keep wires short (< 10cm) for signal integrity

3. **Optional: Add Status LEDs**
   ```
   GPIO12 → 330Ω resistor → LED → GND (USB Active)
   GPIO13 → 330Ω resistor → LED → GND (PCIe Active)
   GPIO14 → 330Ω resistor → LED → GND (Network Activity)
   ```

4. **Connect Ethernet Cable**
   - Plug RJ45 cable into W5500 module
   - Connect to your router/switch

### Programming

1. **Install Arduino IDE**
   - Download from arduino.cc
   - Install ESP32 board support

2. **Install Libraries**
   ```
   Tools → Manage Libraries
   Search and install:
   - Ethernet (by Various)
   - SPI (built-in)
   ```

3. **Configure Board**
   ```
   Tools → Board → ESP32 Arduino → ESP32S3 Dev Module
   Tools → USB CDC On Boot → Enabled
   Tools → USB Mode → USB-OTG (TinyUSB)
   Tools → Port → (select your ESP32)
   ```

4. **Upload Firmware**
   - Open `firmware/dual_interface_nic.ino`
   - Click Upload
   - Wait for "Done uploading"

5. **Test**
   - Open Serial Monitor (115200 baud)
   - Should see "Dual Interface NIC - Starting..."
   - Should get IP address via DHCP

## Phase 2: Add PCIe Support (Advanced)

### Additional Components
- PCIe-to-USB adapter board ($15)
- USB-C breakout board ($5)
- Additional jumper wires
- Soldering iron (for modifications)

### PCIe Adapter Integration

1. **Modify PCIe-USB Adapter**
   - Locate USB data lines (D+ and D-)
   - Solder wires to USB data pads
   - Connect to ESP32 USB pins

2. **PCIe Presence Detection**
   ```
   PCIe Pin A16 (PRSNT1#) → 10kΩ pull-up → ESP32 GPIO2
   ```

3. **Power Routing**
   ```
   PCIe 3.3V (Pin A3) → ESP32 3.3V input
   PCIe GND (Pin A4) → ESP32 GND
   ```

4. **Interface Selection Logic**
   - Firmware automatically detects active interface
   - PCIe takes priority if present
   - Falls back to USB-C if PCIe not detected

### Testing PCIe Mode

1. **Insert into PCIe Slot**
   - Power off computer
   - Insert card into PCIe x1 slot
   - Power on computer

2. **Verify Detection**
   ```bash
   # Linux
   lspci | grep -i network
   
   # Should show your device
   ```

3. **Check Network Interface**
   ```bash
   # Linux
   ip link show
   
   # Should show new ethX or similar
   ```

## Enclosure Options

### 3D Printed Case
- Design files in `hardware/enclosure/`
- Print with PLA or PETG
- Includes mounting for both boards
- Cutouts for USB-C and RJ45

### Acrylic Sandwich
- Laser cut 3mm acrylic
- Stack boards with standoffs
- Simple and clean look
- Files in `hardware/acrylic/`

### Heat Shrink Wrap
- Cheapest option
- Use large diameter heat shrink
- Cut holes for connectors
- Not pretty but functional

## Troubleshooting

### No IP Address
- Check Ethernet cable connection
- Verify W5500 power (3.3V)
- Check SPI wiring
- Try static IP instead of DHCP

### USB Not Detected
- Enable "USB CDC On Boot" in Arduino IDE
- Check USB cable (must support data)
- Try different USB port
- Check USB device descriptors

### PCIe Not Working
- Verify PCIe power rails (3.3V present)
- Check PRSNT# pin connection
- Ensure proper grounding
- Test USB mode first

### Compilation Errors
- Update ESP32 board package
- Install all required libraries
- Check board selection
- Verify USB mode settings

## Next Steps

Once basic functionality works:
1. Test on different host systems (x86, ARM)
2. Measure network performance
3. Add advanced features (VLAN, QoS)
4. Design custom PCB
5. Add proper enclosure
