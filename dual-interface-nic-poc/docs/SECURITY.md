 # Security Features Guide

## Overview

Your Dual Interface NIC now includes four security features:

1. **MAC Address Filtering** - Control which devices can communicate
2. **Packet Inspection (Firewall)** - Block unwanted protocols and ports
3. **VPN Support** - Encrypt all traffic passing through the NIC
4. **Secure Boot** - Verify firmware integrity

---

## 1. MAC Address Filtering

### What It Does
Controls which devices (by MAC address) can send/receive packets through your NIC.

### Two Modes:

**Blacklist Mode (Default):**
- Allows all MACs except those explicitly blocked
- Good for blocking specific malicious devices

**Whitelist Mode:**
- Blocks all MACs except those explicitly allowed
- More secure, but requires maintaining allowed list

### Configuration

**Block a specific MAC address:**
```cpp
uint8_t blockedMAC[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
macFilter.addBlockedMAC(blockedMAC);
```

**Enable whitelist mode:**
```cpp
macFilter.setWhitelistMode(true);
uint8_t allowedMAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
macFilter.addAllowedMAC(allowedMAC);
```

**Disable MAC filtering:**
```cpp
enableMACFiltering = false; // In setup()
```

### Use Cases
- Block specific devices from network access
- Create isolated network segments
- Prevent MAC spoofing attacks
- Implement device whitelisting

---

## 2. Packet Inspection (Basic Firewall)

### What It Does
Inspects packets and blocks based on:
- Protocol type (ICMP, TCP, UDP)
- Port numbers
- Packet content

### Configuration

**Block ICMP (ping):**
```cpp
packetInspector.blockProtocol(PacketInspector::ICMP, true);
```

**Block specific ports:**
```cpp
packetInspector.addBlockedPort(23);   // Telnet
packetInspector.addBlockedPort(135);  // Windows RPC
packetInspector.addBlockedPort(445);  // SMB
packetInspector.addBlockedPort(3389); // RDP
```

**Block all TCP:**
```cpp
packetInspector.blockProtocol(PacketInspector::TCP, true);
```

**Disable firewall:**
```cpp
enableFirewall = false; // In setup()
```

### Common Port Blocks

**Security:**
- Port 23 (Telnet) - Unencrypted remote access
- Port 135 (RPC) - Windows vulnerability
- Port 445 (SMB) - Ransomware vector
- Port 3389 (RDP) - Brute force target

**P2P/Torrents:**
- Port 6881-6889 (BitTorrent)
- Port 6346 (Gnutella)

**Gaming:**
- Port 27015 (Steam)
- Port 3074 (Xbox Live)

### Use Cases
- Block malware communication
- Prevent unauthorized services
- Implement parental controls
- Reduce attack surface

---

## 3. VPN Support (Encryption)

### What It Does
Encrypts all traffic passing through the NIC using AES-256 encryption.

### ⚠️ Important Notes
- **Performance Impact:** Encryption is CPU-intensive
- **Both Ends:** VPN must be configured on both sides
- **Key Management:** Secure key storage is critical
- **Not Full VPN:** This is packet-level encryption, not a complete VPN solution

### Configuration

**Enable VPN:**
```cpp
enableVPN = true; // In setup()
```

**Set encryption key:**
```cpp
uint8_t vpnKey[32] = {
  // Your 256-bit key here
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};
vpnEncryption.setKey(vpnKey, 32);
```

**Generate secure key (Python):**
```python
import os
key = os.urandom(32)
print(','.join(f'0x{b:02X}' for b in key))
```

### Performance Impact
- **Without VPN:** ~80-90 Mbps throughput
- **With VPN:** ~20-40 Mbps throughput
- **CPU Usage:** Increases from 10% to 40-60%

### Use Cases
- Secure public Wi-Fi connections
- Protect sensitive data in transit
- Bypass network monitoring
- Create encrypted tunnels

---

## 4. Secure Boot

### What It Does
Verifies firmware integrity before execution, preventing:
- Malware injection
- Unauthorized firmware modifications
- Bootloader attacks

### Current Status
Secure boot requires ESP-IDF (not Arduino IDE) and special configuration.

**Check status:**
```cpp
SecureBoot::printStatus();
SecureBoot::verifyFirmware();
```

### Enabling Secure Boot

**Requirements:**
- ESP-IDF framework (not Arduino)
- Signing key generation
- One-time fuse burning (irreversible!)

**Steps:**

1. **Install ESP-IDF:**
```bash
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
. ./export.sh
```

2. **Enable in menuconfig:**
```bash
idf.py menuconfig
# Navigate to: Security features → Secure Boot V2
# Enable: "Enable hardware Secure Boot in bootloader"
```

