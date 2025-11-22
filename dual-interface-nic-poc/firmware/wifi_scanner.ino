/*
 * Wi-Fi Scanner - Debug Tool
 * Scans for available Wi-Fi networks
 * Helps diagnose connection issues
 */

#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n=== Wi-Fi Scanner ===\n");
  
  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  Serial.println("Scanning for Wi-Fi networks...\n");
}

void loop() {
  // Scan for networks
  int n = WiFi.scanNetworks();
  
  Serial.println("Scan complete!");
  Serial.print("Found ");
  Serial.print(n);
  Serial.println(" networks:\n");
  
  if (n == 0) {
    Serial.println("No networks found");
  } else {
    Serial.println("Nr | SSID                             | RSSI | Channel | Encryption");
    Serial.println("---|----------------------------------|------|---------|------------");
    
    for (int i = 0; i < n; i++) {
      // Print network details
      Serial.printf("%2d", i + 1);
      Serial.print(" | ");
      Serial.printf("%-32s", WiFi.SSID(i).c_str());
      Serial.print(" | ");
      Serial.printf("%4d", WiFi.RSSI(i));
      Serial.print(" | ");
      Serial.printf("%7d", WiFi.channel(i));
      Serial.print(" | ");
      
      // Encryption type
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
        case WIFI_AUTH_WPA2_ENTERPRISE:
          Serial.print("WPA2-Enterprise");
          break;
        default:
          Serial.print("Unknown");
      }
      Serial.println();
    }
  }
  
  Serial.println("\n--- Looking for 'vivo Y100' ---");
  bool found = false;
  for (int i = 0; i < n; i++) {
    if (WiFi.SSID(i) == "vivo Y100") {
      found = true;
      Serial.println("✓ Found 'vivo Y100'!");
      Serial.print("  Signal: ");
      Serial.print(WiFi.RSSI(i));
      Serial.println(" dBm");
      Serial.print("  Channel: ");
      Serial.println(WiFi.channel(i));
      Serial.print("  Encryption: ");
      Serial.println(WiFi.encryptionType(i));
      
      // Check if 2.4GHz (channels 1-14) or 5GHz (channels 36+)
      if (WiFi.channel(i) <= 14) {
        Serial.println("  ✓ 2.4GHz - Compatible!");
      } else {
        Serial.println("  ✗ 5GHz - NOT COMPATIBLE!");
        Serial.println("  Change your phone hotspot to 2.4GHz");
      }
    }
  }
  
  if (!found) {
    Serial.println("✗ 'vivo Y100' not found!");
    Serial.println("  Possible reasons:");
    Serial.println("  - Hotspot is off");
    Serial.println("  - Out of range");
    Serial.println("  - SSID name is different");
  }
  
  Serial.println("\n\nScanning again in 10 seconds...\n");
  delay(10000);
}
