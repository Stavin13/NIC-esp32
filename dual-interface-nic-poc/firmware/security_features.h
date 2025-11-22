/*
 * Security Features for Dual Interface NIC
 * 
 * Features:
 * - MAC Address Filtering
 * - Packet Inspection (Basic Firewall)
 * - VPN Support
 * - Secure Boot Configuration
 */

#ifndef SECURITY_FEATURES_H
#define SECURITY_FEATURES_H

#include <Arduino.h>

// ============================================
// 1. MAC ADDRESS FILTERING
// ============================================

#define MAX_ALLOWED_MACS 10
#define MAX_BLOCKED_MACS 10

class MACFilter {
private:
    uint8_t allowedMACs[MAX_ALLOWED_MACS][6];
    uint8_t blockedMACs[MAX_BLOCKED_MACS][6];
    int allowedCount = 0;
    int blockedCount = 0;
    bool whitelistMode = false; // false = blacklist, true = whitelist
    
public:
    // Add MAC to allowed list
    bool addAllowedMAC(const uint8_t* mac) {
        if (allowedCount >= MAX_ALLOWED_MACS) return false;
        memcpy(allowedMACs[allowedCount], mac, 6);
        allowedCount++;
        return true;
    }
    
    // Add MAC to blocked list
    bool addBlockedMAC(const uint8_t* mac) {
        if (blockedCount >= MAX_BLOCKED_MACS) return false;
        memcpy(blockedMACs[blockedCount], mac, 6);
        blockedCount++;
        return true;
    }
    
    // Check if MAC is allowed
    bool isAllowed(const uint8_t* mac) {
        // Check if blocked
        for (int i = 0; i < blockedCount; i++) {
            if (memcmp(blockedMACs[i], mac, 6) == 0) {
                Serial.println("MAC blocked!");
                return false;
            }
        }
        
        // If whitelist mode, check if in allowed list
        if (whitelistMode) {
            for (int i = 0; i < allowedCount; i++) {
                if (memcmp(allowedMACs[i], mac, 6) == 0) {
                    return true;
                }
            }
            Serial.println("MAC not in whitelist!");
            return false;
        }
        
        return true; // Allow by default in blacklist mode
    }
    
    void setWhitelistMode(bool enabled) {
        whitelistMode = enabled;
    }
    
    void printMAC(const uint8_t* mac) {
        for (int i = 0; i < 6; i++) {
            Serial.printf("%02X", mac[i]);
            if (i < 5) Serial.print(":");
        }
        Serial.println();
    }
};

// ============================================
// 2. PACKET INSPECTION (BASIC FIREWALL)
// ============================================

// Ethernet frame structure
struct EthernetFrame {
    uint8_t destMAC[6];
    uint8_t srcMAC[6];
    uint16_t etherType;
    uint8_t payload[1500];
};

// IP packet structure (simplified)
struct IPPacket {
    uint8_t versionIHL;
    uint8_t tos;
    uint16_t totalLength;
    uint16_t identification;
    uint16_t flagsFragment;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t srcIP;
    uint32_t destIP;
};

class PacketInspector {
private:
    bool blockICMP = false;
    bool blockUDP = false;
    bool blockTCP = false;
    uint16_t blockedPorts[20];
    int blockedPortCount = 0;
    
public:
    // Protocol numbers
    enum Protocol {
        ICMP = 1,
        TCP = 6,
        UDP = 17
    };
    
    // Inspect packet and decide if it should be forwarded
    bool inspectPacket(const uint8_t* packet, int length) {
        if (length < 14) return false; // Too short for Ethernet frame
        
        EthernetFrame* frame = (EthernetFrame*)packet;
        uint16_t etherType = ntohs(frame->etherType);
        
        // Check if it's an IP packet (0x0800)
        if (etherType == 0x0800) {
            if (length < 34) return false; // Too short for IP
            
            IPPacket* ip = (IPPacket*)(packet + 14);
            
            // Check protocol blocking
            if (blockICMP && ip->protocol == ICMP) {
                Serial.println("Blocked ICMP packet");
                return false;
            }
            if (blockUDP && ip->protocol == UDP) {
                Serial.println("Blocked UDP packet");
                return false;
            }
            if (blockTCP && ip->protocol == TCP) {
                Serial.println("Blocked TCP packet");
                return false;
            }
            
            // Check port blocking (for TCP/UDP)
            if (ip->protocol == TCP || ip->protocol == UDP) {
                if (length >= 38) {
                    uint16_t destPort = ntohs(*(uint16_t*)(packet + 36));
                    if (isPortBlocked(destPort)) {
                        Serial.printf("Blocked port %d\n", destPort);
                        return false;
                    }
                }
            }
        }
        
        return true; // Allow packet
    }
    
