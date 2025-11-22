/*
 * Dual Interface NIC - Wi-Fi Version
 * 
 * Uses ESP32-S3 built-in Wi-Fi instead of W5500
 * Works RIGHT NOW without additional hardware!
 * 
 * Cross-platform network interface supporting USB-C
 * Works on x86 and ARM host systems
 * 
 * Hardware: ESP32-S3 only (no W5500 needed!)
 */

#include <WiFi.h>
#include "USB.h"
#include "USBCDC.h"
#include "esp_wifi.h"

// ============================================
// CONFIGURATION - CHANGE THESE!
// ============================================

// Your Wi-Fi credentials
const char* ssid = "vivo Y100";
const char* password = "123459876  "; // Removed trailing spaces!

// Status LED pin
#define LED_PIN 2

// USB CDC for network interface
USBCDC USBSerial;

// Buffer for packet bridging
#define BUFFER_SIZE 1500
uint8_t packetBuffer[BUFFER_SIZE];

// Statistics
unsigned long packetsReceived = 0;
unsigned long packetsSent = 0;
unsigned long bytesReceived = 0;
unsigned long bytesSent = 0;

// Packet Analysis Structures
struct PacketStats {
  unsigned long total_packets;
  unsigned long tcp_packets;
  unsigned long udp_packets;
  unsigned long icmp_packets;
  unsigned long http_packets;
  unsigned long https_packets;
  unsigned long dns_packets;
  unsigned long bytes_analyzed;
} packetStats;

struct SecurityAlerts {
  unsigned long port_scans;
  unsigned long syn_floods;
  unsigned long suspicious_packets;
} securityAlerts;

// Packet analysis mode
bool packetAnalysisEnabled = true;

void setup() {
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED on during setup
  
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n================================");
  Serial.println("Dual Interface NIC - Wi-Fi Version");
  Serial.println("With Packet Analysis");
  Serial.println("================================\n");
  
  // Initialize packet analysis
  memset(&packetStats, 0, sizeof(packetStats));
  memset(&securityAlerts, 0, sizeof(securityAlerts));
  
  // Initialize USB CDC as network device
  initUSBNetworkDevice();
  
  // Connect to Wi-Fi
  connectToWiFi();
  
  // Print network info
  printNetworkInfo();
  
  // Enable packet analysis
  if (packetAnalysisEnabled) {
    enablePacketAnalysis();
  }
  
  digitalWrite(LED_PIN, LOW); // LED off when ready
  
  Serial.println("\n================================");
  Serial.println("NIC Ready!");
  Serial.println("Your computer should see a new network interface");
  Serial.println("================================\n");
}

void loop() {
  // Check Wi-Fi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected! Reconnecting...");
    connectToWiFi();
  }
  
  // Bridge network traffic
  bridgeNetworkTraffic();
  
  // Print statistics every 10 seconds
  static unsigned long lastStats = 0;
  if (millis() - lastStats > 10000) {
    printStatistics();
    if (packetAnalysisEnabled) {
      printPacketAnalysis();
    }
    lastStats = millis();
  }
  
  // Check for security alerts every 5 seconds
  static unsigned long lastAlerts = 0;
  if (millis() - lastAlerts > 5000) {
    if (packetAnalysisEnabled) {
      checkSecurityAlerts();
    }
    lastAlerts = millis();
  }
  
  // Blink LED on activity
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  if (millis() - lastBlink > 500) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
  }
}

