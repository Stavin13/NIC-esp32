/*
 * Dual Interface NIC - Proof of Concept
 * 
 * Cross-platform network interface supporting both USB-C and PCIe
 * Works on x86 and ARM host systems
 * 
 * Hardware: ESP32-S3 + W5500 Ethernet Module
 * 
 * Security Features:
 * - MAC Address Filtering
 * - Packet Inspection (Firewall)
 * - VPN Encryption
 * - Secure Boot Support
 */

#include <SPI.h>
#include <Ethernet.h>
#include "USB.h"
#include "USBCDC.h"
#include "security_features.h"

// Pin definitions for W5500
#define W5500_CS    34
#define W5500_RST   33
#define W5500_MISO  37
#define W5500_MOSI  35
#define W5500_SCK   36

// Interface detection pins
#define PCIE_PRESENT_PIN  2
#define USB_CC_DETECT_PIN 15

// Status LED pins
#define LED_USB_ACTIVE    12
#define LED_PCIE_ACTIVE   13
#define LED_NETWORK       14

// Interface types
enum InterfaceType {
  INTERFACE_NONE,
  INTERFACE_USB_C,
  INTERFACE_PCIE
};

InterfaceType activeInterface = INTERFACE_NONE;

// MAC address for Ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// USB CDC for network interface
USBCDC USBSerial;

// Buffer for packet bridging
#define BUFFER_SIZE 1500
uint8_t packetBuffer[BUFFER_SIZE];

// Security features
MACFilter macFilter;
PacketInspector packetInspector;
VPNEncryption vpnEncryption;

// Security configuration
bool enableMACFiltering = true;
bool enableFirewall = true;
bool enableVPN = false; // Disabled by default (performance impact)

void setup() {
  // Initialize pins
  pinMode(PCIE_PRESENT_PIN, INPUT_PULLUP);
  pinMode(USB_CC_DETECT_PIN, INPUT);
  pinMode(LED_USB_ACTIVE, OUTPUT);
  pinMode(LED_PCIE_ACTIVE, OUTPUT);
  pinMode(LED_NETWORK, OUTPUT);
  
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Dual Interface NIC - Starting...");
  Serial.println("With Security Features Enabled");
  
  // Initialize security features
  initSecurityFeatures();
  
  // Detect active interface
  activeInterface = detectInterface();
  
  // Initialize USB CDC as network device
  if (activeInterface == INTERFACE_USB_C || activeInterface == INTERFACE_PCIE) {
    initUSBNetworkDevice();
  }
  
  // Initialize W5500 Ethernet
  initEthernet();
  
  // Update status LEDs
  updateStatusLEDs();
  
  Serial.print("Active Interface: ");
  Serial.println(getInterfaceName(activeInterface));
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Maintain DHCP lease
  Ethernet.maintain();
  
  // Bridge network traffic between USB/PCIe and Ethernet
  bridgeNetworkTraffic();
  
  // Check for interface changes (hot-plug detection)
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 1000) {
    InterfaceType newInterface = detectInterface();
    if (newInterface != activeInterface) {
      Serial.println("Interface changed, reinitializing...");
      activeInterface = newInterface;
      updateStatusLEDs();
    }
    lastCheck = millis();
  }
  
  // Blink network LED on activity
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 500) {
    digitalWrite(LED_NETWORK, !digitalRead(LED_NETWORK));
    lastBlink = millis();
  }
}

InterfaceType detectInterface() {
  // Check PCIe presence (active low)
  if (digitalRead(PCIE_PRESENT_PIN) == LOW) {
    Serial.println("PCIe interface detected");
    return INTERFACE_PCIE;
  }
  
  // Check USB-C connection via CC pin
  int ccVoltage = analogRead(USB_CC_DETECT_PIN);
  if (ccVoltage > 500) {  // Threshold for USB-C connection
    Serial.println("USB-C interface detected");
    return INTERFACE_USB_C;
  }
  
  // Default to USB if nothing else detected
  return INTERFACE_USB_C;
}

void initUSBNetworkDevice() {
  // Configure USB device descriptors
  USB.VID(0x1234);  // Vendor ID (use your own)
  USB.PID(0x5678);  // Product ID (use your own)
  USB.productName("Dual Interface NIC");
  USB.manufacturerName("DIY Hardware");
  USB.serialNumber("001");
  
  // Start USB CDC
  USBSerial.begin();
  USB.begin();
  
  Serial.println("USB Network Device initialized");
}

void initEthernet() {
  // Reset W5500
  pinMode(W5500_RST, OUTPUT);
  digitalWrite(W5500_RST, LOW);
  delay(10);
  digitalWrite(W5500_RST, HIGH);
  delay(100);
  
  // Initialize SPI with custom pins
  SPI.begin(W5500_SCK, W5500_MISO, W5500_MOSI, W5500_CS);
  
  // Initialize Ethernet with DHCP
  Serial.println("Initializing Ethernet...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP failed, using static IP");
    IPAddress ip(192, 168, 1, 177);
    IPAddress dns(192, 168, 1, 1);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    Ethernet.begin(mac, ip, dns, gateway, subnet);
  }
  
  Serial.println("Ethernet initialized");
}

