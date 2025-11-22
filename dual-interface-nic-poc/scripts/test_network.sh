#!/bin/bash
# Test network connectivity through the NIC
# Usage: ./test_network.sh

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}=== Dual Interface NIC - Network Test ===${NC}"
echo ""

# Detect the interface
echo "Detecting network interface..."
IFACE=""

# Look for common interface names
for if in eth1 eth2 usb0 enx* enp*; do
    if ip link show $if 2>/dev/null | grep -q "link/ether de:ad:be:ef:fe:ed"; then
        IFACE=$if
        break
    fi
done

if [ -z "$IFACE" ]; then
    echo -e "${YELLOW}Could not auto-detect interface${NC}"
    echo "Available interfaces:"
    ip link show | grep -E "^[0-9]+" | awk '{print $2}' | sed 's/://'
    echo ""
    read -p "Enter interface name: " IFACE
fi

echo -e "Using interface: ${GREEN}$IFACE${NC}"
echo ""

# Check if interface exists
if ! ip link show $IFACE &>/dev/null; then
    echo -e "${RED}Error: Interface $IFACE not found${NC}"
    exit 1
fi

# Bring interface up
echo "Bringing interface up..."
sudo ip link set $IFACE up
sleep 2

# Check link status
if ip link show $IFACE | grep -q "state UP"; then
    echo -e "${GREEN}✓ Interface is UP${NC}"
else
    echo -e "${RED}✗ Interface is DOWN${NC}"
    exit 1
fi

# Get IP address (DHCP)
echo ""
echo "Requesting IP address via DHCP..."
sudo dhclient -v $IFACE 2>&1 | grep -E "DHCPACK|bound to" || true
sleep 3

# Show IP configuration
IP=$(ip addr show $IFACE | grep 'inet ' | awk '{print $2}')
if [ -n "$IP" ]; then
    echo -e "${GREEN}✓ IP Address: $IP${NC}"
else
    echo -e "${RED}✗ No IP address assigned${NC}"
    echo "Trying static IP..."
    sudo ip addr add 192.168.1.100/24 dev $IFACE
    IP="192.168.1.100/24"
fi

# Show interface details
echo ""
echo "Interface details:"
ip addr show $IFACE
echo ""

# Test connectivity
echo "Testing connectivity..."
echo ""

# Test 1: Ping gateway
GATEWAY=$(ip route | grep default | grep $IFACE | awk '{print $3}')
if [ -n "$GATEWAY" ]; then
    echo -n "Ping gateway ($GATEWAY): "
    if ping -c 3 -W 2 -I $IFACE $GATEWAY &>/dev/null; then
        echo -e "${GREEN}✓ Success${NC}"
    else
        echo -e "${RED}✗ Failed${NC}"
    fi
else
    echo -e "${YELLOW}No gateway found${NC}"
fi

# Test 2: Ping DNS
echo -n "Ping DNS (8.8.8.8): "
if ping -c 3 -W 2 -I $IFACE 8.8.8.8 &>/dev/null; then
    echo -e "${GREEN}✓ Success${NC}"
else
    echo -e "${RED}✗ Failed${NC}"
fi

# Test 3: DNS resolution
echo -n "DNS resolution (google.com): "
if nslookup google.com &>/dev/null; then
    echo -e "${GREEN}✓ Success${NC}"
else
    echo -e "${RED}✗ Failed${NC}"
fi

# Test 4: HTTP request
echo -n "HTTP request (http://example.com): "
if curl -s --interface $IFACE --max-time 5 http://example.com >/dev/null; then
    echo -e "${GREEN}✓ Success${NC}"
else
    echo -e "${RED}✗ Failed${NC}"
fi

# Performance test
echo ""
echo "Performance test..."
echo ""

# Download speed test
echo "Testing download speed (10 seconds)..."
if command -v iperf3 &>/dev/null; then
    # Use iperf3 if available
    echo "Using iperf3 (connect to public server)..."
    iperf3 -c iperf.he.net -t 10 -B ${IP%/*} 2>/dev/null || echo "iperf3 test failed"
else
    # Fallback to wget
    echo "Using wget..."
    SPEED=$(wget --bind-address=${IP%/*} -O /dev/null http://speedtest.tele2.net/1MB.zip 2>&1 | \
            grep -oP '\(\K[0-9.]+ [KM]B/s')
    if [ -n "$SPEED" ]; then
        echo -e "Download speed: ${GREEN}$SPEED${NC}"
    else
        echo "Speed test failed"
    fi
fi

# Statistics
echo ""
echo "Interface statistics:"
ip -s link show $IFACE

echo ""
echo -e "${GREEN}=== Test Complete ===${NC}"