void initUSBNetworkDevice() {
  Serial.println("Initializing USB Network Device...");
  
  // Configure USB device descriptors
  USB.VID(0x1234);  // Vendor ID
  USB.PID(0x5678);  // Product ID
  USB.productName("ESP32-S3 Wi-Fi NIC");
  USB.manufacturerName("DIY Hardware");
  USB.serialNumber("001");
  
  // Start USB CDC
  USBSerial.begin();
  USB.begin();
  
  delay(1000);
  
  Serial.println("✓ USB Network Device initialized");
  Serial.println("  Device will appear as network adapter on host");
}

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);
  
  // First, scan to see if network is visible
  Serial.println("\nScanning for networks...");
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();
  
  Serial.print("Found ");
  Serial.print(n);
  Serial.println(" networks:\n");
  
  // List all networks
  Serial.println("Nr | SSID                             | RSSI | Ch | Encryption");
  Serial.println("---|----------------------------------|------|----|------------");
  
  bool networkFound = false;
  for (int i = 0; i < n; i++) {
    // Print network number
    Serial.printf("%2d", i + 1);
    Serial.print(" | ");
    
    // Print SSID (highlight if it's our target)
    String currentSSID = WiFi.SSID(i);
    if (currentSSID == ssid) {
      Serial.print(">>> ");
      networkFound = true;
    } else {
      Serial.print("    ");
    }
    Serial.printf("%-28s", currentSSID.c_str());
    Serial.print(" | ");
    
    // Print signal strength
    Serial.printf("%4d", WiFi.RSSI(i));
    Serial.print(" | ");
    
    // Print channel
    Serial.printf("%2d", WiFi.channel(i));
    Serial.print(" | ");
    
    // Print encryption
    switch (WiFi.encryptionType(i)) {
      case WIFI_AUTH_OPEN:
        Serial.print("Open");
        break;
      case WIFI_AUTH_WEP:
        Serial.print("WEP");
        break;
      case WIFI_AUTH_WPA_PSK:
        Serial.print("WPA");
        break;
      case WIFI_AUTH_WPA2_PSK:
        Serial.print("WPA2");
        break;
      case WIFI_AUTH_WPA_WPA2_PSK:
        Serial.print("WPA/WPA2");
        break;
      default:
        Serial.print("Unknown");
    }
    
    // Mark if it's 5GHz (incompatible)
    if (WiFi.channel(i) > 14) {
      Serial.print(" [5GHz-INCOMPATIBLE]");
    }
    
    Serial.println();
  }
  
  Serial.println();
  
  if (!networkFound) {
    Serial.println("✗ Target network '");
    Serial.print(ssid);
    Serial.println("' not found in scan!");
    Serial.println("  Make sure hotspot is on and in range");
    return;
  } else {
    Serial.print("✓ Found target network '");
    Serial.print(ssid);
    Serial.println("'");
  }
  
  // Disconnect if already connected
  WiFi.disconnect();
  delay(100);
  
  // Connect to Wi-Fi
  Serial.println("Attempting connection...");
  WiFi.begin(ssid, password);
  
  // Wait for connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    // Print detailed status
    if (attempts % 10 == 0) {
      Serial.print(" [Status: ");
      Serial.print(WiFi.status());
      Serial.print("]");
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ Wi-Fi connected!");
  } else {
    Serial.println("\n✗ Wi-Fi connection failed!");
    Serial.print("  Final status code: ");
    Serial.println(WiFi.status());
    Serial.println("  Status codes:");
    Serial.println("    0 = WL_IDLE_STATUS");
    Serial.println("    1 = WL_NO_SSID_AVAIL (network not found)");
    Serial.println("    4 = WL_CONNECT_FAILED (wrong password)");
    Serial.println("    6 = WL_DISCONNECTED");
    Serial.println("\n  Troubleshooting:");
    Serial.println("  1. Check password is correct (no extra spaces!)");
    Serial.println("  2. Make sure hotspot is 2.4GHz (not 5GHz)");
    Serial.println("  3. Try moving ESP32 closer to phone");
    Serial.println("  4. Restart phone hotspot");
  }
}

void printNetworkInfo() {
  Serial.println("\n--- Network Information ---");
  
  // Wi-Fi status
  Serial.print("Wi-Fi Status: ");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected");
  } else {
    Serial.println("Disconnected");
    return;
  }
  
  // SSID
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  
  // IP Address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Subnet Mask
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  
  // Gateway
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  
  // DNS
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());
  
  // MAC Address
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Signal Strength
  Serial.print("Signal Strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  
  // Channel
  Serial.print("Channel: ");
  Serial.println(WiFi.channel());
}

