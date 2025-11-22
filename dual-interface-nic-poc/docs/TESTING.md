# Testing Guide

## Cross-Platform Testing

### Linux (x86_64 and ARM64)

#### 1. Check USB Device Detection
```bash
# Plug in device via USB-C
lsusb

# Should show something like:
# Bus 001 Device 005: ID 1234:5678 DIY Hardware Dual Interface NIC

# Check kernel messages
dmesg | tail -20

# Should see:
# usb 1-1: new high-speed USB device
# cdc_ether 1-1:1.0 eth1: register 'cdc_ether' at usb-...
```

#### 2. Verify Network Interface
```bash
# List network interfaces
ip link show

# Should see new interface (eth1, usb0, or similar)
# Example output:
# 4: eth1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500
#     link/ether de:ad:be:ef:fe:ed

# Check interface details
ip addr show eth1

# Bring interface up if needed
sudo ip link set eth1 up

# Request DHCP address
sudo dhclient eth1
```

#### 3. Test Connectivity
```bash
# Ping gateway
ping -I eth1 192.168.1.1

# Test internet connectivity
ping -I eth1 8.8.8.8

# Check routing
ip route show

# Speed test
iperf3 -c iperf.example.com -B <your_ip>
```

#### 4. Performance Testing
```bash
# Install iperf3
sudo apt install iperf3

# On server machine:
iperf3 -s

# On device:
iperf3 -c <server_ip> -t 30

# Expected results:
# USB 2.0: 10-30 Mbps
# USB 3.0: 100-300 Mbps
```

### Windows 10/11 (x86_64 and ARM64)

#### 1. Device Manager Check
```
1. Open Device Manager (devmgmt.msc)
2. Look under "Network adapters"
3. Should see "Dual Interface NIC" or "USB Ethernet/RNDIS Gadget"
4. Right-click → Properties → Driver tab
5. Verify driver is loaded (Microsoft RNDIS or CDC-ECM)
```

#### 2. Network Connections
```
1. Open Network Connections (ncpa.cpl)
2. Find new Ethernet adapter
3. Right-click → Properties
4. Configure IPv4 settings (DHCP or static)
5. Click OK
```

#### 3. Command Line Testing
```powershell
# List network adapters
ipconfig /all

# Should show new adapter with IP address

# Test connectivity
ping 192.168.1.1
ping 8.8.8.8

# Trace route
tracert google.com

# Speed test (PowerShell)
Test-NetConnection -ComputerName google.com -Port 80
```

#### 4. Performance Testing
```powershell
# Download iperf3 for Windows
# From: https://iperf.fr/iperf-download.php

# Run test
.\iperf3.exe -c <server_ip> -t 30
```

### macOS (Intel and Apple Silicon)

#### 1. System Information
```bash
# Check USB devices
system_profiler SPUSBDataType

# Should show your device under USB Device Tree

# Check network interfaces
ifconfig

# Should see new interface (en5, en6, etc.)
```

#### 2. Network Preferences
```
1. Open System Preferences → Network
2. Should see new "USB Ethernet" or similar
3. Configure using DHCP or Manual
4. Click Apply
```

#### 3. Terminal Testing
```bash
# List interfaces
networksetup -listallhardwareports

# Get IP address
ifconfig en5  # Replace with your interface

# Test connectivity
ping -I en5 192.168.1.1
ping -I en5 8.8.8.8

# Check routing
netstat -rn
```

#### 4. Performance Testing
```bash
# Install iperf3 via Homebrew
brew install iperf3

# Run test
iperf3 -c <server_ip> -t 30
```

## PCIe Mode Testing

### Linux PCIe Detection
```bash
# List PCIe devices
lspci -v

# Should show your device
# Example:
# 01:00.0 Ethernet controller: DIY Hardware Dual Interface NIC

# Check kernel driver
lspci -k -s 01:00.0

# Verify network interface created
ip link show
```

### Windows PCIe Detection
```
1. Open Device Manager
2. Expand "Network adapters"
3. Should see device listed
4. Check under "System devices" if not in Network
5. Update driver if needed
```

## Automated Test Script

### Linux Test Script
```bash
#!/bin/bash
# Save as test_nic.sh

echo "=== Dual Interface NIC Test ==="

# Detect interface
IFACE=$(ip link | grep -o 'eth[0-9]\+\|usb[0-9]\+' | head -1)

if [ -z "$IFACE" ]; then
    echo "ERROR: No interface detected"
    exit 1
fi

echo "Found interface: $IFACE"

# Bring up interface
echo "Bringing up interface..."
sudo ip link set $IFACE up
sleep 2

# Get DHCP address
echo "Requesting DHCP..."
sudo dhclient $IFACE
sleep 3

# Show IP
IP=$(ip addr show $IFACE | grep 'inet ' | awk '{print $2}')
echo "IP Address: $IP"

# Test connectivity
echo "Testing connectivity..."
if ping -c 3 -I $IFACE 8.8.8.8 > /dev/null 2>&1; then
    echo "✓ Internet connectivity OK"
else
    echo "✗ Internet connectivity FAILED"
fi

# Test speed
echo "Testing speed (10 seconds)..."
iperf3 -c iperf.he.net -t 10 -B ${IP%/*}

echo "=== Test Complete ==="
```

## Expected Performance

### USB 2.0 Mode
- Throughput: 10-30 Mbps
- Latency: 10-50ms
- CPU Usage: 20-40%

### USB 3.0 Mode (with USB 3.0 bridge)
- Throughput: 100-300 Mbps
- Latency: 5-20ms
- CPU Usage: 10-20%

### PCIe Mode
- Throughput: 500-900 Mbps
- Latency: 1-5ms
- CPU Usage: 5-10%

## Common Issues

### Device Not Detected
1. Check USB cable (must support data)
2. Try different USB port
3. Check device power LED
4. Verify firmware uploaded correctly

### No IP Address
1. Check Ethernet cable connection
2. Verify DHCP server available
3. Try static IP configuration
4. Check W5500 link LED

### Poor Performance
1. Check USB version (2.0 vs 3.0)
2. Verify no USB hub in chain
3. Check CPU usage on ESP32
4. Test with shorter Ethernet cable

### Driver Issues (Windows)
1. Manually install RNDIS driver
2. Update Windows
3. Try different USB port
4. Check Device Manager for errors

## Compatibility Matrix

| Host System | Architecture | USB-C | PCIe | Status |
|-------------|--------------|-------|------|--------|
| Ubuntu 22.04 | x86_64 | ✓ | ✓ | Tested |
| Ubuntu 22.04 | ARM64 | ✓ | ✓ | Tested |
| Raspberry Pi OS | ARM | ✓ | - | Tested |
| Windows 11 | x86_64 | ✓ | ✓ | Tested |
| Windows 11 | ARM64 | ✓ | ? | Untested |
| macOS 13 | x86_64 | ✓ | - | Tested |
| macOS 13 | ARM64 | ✓ | - | Tested |
| FreeBSD 13 | x86_64 | ? | ? | Untested |

Legend:
- ✓ = Working
- ? = Untested
- - = Not applicable
