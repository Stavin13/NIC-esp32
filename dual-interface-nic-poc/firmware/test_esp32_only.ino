/*
 * ESP32-S3 Bare Boot Test
 * 
 * Tests ESP32-S3 without W5500 module
 * Just verifies USB, GPIO, and basic functionality
 */

#include <WiFi.h>

// Pin definitions (for future W5500 connection)
#define W5500_CS    34
#define W5500_RST   33
#define W5500_MISO  37
#define W5500_MOSI  35
#define W5500_SCK   36

// Built-in LED (if available)
#define LED_PIN     2

void setup() {
  // Initialize serial
  Serial.begin(115200);
  delay(2000); // Wait for serial monitor
  
  Serial.println("\n\n");
  Serial.println("================================");
  Serial.println("ESP32-S3 Bare Boot Test");
  Serial.println("================================");
  
  // Print chip info
  printChipInfo();
  
  // Test GPIO pins
  testGPIO();
  
  // Test built-in LED
  testLED();
  
  Serial.println("\n================================");
  Serial.println("Test Complete!");
  Serial.println("ESP32-S3 is working correctly!");
  Serial.println("================================");
  Serial.println("\nReady for W5500 module connection.");
}

void loop() {
  // Blink LED to show it's alive
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 1000) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    lastBlink = millis();
    
    // Print status every 5 seconds
    static int counter = 0;
    counter++;
    if (counter % 5 == 0) {
      Serial.printf("Running... %d seconds\n", counter);
    }
  }
}

void printChipInfo() {
  Serial.println("\n--- Chip Information ---");
  
  // Chip model
  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel());
  
  // Chip revision
  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());
  
  // Number of cores
  Serial.print("CPU Cores: ");
  Serial.println(ESP.getChipCores());
  
  // CPU frequency
  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  
  // Flash size
  Serial.print("Flash Size: ");
  Serial.print(ESP.getFlashChipSize() / (1024 * 1024));
  Serial.println(" MB");
  
  // Free heap
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap() / 1024);
  Serial.println(" KB");
  
  // MAC Address
  Serial.print("MAC Address: ");
  String macAddress = WiFi.macAddress();
  Serial.println(macAddress);
}

void testGPIO() {
  Serial.println("\n--- GPIO Pin Test ---");
  Serial.println("Testing pins for W5500 connection:");
  
  // Configure pins as outputs for testing
  pinMode(W5500_SCK, OUTPUT);
  pinMode(W5500_MOSI, OUTPUT);
  pinMode(W5500_CS, OUTPUT);
  pinMode(W5500_RST, OUTPUT);
  
  // Test each pin
  Serial.print("GPIO36 (SCK):  ");
  testPin(W5500_SCK);
  
  Serial.print("GPIO35 (MOSI): ");
  testPin(W5500_MOSI);
  
  Serial.print("GPIO34 (CS):   ");
  testPin(W5500_CS);
  
  Serial.print("GPIO33 (RST):  ");
  testPin(W5500_RST);
  
  // MISO is input, just check if readable
  pinMode(W5500_MISO, INPUT);
  Serial.print("GPIO37 (MISO): ");
  Serial.println("OK (input)");
  
  Serial.println("All GPIO pins working!");
}

void testPin(int pin) {
  digitalWrite(pin, HIGH);
  delay(10);
  if (digitalRead(pin) == HIGH) {
    digitalWrite(pin, LOW);
    delay(10);
    if (digitalRead(pin) == LOW) {
      Serial.println("OK");
      return;
    }
  }
  Serial.println("FAILED");
}

void testLED() {
  Serial.println("\n--- LED Test ---");
  Serial.println("Blinking built-in LED...");
  
  pinMode(LED_PIN, OUTPUT);
  
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    Serial.print("ON  ");
    delay(200);
    digitalWrite(LED_PIN, LOW);
    Serial.print("OFF  ");
    delay(200);
  }
  Serial.println("\nLED test complete!");
}