3. **Generate signing key:**
```bash
espsecure.py generate_signing_key secure_boot_signing_key.pem
```

4. **Build and flash:**
```bash
idf.py build
idf.py flash
```

5. **Burn efuses (IRREVERSIBLE!):**
```bash
espefuse.py burn_key secure_boot_v2 secure_boot_signing_key.pem
espefuse.py burn_efuse SECURE_BOOT_EN
```

### ⚠️ Warning
- Secure boot is **permanent** once enabled
- Lost signing key = bricked device
- Cannot downgrade or disable
- Only enable on production devices

### Use Cases
- Production deployments
- High-security applications
- Prevent firmware tampering
- Compliance requirements

---

## Security Configuration Examples

### Home Network (Balanced)
```cpp
enableMACFiltering = false;  // Trust home devices
enableFirewall = true;       // Block dangerous ports
enableVPN = false;           // Not needed on home network
// Secure boot: Optional
```

### Public Network (High Security)
```cpp
enableMACFiltering = true;   // Whitelist mode
macFilter.setWhitelistMode(true);
enableFirewall = true;       // Block everything suspicious
enableVPN = true;            // Encrypt all traffic
// Secure boot: Recommended
```

### Corporate Network (Compliance)
```cpp
enableMACFiltering = true;   // Strict device control
enableFirewall = true;       // Policy enforcement
enableVPN = true;            // Data protection
// Secure boot: Required
```

### Development/Testing (Minimal)
```cpp
enableMACFiltering = false;  // Allow all devices
enableFirewall = false;      // Don't block anything
enableVPN = false;           // No encryption overhead
// Secure boot: Disabled
```

---

## Performance Comparison

| Configuration | Throughput | Latency | CPU Usage |
|---------------|------------|---------|-----------|
| No Security | 80-90 Mbps | 5-10ms | 10-15% |
| MAC Filter Only | 75-85 Mbps | 6-12ms | 15-20% |
| Firewall Only | 70-80 Mbps | 8-15ms | 20-25% |
| MAC + Firewall | 65-75 Mbps | 10-18ms | 25-30% |
| All Features | 20-40 Mbps | 20-40ms | 50-70% |

---

## Troubleshooting

### MAC Filtering Issues

**Problem:** Legitimate devices blocked
```cpp
// Check blocked MAC list
macFilter.printMAC(deviceMAC);
// Temporarily disable to test
enableMACFiltering = false;
```

**Problem:** Whitelist too restrictive
```cpp
// Switch to blacklist mode
macFilter.setWhitelistMode(false);
```

### Firewall Issues

**Problem:** Can't access needed services
```cpp
// Check if port is blocked
if (packetInspector.isPortBlocked(port)) {
  // Remove from blocked list or disable firewall
}
```

**Problem:** Too many false positives
```cpp
// Disable specific protocol blocking
packetInspector.blockProtocol(PacketInspector::ICMP, false);
```

### VPN Issues

**Problem:** Very slow performance
```cpp
// Disable VPN temporarily
enableVPN = false;
// Or use hardware crypto acceleration (ESP32-S3 feature)
```

**Problem:** Connection fails
```cpp
// Verify both ends have same key
// Check key length (must be 32 bytes)
// Ensure VPN enabled on both sides
```

### Secure Boot Issues

**Problem:** Firmware won't boot
- Verify signing key matches
- Check efuse settings
- Reflash with signed firmware

**Problem:** Can't update firmware
- Sign new firmware with same key
- Use OTA update with signature verification

---

## Best Practices

1. **Start Simple:** Enable features one at a time
2. **Test Thoroughly:** Verify each feature works before adding more
3. **Monitor Performance:** Check CPU and throughput impact
4. **Secure Keys:** Never hardcode production keys in firmware
5. **Log Everything:** Enable serial logging for debugging
6. **Update Regularly:** Keep firmware and security rules current
7. **Backup Keys:** Store signing keys securely offline
8. **Document Changes:** Track security configuration changes

---

## Security Limitations

**What This Provides:**
- Basic packet filtering
- Simple encryption
- Device access control
- Firmware verification

**What This Doesn't Provide:**
- Deep packet inspection (DPI)
- Intrusion detection system (IDS)
- Full VPN protocol (OpenVPN, WireGuard)
- Advanced threat protection
- Network monitoring/logging

**For Production:**
Consider using dedicated security appliances or professional VPN solutions for critical applications.

---

## Further Reading

- [ESP32-S3 Security Features](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/security/index.html)
- [Secure Boot Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/security/secure-boot-v2.html)
- [mbedTLS Documentation](https://tls.mbed.org/)
- [Network Security Best Practices](https://www.nist.gov/cybersecurity)
