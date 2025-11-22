/*
 * USB Device Descriptors for CDC-ECM Network Interface
 * 
 * These descriptors make the device appear as a standard network adapter
 * Compatible with x86, ARM, and other architectures
 */

#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

// USB Vendor and Product IDs
// NOTE: These are example IDs. For production, get your own from USB-IF
#define USB_VID  0x1234  // Vendor ID
#define USB_PID  0x5678  // Product ID

// Device Descriptor
const uint8_t device_descriptor[] = {
    0x12,        // bLength
    0x01,        // bDescriptorType (Device)
    0x00, 0x02,  // bcdUSB 2.00
    0x02,        // bDeviceClass (CDC)
    0x00,        // bDeviceSubClass
    0x00,        // bDeviceProtocol
    0x40,        // bMaxPacketSize0 64
    (USB_VID & 0xFF), (USB_VID >> 8),  // idVendor
    (USB_PID & 0xFF), (USB_PID >> 8),  // idProduct
    0x00, 0x01,  // bcdDevice 1.00
    0x01,        // iManufacturer (String Index)
    0x02,        // iProduct (String Index)
    0x03,        // iSerialNumber (String Index)
    0x01         // bNumConfigurations
};

// Configuration Descriptor (CDC-ECM)
const uint8_t config_descriptor[] = {
    // Configuration Descriptor
    0x09,        // bLength
    0x02,        // bDescriptorType (Configuration)
    0x4B, 0x00,  // wTotalLength 75 bytes
    0x02,        // bNumInterfaces 2
    0x01,        // bConfigurationValue
    0x00,        // iConfiguration (String Index)
    0x80,        // bmAttributes (Bus Powered)
    0xFA,        // bMaxPower 500mA

    // Interface Association Descriptor
    0x08,        // bLength
    0x0B,        // bDescriptorType (IAD)
    0x00,        // bFirstInterface
    0x02,        // bInterfaceCount
    0x02,        // bFunctionClass (CDC)
    0x06,        // bFunctionSubClass (ECM)
    0x00,        // bFunctionProtocol
    0x00,        // iFunction

    // Interface Descriptor (Communication)
    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x00,        // bInterfaceNumber 0
    0x00,        // bAlternateSetting
    0x01,        // bNumEndpoints 1
    0x02,        // bInterfaceClass (CDC)
    0x06,        // bInterfaceSubClass (ECM)
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    // CDC Header Functional Descriptor
    0x05,        // bFunctionLength
    0x24,        // bDescriptorType (CS_INTERFACE)
    0x00,        // bDescriptorSubtype (Header)
    0x10, 0x01,  // bcdCDC 1.10

    // CDC Union Functional Descriptor
    0x05,        // bFunctionLength
    0x24,        // bDescriptorType (CS_INTERFACE)
    0x06,        // bDescriptorSubtype (Union)
    0x00,        // bControlInterface
    0x01,        // bSubordinateInterface0

    // CDC Ethernet Networking Functional Descriptor
    0x0D,        // bFunctionLength
    0x24,        // bDescriptorType (CS_INTERFACE)
    0x0F,        // bDescriptorSubtype (Ethernet Networking)
    0x04,        // iMACAddress (String Index)
    0x00, 0x00, 0x00, 0x00,  // bmEthernetStatistics
    0xEA, 0x05,  // wMaxSegmentSize 1514
    0x00, 0x00,  // wNumberMCFilters
    0x00,        // bNumberPowerFilters

    // Endpoint Descriptor (Interrupt IN)
    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x81,        // bEndpointAddress (IN, Endpoint 1)
    0x03,        // bmAttributes (Interrupt)
    0x08, 0x00,  // wMaxPacketSize 8
    0x10,        // bInterval 16ms

    // Interface Descriptor (Data)
    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x01,        // bInterfaceNumber 1
    0x00,        // bAlternateSetting
    0x02,        // bNumEndpoints 2
    0x0A,        // bInterfaceClass (CDC Data)
    0x00,        // bInterfaceSubClass
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    // Endpoint Descriptor (Bulk OUT)
    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x02,        // bEndpointAddress (OUT, Endpoint 2)
    0x02,        // bmAttributes (Bulk)
    0x40, 0x00,  // wMaxPacketSize 64
    0x00,        // bInterval

    // Endpoint Descriptor (Bulk IN)
    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x82,        // bEndpointAddress (IN, Endpoint 2)
    0x02,        // bmAttributes (Bulk)
    0x40, 0x00,  // wMaxPacketSize 64
    0x00         // bInterval
};

// String Descriptors
const char* string_descriptors[] = {
    "DIY Hardware",              // Manufacturer
    "Dual Interface NIC",        // Product
    "001",                       // Serial Number
    "DEADBEEFFEED"              // MAC Address (will be formatted)
};

// Helper function to format MAC address string
void formatMACAddress(char* buffer, const uint8_t* mac) {
    sprintf(buffer, "%02X%02X%02X%02X%02X%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

#endif // USB_DESCRIPTORS_H