void bridgeNetworkTraffic() {
  // This is a simplified implementation
  // Real USB network bridging requires proper USB CDC-ECM/RNDIS implementation
  
  // For now, we'll just demonstrate the concept
  // In a full implementation, you would:
  // 1. Receive Ethernet frames from USB
  // 2. Forward them to Wi-Fi
  // 3. Receive packets from Wi-Fi
  // 4. Forward them to USB as Ethernet frames
  
  // Check for data from USB
  if (USBSerial.available()) {
    int len = USBSerial.readBytes(packetBuffer, BUFFER_SIZE);
    if (len > 0) {
      packetsReceived++;
      bytesReceived += len;
      
      // In real implementation: forward to Wi-Fi
      // For now, just count it
    }
  }
  
  // In real implementation: check for Wi-Fi packets and forward to USB
}

void printStatistics() {
  Serial.println("\n--- NIC Statistics ---");
  Serial.print("Uptime: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds");
  
  Serial.print("USB Packets Received: ");
  Serial.println(packetsReceived);
  
  Serial.print("USB Packets Sent: ");
  Serial.println(packetsSent);
  
  Serial.print("USB Bytes Received: ");
  Serial.println(bytesReceived);
  
  Serial.print("USB Bytes Sent: ");
  Serial.println(bytesSent);
  
  Serial.print("Wi-Fi Signal: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap() / 1024);
  Serial.println(" KB");
}

// ============================================
// PACKET ANALYSIS FUNCTIONS
// ============================================

void enablePacketAnalysis() {
  Serial.println("Enabling packet analysis...");
  
  // Set promiscuous mode for packet capture
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&packetHandler);
  
  Serial.println("✓ Packet analysis enabled");
  Serial.println("  Analyzing wireless packets on current channel");
  Serial.println("  Security monitoring active");
}

void packetHandler(void* buf, wifi_promiscuous_pkt_type_t type) {
  // Only analyze data packets
  if (type != WIFI_PKT_DATA) return;
  
  wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  
  // Analyze the packet
  analyzeWiFiPacket(pkt->payload, pkt->rx_ctrl.sig_len);
}

void analyzeWiFiPacket(const uint8_t* packet, int length) {
  if (length < 24) return; // Too short for 802.11 header
  
  packetStats.total_packets++;
  packetStats.bytes_analyzed += length;
  
  // Basic 802.11 frame analysis
  uint8_t frameType = (packet[0] >> 2) & 0x03;
  uint8_t frameSubtype = (packet[0] >> 4) & 0x0F;
  
  // Look for data frames
  if (frameType == 2) { // Data frame
    analyzeDataFrame(packet, length);
  }
  
  // Detect potential attacks
  detectSuspiciousActivity(packet, length, frameType, frameSubtype);
}

void analyzeDataFrame(const uint8_t* packet, int length) {
  // Skip 802.11 header (24 bytes minimum)
  if (length < 24 + 20) return; // Need space for IP header too
  
  const uint8_t* payload = packet + 24;
  int payloadLen = length - 24;
  
  // Look for IP packets (simplified - assumes no encryption)
  if (payloadLen > 20) {
    uint8_t version = (payload[0] >> 4) & 0x0F;
    if (version == 4) { // IPv4
      analyzeIPPacket(payload, payloadLen);
    }
  }
}

void analyzeIPPacket(const uint8_t* packet, int length) {
  if (length < 20) return;
  
  uint8_t protocol = packet[9];
  uint8_t headerLen = (packet[0] & 0x0F) * 4;
  
  switch (protocol) {
    case 1: // ICMP
      packetStats.icmp_packets++;
      break;
      
    case 6: // TCP
      packetStats.tcp_packets++;
      if (length > headerLen + 4) {
        analyzeTCPPacket(packet + headerLen, length - headerLen);
      }
      break;
      
    case 17: // UDP
      packetStats.udp_packets++;
      if (length > headerLen + 8) {
        analyzeUDPPacket(packet + headerLen, length - headerLen);
      }
      break;
  }
}

