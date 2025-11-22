# Architecture Documentation

## System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                     Host Computer                            │
│                  (x86, ARM, RISC-V)                         │
└────────────────┬────────────────────┬───────────────────────┘
                 │                    │
          ┌──────▼──────┐      ┌─────▼──────┐
          │   USB-C     │      │   PCIe     │
          │  Interface  │      │ Interface  │
          └──────┬──────┘      └─────┬──────┘
                 │                    │
                 └────────┬───────────┘
                          │
                 ┌────────▼────────┐
                 │   ESP32-S3      │
                 │ (Bridge/Router) │
                 └────────┬────────┘
                          │
                 ┌────────▼────────┐
                 │     W5500       │
                 │ Ethernet MAC/PHY│
                 └────────┬────────┘
                          │
                    ┌─────▼─────┐
                    │  Network  │
                    └───────────┘
```

## Component Responsibilities

### ESP32-S3 (Main Controller)
- **Interface Detection**: Automatically detects USB-C or PCIe connection
- **Protocol Bridging**: Translates between USB/PCIe and SPI
- **Packet Routing**: Forwards network packets between interfaces
- **Power Management**: Handles power from USB or PCIe
- **Status Indication**: Controls LEDs for system status

### W5500 (Ethernet Controller)
- **Hardware TCP/IP Stack**: Offloads network processing
- **MAC/PHY Integration**: Complete Ethernet solution
- **Packet Buffering**: 32KB internal buffer
- **Auto-negotiation**: 10/100 Mbps speed detection
- **Link Management**: Handles physical layer

### USB-C Interface
- **Device Mode**: Presents as CDC-ECM network adapter
- **Power Delivery**: Negotiates power requirements
- **Hot-plug Support**: Dynamic connection/disconnection
- **Cross-platform**: Standard drivers on all OS

### PCIe Interface (via Bridge)
- **Protocol Conversion**: PCIe to USB translation
- **Power Rails**: Uses PCIe 3.3V and 12V
- **Presence Detection**: PRSNT# pin monitoring
- **Higher Performance**: Lower latency than USB

## Data Flow

### Outbound (Host → Network)

```
1. Host Application
   ↓ (TCP/IP stack)
2. Network Driver (CDC-ECM/RNDIS)
   ↓ (USB packets)
3. USB Controller (Host)
   ↓ (USB protocol)
4. ESP32 USB Interface
   ↓ (packet extraction)
5. ESP32 Firmware (routing logic)
   ↓ (SPI commands)
6. W5500 Controller
   ↓ (Ethernet frames)
7. Physical Network
```

### Inbound (Network → Host)

```
1. Physical Network
   ↓ (Ethernet frames)
2. W5500 Controller
   ↓ (interrupt to ESP32)
3. ESP32 Firmware (SPI read)
   ↓ (packet processing)
4. ESP32 USB Interface
   ↓ (USB packets)
5. USB Controller (Host)
   ↓ (driver processing)
6. Network Driver
   ↓ (TCP/IP stack)
7. Host Application
```

## Interface Detection Logic

```cpp
Priority Order:
1. Check PCIe PRSNT# pin (active low)
   - If LOW: PCIe is present → Use PCIe mode
   
2. Check USB-C CC pins (voltage detection)
   - If voltage > threshold: USB-C connected → Use USB mode
   
3. Default: USB mode (most compatible)
```

## Power Architecture

### USB-C Power Path
```
USB-C VBUS (5V) → Buck Converter → 3.3V → ESP32 + W5500
                                         → Status LEDs
```

### PCIe Power Path
```
PCIe 12V → Buck Converter → 5V → Buck Converter → 3.3V
PCIe 3.3V → Direct (with filtering) → 3.3V Rail
```

### Power Selection
```
if (PCIe_Present) {
    Use PCIe power (higher current available)
} else if (USB_Connected) {
    Use USB power (limited to 500mA/900mA)
} else {
    Low power mode
}
```

## Memory Architecture

### ESP32-S3 Memory Usage
```
Flash (8MB):
├── Bootloader: 32KB
├── Partition Table: 4KB
├── Firmware: ~1MB
├── OTA Update: 1MB (reserved)
└── File System: Remaining

RAM (512KB):
├── System: ~100KB
├── WiFi Stack: ~50KB (unused in this project)
├── Packet Buffers: 32KB
├── USB Stack: ~20KB
└── Application: Remaining
```

### W5500 Memory
```
Internal SRAM (32KB):
├── TX Buffers: 16KB (configurable)
└── RX Buffers: 16KB (configurable)