void bridgeNetworkTraffic() {
  // Check for incoming packets from Ethernet
  int packetSize = Ethernet.parsePacket();
  if (packetSize > 0) {
    // Read packet from Ethernet
    int len = Ethernet.read(packetBuffer, min(packetSize, BUFFER_SIZE));
    
    // Security checks
    if (!processIncomingPacket(packetBuffer, len)) {
      return; // Packet blocked by security
    }
    
    // Forward to USB/PCIe interface
    if (USBSerial.connected()) {
      USBSerial.write(packetBuffer, len);
    }
  }
  
  // Check for incoming packets from USB/PCIe
  if (USBSerial.available()) {
    int len = USBSerial.readBytes(packetBuffer, BUFFER_SIZE);
    
    // Security checks
    if (!processOutgoingPacket(packetBuffer, len)) {
      return; // Packet blocked by security
    }
    
    // Forward to Ethernet
    // Note: This is simplified - real implementation needs proper packet parsing
    Ethernet.beginPacket(Ethernet.remoteIP(), Ethernet.remotePort());
    Ethernet.write(packetBuffer, len);
    Ethernet.endPacket();
  }
}

void updateStatusLEDs() {
  // Turn off all LEDs
  digitalWrite(LED_USB_ACTIVE, LOW);
  digitalWrite(LED_PCIE_ACTIVE, LOW);
  
  // Turn on active interface LED
  if (activeInterface == INTERFACE_USB_C) {
    digitalWrite(LED_USB_ACTIVE, HIGH);
  } else if (activeInterface == INTERFACE_PCIE) {
    digitalWrite(LED_PCIE_ACTIVE, HIGH);
  }
}

const char* getInterfaceName(InterfaceType type) {
  switch (type) {
    case INTERFACE_USB_C: return "USB-C";
    case INTERFACE_PCIE: return "PCIe";
    default: return "None";
  }
}

// ============================================
// SECURITY FEATURE IMPLEMENTATIONS
// ============================================

void initSecurityFeatures() {
  Serial.println("\n=== Initializing Security Features ===");
  
  // 1. MAC Address Filtering
  if (enableMACFiltering) {
    Serial.println("MAC Filtering: ENABLED");
    
    // Example: Block a specific MAC address
    uint8_t blockedMAC[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    macFilter.addBlockedMAC(blockedMAC);
    
    // Example: Allow only specific MACs (whitelist mode)
    // macFilter.setWhitelistMode(true);
    // uint8_t allowedMAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    // macFilter.addAllowedMAC(allowedMAC);
  } else {
    Serial.println("MAC Filtering: DISABLED");
  }
  
  // 2. Packet Inspection (Firewall)
  if (enableFirewall) {
    Serial.println("Firewall: ENABLED");
    
    // Example: Block ICMP (ping)
    // packetInspector.blockProtocol(PacketInspector::ICMP, true);
    
    // Example: Block specific ports
    packetInspector.addBlockedPort(23);   // Telnet
    packetInspector.addBlockedPort(135);  // Windows RPC
    packetInspector.addBlockedPort(445);  // SMB
    
    Serial.println("Blocked ports: 23, 135, 445");
  } else {
    Serial.println("Firewall: DISABLED");
  }
  
  // 3. VPN Encryption
  if (enableVPN) {
    Serial.println("VPN: ENABLED");
    
    // Set encryption key (in production, load from secure storage)
    uint8_t vpnKey[32] = {
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
    };
    
    if (vpnEncryption.setKey(vpnKey, 32)) {
      Serial.println("VPN encryption key set");
    } else {
      Serial.println("VPN encryption key setup failed!");
      enableVPN = false;
    }
  } else {
    Serial.println("VPN: DISABLED (enable for encrypted traffic)");
  }
  
  // 4. Secure Boot Status
  SecureBoot::printStatus();
  SecureBoot::verifyFirmware();
  
  Serial.println("=====================================\n");
}

bool processIncomingPacket(uint8_t* packet, int length) {
  // 1. MAC Address Filtering
  if (enableMACFiltering && length >= 12) {
    uint8_t* srcMAC = packet + 6; // Source MAC is at offset 6
    if (!macFilter.isAllowed(srcMAC)) {
      Serial.print("Blocked MAC: ");
      macFilter.printMAC(srcMAC);
      return false;
    }
  }
  
  // 2. Packet Inspection (Firewall)
  if (enableFirewall) {
    if (!packetInspector.inspectPacket(packet, length)) {
      return false; // Blocked by firewall
    }
  }
  
  // 3. VPN Decryption
  if (enableVPN) {
    vpnEncryption.decryptPacket(packet, length);
  }
  
  return true; // Packet allowed
}

bool processOutgoingPacket(uint8_t* packet, int length) {
  // 1. Packet Inspection (Firewall)
  if (enableFirewall) {
    if (!packetInspector.inspectPacket(packet, length)) {
      return false; // Blocked by firewall
    }
  }
  
  // 2. VPN Encryption
  if (enableVPN) {
    vpnEncryption.encryptPacket(packet, length);
  }
  
  return true; // Packet allowed
}
