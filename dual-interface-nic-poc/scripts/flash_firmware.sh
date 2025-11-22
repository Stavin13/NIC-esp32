#!/bin/bash
# Flash firmware to ESP32-S3
# Usage: ./flash_firmware.sh [port]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Dual Interface NIC - Firmware Flasher ===${NC}"

# Detect port if not provided
if [ -z "$1" ]; then
    echo "Detecting ESP32 port..."
    
    # Try common port names
    if [ -e /dev/ttyUSB0 ]; then
        PORT=/dev/ttyUSB0
    elif [ -e /dev/ttyACM0 ]; then
        PORT=/dev/ttyACM0
    elif [ -e /dev/cu.usbserial* ]; then
        PORT=$(ls /dev/cu.usbserial* | head -1)
    elif [ -e /dev/cu.SLAB_USBtoUART ]; then
        PORT=/dev/cu.SLAB_USBtoUART
    else
        echo -e "${RED}Error: Could not detect ESP32 port${NC}"
        echo "Please specify port manually: $0 /dev/ttyUSB0"
        exit 1
    fi
else
    PORT=$1
fi

echo -e "Using port: ${GREEN}$PORT${NC}"

# Check if esptool is installed
if ! command -v esptool.py &> /dev/null; then
    echo -e "${YELLOW}esptool.py not found. Installing...${NC}"
    pip install esptool
fi

# Check if Arduino CLI is available
if command -v arduino-cli &> /dev/null; then
    echo -e "${GREEN}Using Arduino CLI${NC}"
    
    # Compile and upload
    arduino-cli compile --fqbn esp32:esp32:esp32s3 firmware/dual_interface_nic.ino
    arduino-cli upload -p $PORT --fqbn esp32:esp32:esp32s3 firmware/dual_interface_nic.ino
    
else
    echo -e "${YELLOW}Arduino CLI not found${NC}"
    echo "Please compile in Arduino IDE and use esptool manually"
    echo ""
    echo "Or install Arduino CLI:"
    echo "  curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh"
    exit 1
fi

echo -e "${GREEN}Firmware flashed successfully!${NC}"
echo ""
echo "Opening serial monitor..."
echo "Press Ctrl+C to exit"
echo ""

# Open serial monitor
if command -v screen &> /dev/null; then
    screen $PORT 115200
elif command -v minicom &> /dev/null; then
    minicom -D $PORT -b 115200
else
    echo "Install 'screen' or 'minicom' to view serial output"
    echo "Or use Arduino IDE Serial Monitor"
fi