Socket Allocation:
├── Socket 0: 8KB TX, 8KB RX (main data)
├── Socket 1-7: Unused (available for expansion)
```

## Protocol Stack

### USB CDC-ECM Stack
```
Layer 7: Application Data
Layer 4: TCP/UDP (handled by host)
Layer 3: IP (handled by host)
Layer 2: Ethernet Frames
         ↓
    CDC-ECM Encapsulation
         ↓
    USB Bulk Transfer
         ↓
    USB Protocol Layer
         ↓
    Physical USB
```

### SPI Communication (ESP32 ↔ W5500)
```
Command Structure:
┌──────────┬──────────┬──────────┬──────────┐
│ OpCode   │ Address  │ Control  │ Data     │
│ (1 byte) │ (2 bytes)│ (1 byte) │ (N bytes)│
└──────────┴──────────┴──────────┴──────────┘

OpCode:
- 0x00: Read
- 0x04: Write

Control Byte:
- Block Select (Socket, Common, etc.)
- Read/Write mode
```

## Performance Characteristics

### Latency Budget
```
USB-C Mode:
├── USB Protocol: 1-5ms
├── ESP32 Processing: 1-3ms
├── SPI Transfer: 0.1-0.5ms
├── W5500 Processing: 0.5-1ms
└── Total: ~3-10ms

PCIe Mode:
├── PCIe Protocol: 0.1-1ms
├── Bridge Conversion: 0.5-2ms
├── ESP32 Processing: 1-3ms
├── SPI Transfer: 0.1-0.5ms
├── W5500 Processing: 0.5-1ms
└── Total: ~2-8ms
```

### Throughput Limits
```
Theoretical Maximum:
├── W5500: 100 Mbps (hardware limit)
├── SPI @ 33MHz: ~264 Mbps (sufficient)
├── USB 2.0: 480 Mbps (sufficient)
├── USB 3.0: 5 Gbps (overkill)
└── PCIe Gen2 x1: 5 Gbps (overkill)

Practical Throughput:
├── USB 2.0 Mode: 10-30 Mbps
├── USB 3.0 Mode: 50-90 Mbps
└── PCIe Mode: 70-95 Mbps
```

## Cross-Platform Compatibility

### Driver Architecture
```
Linux:
├── USB: cdc_ether kernel module (built-in)
├── PCIe: Custom driver or USB bridge driver
└── Interface: Standard netdev

Windows:
├── USB: RNDIS driver (built-in)
├── PCIe: Custom driver or USB bridge driver
└── Interface: NDIS miniport

macOS:
├── USB: CDC-ECM driver (built-in)
├── PCIe: Custom driver required
└── Interface: BSD network stack
```

### Device Enumeration
```
1. Host detects USB device
2. Reads device descriptor
3. Identifies as CDC class (0x02)
4. Loads appropriate driver
5. Creates network interface
6. Assigns interface name (eth0, en0, etc.)
7. Ready for network configuration
```

## Security Considerations

### Attack Surface
```
Potential Vulnerabilities:
├── USB Fuzzing: Malformed USB packets
├── Network Injection: Malicious Ethernet frames
├── Firmware Tampering: Unauthorized updates
└── Side-channel: Power analysis

Mitigations:
├── Input validation on all packets
├── Firmware signature verification
├── Secure boot (ESP32-S3 feature)
└── Rate limiting on packet processing
```

### Isolation
```
ESP32 Firmware:
├── No direct memory access to host
├── Sandboxed execution
├── Limited attack surface
└── Can be reset by host

W5500:
├── Hardware TCP/IP stack (isolated)
├── No code execution capability
├── Fixed functionality
└── Physical layer only
```

## Extensibility

### Future Enhancements
```
Hardware:
├── Add Wi-Fi mode (ESP32 built-in)
├── Multiple Ethernet ports
├── PoE support
├── Gigabit Ethernet (different PHY)
└── USB 3.0 native support

Firmware:
├── VLAN tagging
├── QoS/traffic shaping
├── Packet filtering/firewall
├── VPN offload
└── Network monitoring/statistics

Software:
├── Web configuration interface
├── SNMP support
├── Remote management
├── Firmware OTA updates
└── Custom driver features
```

## Debug and Monitoring

### Debug Interfaces
```
Serial Console (UART):
├── Boot messages
├── Firmware logs
├── Packet statistics
└── Error reporting

Status LEDs:
├── Power: Solid when powered
├── USB Active: Blinks on USB traffic
├── PCIe Active: Blinks on PCIe traffic
└── Network: Blinks on Ethernet activity
```

### Monitoring Points
```
ESP32 Metrics:
├── CPU usage
├── Memory usage
├── Packet counts (TX/RX)
├── Error counts
└── Interface state

W5500 Metrics:
├── Link status
├── Speed/duplex
├── Packet statistics
├── Buffer usage
└── PHY registers
```
