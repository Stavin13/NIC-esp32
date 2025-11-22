/*
 * ESP32-S3 Packet Analyzer
 * 
 * Legitimate network packet analysis tool
 * Analyzes packets passing through your NIC
 * Educational and debugging purposes
 * 
 * Features:
 * - Real-time packet inspection
 * - Protocol identification
 * - Traffic statistics
 * - Security anomaly detection
 */

#include <WiFi.h>
#include "esp_wifi.h"

// Configuration
const char* ssid = "vivo Y100";
const char* password = "123459876";

// Packet analysis structures
struct EthernetHeader {
  uint8_t dest_mac[6];
  uint8_t src_mac[6];
  uint16_t ether_type;
} __attribute__((packed));

struct IPHeader {
  uint8_t version_ihl;
  uint8_t tos;
  uint16_t total_length;
  uint16_t identification;
  uint16_t flags_fragment;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t checksum;
  uint32_t src_ip;
  uint32_t dest_ip;
} __attribute__((packed));

struct TCPHeader {
  uint16_t src_port;
  uint16_t dest_port;
  uint32_t seq_num;
  uint32_t ack_num;
  uint8_t data_offset;
  uint8_t flags;
  uint16_t window;
  uint16_t checksum;
  uint16_t urgent_ptr;
} __attribute__((packed));

struct UDPHeader {
  uint16_t src_port;
  uint16_t dest_port;
  uint16_t length;
  uint16_t checksum;
} __attribute__((packed));

// Statistics
struct PacketStats {
  unsigned long total_packets;
  unsigned long tcp_packets;
  unsigned long udp_packets;
  unsigned long icmp_packets;
  unsigned long http_packets;
  unsigned long https_packets;
  unsigned long dns_packets;
  unsigned long suspicious_packets;
  unsigned long bytes_analyzed;
} stats;

// Suspicious activity detection
struct SecurityAlerts {
  unsigned long port_scans;
  unsigned long syn_floods;
  unsigned long dns_tunneling;
  unsigned long unusual_protocols;
} alerts;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n================================");
  Serial.println("ESP32-S3 Packet Analyzer");
  Serial.println("Legitimate Network Analysis Tool");
  Serial.println("================================\n");
  
  // Initialize statistics
  memset(&stats, 0, sizeof(stats));
  memset(&alerts, 0, sizeof(alerts));
  
  // Connect to Wi-Fi
  connectToWiFi();
  
  // Enable promiscuous mode for packet capture
  enablePromiscuousMode();
  
  Serial.println("Packet analysis started!");
  Serial.println("Analyzing traffic on your network...\n");
  
  printAnalysisHeader();
}

void loop() {
  // Print statistics every 10 seconds
  static unsigned long lastStats = 0;
  if (millis() - lastStats > 10000) {
    printStatistics();
    lastStats = millis();
  }
  
  // Print security alerts
  static unsigned long lastAlerts = 0;
  if (millis() - lastAlerts > 5000) {
    checkSecurityAlerts();
    lastAlerts = millis();
  }
  
  delay(100);
}

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n✓ Wi-Fi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println();
}

void enablePromiscuousMode() {
  Serial.println("Enabling packet capture mode...");
  
  // Set promiscuous mode callback
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&packetHandler);
  
  Serial.println("✓ Packet capture enabled");
  Serial.println("  Capturing all packets on current channel");
  Serial.println("  Note: Only captures wireless packets, not wired");
  Serial.println();
}

void packetHandler(void* buf, wifi_promiscuous_pkt_type_t type) {
  // Only analyze data packets
  if (type != WIFI_PKT_DATA) return;
  
  wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  
  // Analyze the packet
  analyzePacket(pkt->payload, pkt->rx_ctrl.sig_len);
}

void analyzePacket(const uint8_t* packet, int length) {
  if (length < 14) return; // Too short for Ethernet header
  
  stats.total_packets++;
  stats.bytes_analyzed += length;
  
  // Parse Ethernet header (802.11 to Ethernet conversion)
  // Note: 802.11 headers are different, this is simplified
  
  // Look for IP packets (simplified detection)
  if (length > 34) {
    analyzeIPPacket(packet + 14, length - 14);
  }
  
  // Print packet summary (limit output to avoid spam)
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    printPacketSummary(packet, length);
    lastPrint = millis();
  }
}

void analyzeIPPacket(const uint8_t* packet, int length) {
  if (length < 20) return; // Too short for IP header
  
  IPHeader* ip = (IPHeader*)packet;
  
  // Check IP version
  if ((ip->version_ihl >> 4) != 4) return; // Only IPv4
  
  uint8_t protocol = ip->protocol;
  uint16_t ip_header_len = (ip->version_ihl & 0x0F) * 4;
  
  // Convert network byte order
  uint32_t src_ip = ntohl(ip->src_ip);
  uint32_t dest_ip = ntohl(ip->dest_ip);
  
  // Analyze by protocol
  switch (protocol) {
    case 1: // ICMP
      stats.icmp_packets++;
      analyzeICMP(packet + ip_header_len, length - ip_header_len);
      break;
      
    case 6: // TCP
      stats.tcp_packets++;
      analyzeTCP(packet + ip_header_len, length - ip_header_len, src_ip, dest_ip);
      break;
      
    case 17: // UDP
      stats.udp_packets++;
      analyzeUDP(packet + ip_header_len, length - ip_header_len, src_ip, dest_ip);
      break;
      
    default:
      // Unknown protocol - potentially suspicious
      alerts.unusual_protocols++;
      break;
  }
}