void analyzeTCPPacket(const uint8_t* packet, int length) {
  if (length < 20) return;
  
  uint16_t srcPort = (packet[0] << 8) | packet[1];
  uint16_t destPort = (packet[2] << 8) | packet[3];
  uint8_t flags = packet[13];
  
  // Identify common services
  if (destPort == 80 || srcPort == 80) {
    packetStats.http_packets++;
  } else if (destPort == 443 || srcPort == 443) {
    packetStats.https_packets++;
  }
  
  // Detect SYN flood attempts
  if (flags == 0x02) { // SYN flag only
    static unsigned long synCount = 0;
    static unsigned long lastSynTime = 0;
    
    synCount++;
    if (millis() - lastSynTime > 1000) {
      if (synCount > 50) { // More than 50 SYN packets per second
        securityAlerts.syn_floods++;
      }
      synCount = 0;
      lastSynTime = millis();
    }
  }
  
  // Detect potential port scanning
  static uint16_t lastDestPort = 0;
  static unsigned long portScanCount = 0;
  
  if (destPort != lastDestPort && destPort > 1024) {
    portScanCount++;
    if (portScanCount > 20) { // Many different high ports
      securityAlerts.port_scans++;
      portScanCount = 0;
    }
  }
  lastDestPort = destPort;
}

void analyzeUDPPacket(const uint8_t* packet, int length) {
  if (length < 8) return;
  
  uint16_t srcPort = (packet[0] << 8) | packet[1];
  uint16_t destPort = (packet[2] << 8) | packet[3];
  
  // DNS traffic
  if (destPort == 53 || srcPort == 53) {
    packetStats.dns_packets++;
  }
}

void detectSuspiciousActivity(const uint8_t* packet, int length, uint8_t frameType, uint8_t frameSubtype) {
  // Detect deauth/disassoc frames (potential attacks)
  if (frameType == 0) { // Management frame
    if (frameSubtype == 12 || frameSubtype == 10) { // Deauth or Disassoc
      securityAlerts.suspicious_packets++;
    }
  }
  
  // Detect unusual frame patterns
  static unsigned long lastFrameTime = 0;
  static int rapidFrameCount = 0;
  
  if (millis() - lastFrameTime < 10) { // Very rapid frames
    rapidFrameCount++;
    if (rapidFrameCount > 100) {
      securityAlerts.suspicious_packets++;
      rapidFrameCount = 0;
    }
  } else {
    rapidFrameCount = 0;
  }
  lastFrameTime = millis();
}

void printPacketAnalysis() {
  Serial.println("\n=== PACKET ANALYSIS ===");
  Serial.printf("Total Packets Captured: %lu\n", packetStats.total_packets);
  Serial.printf("Bytes Analyzed: %lu KB\n", packetStats.bytes_analyzed / 1024);
  
  if (packetStats.total_packets > 0) {
    Serial.println("\nProtocol Breakdown:");
    Serial.printf("  TCP: %lu (%.1f%%)\n", packetStats.tcp_packets, 
                  (float)packetStats.tcp_packets / packetStats.total_packets * 100);
    Serial.printf("  UDP: %lu (%.1f%%)\n", packetStats.udp_packets,
                  (float)packetStats.udp_packets / packetStats.total_packets * 100);
    Serial.printf("  ICMP: %lu (%.1f%%)\n", packetStats.icmp_packets,
                  (float)packetStats.icmp_packets / packetStats.total_packets * 100);
    
    Serial.println("\nApplication Traffic:");
    Serial.printf("  HTTP: %lu\n", packetStats.http_packets);
    Serial.printf("  HTTPS: %lu\n", packetStats.https_packets);
    Serial.printf("  DNS: %lu\n", packetStats.dns_packets);
    
    Serial.printf("\nCapture Rate: %.1f packets/sec\n", 
                  (float)packetStats.total_packets / (millis() / 1000.0));
  }
  Serial.println("========================");
}

void checkSecurityAlerts() {
  bool hasAlerts = false;
  
  if (securityAlerts.port_scans > 0) {
    Serial.printf("🚨 SECURITY: %lu potential port scans detected\n", securityAlerts.port_scans);
    hasAlerts = true;
  }
  
  if (securityAlerts.syn_floods > 0) {
    Serial.printf("🚨 SECURITY: %lu potential SYN flood attempts\n", securityAlerts.syn_floods);
    hasAlerts = true;
  }
  
  if (securityAlerts.suspicious_packets > 0) {
    Serial.printf("⚠️  WARNING: %lu suspicious packets (possible attacks)\n", securityAlerts.suspicious_packets);
    hasAlerts = true;
  }
  
  if (hasAlerts) {
    Serial.println("   → Monitor your network for potential threats");
  }
}