    void blockProtocol(Protocol proto, bool block) {
        switch (proto) {
            case ICMP: blockICMP = block; break;
            case TCP: blockTCP = block; break;
            case UDP: blockUDP = block; break;
        }
    }
    
    bool addBlockedPort(uint16_t port) {
        if (blockedPortCount >= 20) return false;
        blockedPorts[blockedPortCount++] = port;
        return true;
    }
    
    bool isPortBlocked(uint16_t port) {
        for (int i = 0; i < blockedPortCount; i++) {
            if (blockedPorts[i] == port) return true;
        }
        return false;
    }
};

// ============================================
// 3. VPN SUPPORT (BASIC ENCRYPTION)
// ============================================

#include "mbedtls/aes.h"

class VPNEncryption {
private:
    mbedtls_aes_context aes_ctx;
    uint8_t key[32]; // 256-bit key
    bool enabled = false;
    
public:
    VPNEncryption() {
        mbedtls_aes_init(&aes_ctx);
    }
    
    ~VPNEncryption() {
        mbedtls_aes_free(&aes_ctx);
    }
    
    // Set encryption key
    bool setKey(const uint8_t* newKey, size_t keyLen) {
        if (keyLen != 32) return false; // Only 256-bit keys
        memcpy(key, newKey, 32);
        
        int ret = mbedtls_aes_setkey_enc(&aes_ctx, key, 256);
        if (ret != 0) {
            Serial.printf("AES key setup failed: %d\n", ret);
            return false;
        }
        
        enabled = true;
        Serial.println("VPN encryption enabled");
        return true;
    }
    
    // Encrypt packet (simplified - real VPN is more complex)
    bool encryptPacket(uint8_t* packet, int length) {
        if (!enabled) return true;
        
        // Pad to 16-byte blocks
        int paddedLen = ((length + 15) / 16) * 16;
        
        // Encrypt in 16-byte blocks
        for (int i = 0; i < paddedLen; i += 16) {
            mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT, 
                                  packet + i, packet + i);
        }
        
        return true;
    }
    
    // Decrypt packet
    bool decryptPacket(uint8_t* packet, int length) {
        if (!enabled) return true;
        
        // Decrypt in 16-byte blocks
        for (int i = 0; i < length; i += 16) {
            mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT,
                                  packet + i, packet + i);
        }
        
        return true;
    }
    
    void setEnabled(bool enable) {
        enabled = enable;
    }
    
    bool isEnabled() {
        return enabled;
    }
};

// ============================================
// 4. SECURE BOOT CONFIGURATION
// ============================================

class SecureBoot {
public:
    // Check if secure boot is enabled
    static bool isEnabled() {
        #ifdef CONFIG_SECURE_BOOT
        return true;
        #else
        return false;
        #endif
    }
    
    // Enable secure boot (requires reflashing with secure bootloader)
    static void printStatus() {
        Serial.println("=== Secure Boot Status ===");
        
        #ifdef CONFIG_SECURE_BOOT
        Serial.println("Secure Boot: ENABLED");
        Serial.println("Firmware signature verification: ACTIVE");
        #else
        Serial.println("Secure Boot: DISABLED");
        Serial.println("To enable:");
        Serial.println("1. Use ESP-IDF (not Arduino IDE)");
        Serial.println("2. Run: idf.py menuconfig");
        Serial.println("3. Enable Security features → Secure Boot");
        Serial.println("4. Generate signing key");
        Serial.println("5. Flash bootloader and app");
        #endif
        
        Serial.println("========================");
    }
    
    // Verify firmware integrity (simplified)
    static bool verifyFirmware() {
        // In production, this would verify cryptographic signature
        Serial.println("Verifying firmware integrity...");
        
        #ifdef CONFIG_SECURE_BOOT
        // Secure boot handles this automatically
        Serial.println("Firmware verified by secure boot");
        return true;
        #else
        Serial.println("Warning: Secure boot not enabled!");
        Serial.println("Firmware integrity cannot be verified");
        return false;
        #endif
    }
};

#endif // SECURITY_FEATURES_H
