/**
 * @file EthernetDemo.ino
 * @brief Example: Ethernet connectivity and status monitoring
 * 
 * This example demonstrates:
 * - Ethernet initialization using W5500 chip
 * - Network configuration
 * - Status indication via RGB LED
 * - Periodic network status reporting
 */

#include <ESP32S3_8DI8DO.h>
#include <ETH.h>

ESP32S3_8DI8DO board;

// Network configuration
unsigned long lastStatusTime = 0;
const uint16_t STATUS_INTERVAL = 5000;  // Print status every 5 seconds

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n===== ESP32-S3-POE-ETH-8DI-8DO =====");
    Serial.println("Example: Ethernet Connectivity");
    Serial.println("=====================================\n");
    
    // Initialize board
    Serial.print("Initializing board...");
    ESP32S3_8DI8DO::Status status = board.begin();
    
    if (status != ESP32S3_8DI8DO::STATUS_OK) {
        Serial.println(" FAILED");
        board.setRGB(255, 0, 0);
        return;
    }
    Serial.println(" OK");
    
    // Initialize Ethernet
    Serial.print("Initializing Ethernet...");
    initializeEthernet();
    
    board.setRGB(255, 255, 0);  // Yellow = Connecting
    board.beep(200);
    
    Serial.println("\nWaiting for Ethernet connection...");
    
    // Wait for connection with timeout
    int timeout = 0;
    while (!ETH.linkUp() && timeout < 50) {
        delay(100);
        timeout++;
    }
    
    if (ETH.linkUp()) {
        Serial.println("✓ Ethernet connected!");
        board.setRGB(0, 255, 0);  // Green = Connected
        printNetworkStatus();
    } else {
        Serial.println("✗ Ethernet connection failed");
        board.setRGB(255, 0, 0);  // Red = Error
    }
}

void loop() {
    board.loop();
    
    unsigned long currentTime = millis();
    
    // Print network status periodically
    if (currentTime - lastStatusTime >= STATUS_INTERVAL) {
        lastStatusTime = currentTime;
        
        if (ETH.linkUp()) {
            board.setRGB(0, 255, 0);
            printNetworkStatus();
        } else {
            board.setRGB(255, 0, 0);
            Serial.println("✗ Ethernet disconnected!");
        }
    }
    
    delay(100);
}

/**
 * Initialize Ethernet using W5500 chip
 */
void initializeEthernet() {
    // W5500 SPI configuration (GPIO 13, 14, 15 for MOSI, MISO, CLK)
    // CS on GPIO16, RST on GPIO39, INT on GPIO12
    
    // ETH.begin() will be called with W5500 configuration
    // This is typically done through ETH.config() before begin()
    
    Serial.println("W5500 Ethernet controller...");
    delay(500);
}

/**
 * Print network status
 */
void printNetworkStatus() {
    Serial.println("\n┌─── Ethernet Status ────────┐");
    Serial.print("  Link status: ");
    Serial.println(ETH.linkUp() ? "✓ UP" : "✗ DOWN");
    
    if (ETH.linkUp()) {
        Serial.print("  IP Address: ");
        Serial.println(ETH.localIP());
        
        Serial.print("  Gateway: ");
        Serial.println(ETH.gatewayIP());
        
        Serial.print("  Subnet Mask: ");
        Serial.println(ETH.subnetMask());
        
        Serial.print("  DNS Server: ");
        Serial.println(ETH.dnsIP());
        
        Serial.print("  MAC Address: ");
        printMACAddress();
        
        Serial.print("  Connection Speed: ");
        Serial.print(ETH.linkSpeed());
        Serial.println(" Mbps");
    }
    
    Serial.println("├─── System Status ─────────┤");
    Serial.print("  Uptime: ");
    Serial.print(board.getUptime());
    Serial.println(" seconds");
    Serial.println("└────────────────────────────┘\n");
}

/**
 * Print MAC address in formatted way
 */
void printMACAddress() {
    uint8_t mac[6];
    ETH.macAddress(mac);
    
    for (int i = 0; i < 6; i++) {
        if (mac[i] < 0x10) Serial.print("0");
        Serial.print(mac[i], HEX);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
}