void analyzeTCP(const uint8_t* packet, int length, uint32_t src_ip, uint32_t dest_ip) {
  if (length < 20) return;
  
  TCPHeader* tcp = (TCPHeader*)packet;
  uint16_t src_port = ntohs(tcp->src_port);
  uint16_t dest_port = ntohs(tcp->dest_port);
  uint8_t flags = tcp->flags;
  
  // Identify common services
  if (dest_port == 80 || src_port == 80) {
    stats.http_packets++;
  } else if (dest_port == 443 || src_port == 443) {
    stats.https_packets++;
  }
  
  // Detect potential attacks
  if (flags == 0x02) { // SYN flag only
    static unsigned long syn_count = 0;
    static unsigned long last_syn_time = 0;
    
    syn_count++;
    if (millis() - last_syn_time > 1000) {
      if (syn_count > 100) { // More than 100 SYN packets per second
        alerts.syn_floods++;
      }
      syn_count = 0;
      last_syn_time = millis();
    }
  }
  
  // Detect port scanning
  static uint16_t last_dest_port = 0;
  static uint32_t last_dest_ip = 0;
  
  if (dest_ip == last_dest_ip && dest_port != last_dest_port) {
    static unsigned long port_scan_count = 0;
    port_scan_count++;
    
    if (port_scan_count > 10) { // Accessing many ports on same IP
      alerts.port_scans++;
      port_scan_count = 0;
    }
  }
  
  last_dest_port = dest_port;
  last_dest_ip = dest_ip;
}

void analyzeUDP(const uint8_t* packet, int length, uint32_t src_ip, uint32_t dest_ip) {
  if (length < 8) return;
  
  UDPHeader* udp = (UDPHeader*)packet;
  uint16_t src_port = ntohs(udp->src_port);
  uint16_t dest_port = ntohs(udp->dest_port);
  
  // DNS traffic
  if (dest_port == 53 || src_port == 53) {
    stats.dns_packets++;
    
    // Detect DNS tunneling (unusually large DNS packets)
    uint16_t udp_length = ntohs(udp->length);
    if (udp_length > 512) { // Normal DNS responses are usually < 512 bytes
      alerts.dns_tunneling++;
    }
  }
}

void analyzeICMP(const uint8_t* packet, int length) {
  // ICMP analysis (ping, etc.)
  // Could detect ping floods, unusual ICMP types, etc.
}

void printPacketSummary(const uint8_t* packet, int length) {
  Serial.print("Packet: ");
  Serial.print(length);
  Serial.print(" bytes | ");
  
  // Print first few bytes in hex
  for (int i = 0; i < min(8, length); i++) {
    Serial.printf("%02X ", packet[i]);
  }
  Serial.println();
}

void printAnalysisHeader() {
  Serial.println("Real-time Packet Analysis:");
  Serial.println("─────────────────────────────────────");
}

void printStatistics() {
  Serial.println("\n=== PACKET STATISTICS ===");
  Serial.printf("Total Packets: %lu\n", stats.total_packets);
  Serial.printf("Bytes Analyzed: %lu KB\n", stats.bytes_analyzed / 1024);
  Serial.println();
  
  Serial.println("Protocol Breakdown:");
  Serial.printf("  TCP: %lu (%.1f%%)\n", stats.tcp_packets, 
                (float)stats.tcp_packets / stats.total_packets * 100);
  Serial.printf("  UDP: %lu (%.1f%%)\n", stats.udp_packets,
                (float)stats.udp_packets / stats.total_packets * 100);
  Serial.printf("  ICMP: %lu (%.1f%%)\n", stats.icmp_packets,
                (float)stats.icmp_packets / stats.total_packets * 100);
  Serial.println();
  
  Serial.println("Application Traffic:");
  Serial.printf("  HTTP: %lu\n", stats.http_packets);
  Serial.printf("  HTTPS: %lu\n", stats.https_packets);
  Serial.printf("  DNS: %lu\n", stats.dns_packets);
  Serial.println();
  
  Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
  Serial.printf("Packets/sec: %.1f\n", (float)stats.total_packets / (millis() / 1000.0));
  Serial.println("═══════════════════════════");
}

void checkSecurityAlerts() {
  bool hasAlerts = false;
  
  if (alerts.port_scans > 0) {
    Serial.printf("🚨 SECURITY ALERT: %lu potential port scans detected\n", alerts.port_scans);
    hasAlerts = true;
  }
  
  if (alerts.syn_floods > 0) {
    Serial.printf("🚨 SECURITY ALERT: %lu potential SYN flood attacks\n", alerts.syn_floods);
    hasAlerts = true;
  }
  
  if (alerts.dns_tunneling > 0) {
    Serial.printf("🚨 SECURITY ALERT: %lu suspicious DNS queries (possible tunneling)\n", alerts.dns_tunneling);
    hasAlerts = true;
  }
  
  if (alerts.unusual_protocols > 0) {
    Serial.printf("ℹ️  INFO: %lu packets with unusual protocols\n", alerts.unusual_protocols);
    hasAlerts = true;
  }
  
  if (hasAlerts) {
    Serial.println("   → Review your network for suspicious activity");
    Serial.println();
  }
}

// Helper function to convert IP to string
String ipToString(uint32_t ip) {
  return String((ip >> 24) & 0xFF) + "." +
         String((ip >> 16) & 0xFF) + "." +
         String((ip >> 8) & 0xFF) + "." +
         String(ip & 0xFF);
}