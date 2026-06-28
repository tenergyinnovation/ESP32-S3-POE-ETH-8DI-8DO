/*
 * EthernetDemo Example
 * 
 * Demonstrates Gigabit Ethernet connectivity on the ESP32-S3-POE-ETH-8DI-8DO board
 * 
 * This example shows how to:
 * - Initialize Ethernet interface
 * - Connect to Ethernet network
 * - Perform basic network communication
 * - Use PoE for power supply
 * 
 * Hardware Setup:
 * - Connect Ethernet cable to RJ45 jack
 * - Board can be powered via PoE or 5V DC
 */

#include <ESP32S3_8DI8DO.h>
#include <WiFi.h>
#include <ETH.h>

// Create board instance
ESP32S3_8DI8DO board;

// Ethernet event callback
static bool eth_connected = false;

void onEthEvent(arduino_event_id_t event) {
    switch (event) {
        case ARDUINO_EVENT_ETH_START:
            Serial.println("🌐 ETH Started");
            board.setRGBColor(255, 255, 0);  // Yellow
            break;
            
        case ARDUINO_EVENT_ETH_CONNECTED:
            Serial.println("🌐 ETH Connected");
            board.setRGBColor(255, 0, 255);  // Magenta
            break;
            
        case ARDUINO_EVENT_ETH_GOT_IP:
            Serial.print("🌐 ETH IP: ");
            Serial.println(ETH.localIP());
            Serial.print("    Gateway: ");
            Serial.println(ETH.gatewayIP());
            Serial.print("    DNS: ");
            Serial.println(ETH.dnsIP());
            eth_connected = true;
            board.setRGBColor(0, 255, 0);  // Green = connected
            board.buzzerBeep(3, 100, 100);
            break;
            
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            Serial.println("🌐 ETH Disconnected");
            eth_connected = false;
            board.setRGBColor(255, 0, 0);  // Red
            break;
            
        case ARDUINO_EVENT_ETH_STOP:
            Serial.println("🌐 ETH Stopped");
            eth_connected = false;
            board.LEDOff();
            break;
            
        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n╔════════════════════════════════════════╗");
    Serial.println("║   ESP32-S3-POE-ETH-8DI-8DO            ║");
    Serial.println("║   Ethernet Demo                       ║");
    Serial.println("╚════════════════════════════════════════╝\n");
    
    // Initialize board
    if (!board.begin()) {
        Serial.println("❌ Board initialization failed!");
        while (1) delay(1000);
    }
    
    Serial.println(board.getBoardInfo());
    
    // Setup event listener
    Network.onEvent(onEthEvent);
    
    // Start Ethernet
    Serial.println("\n🌐 Initializing Ethernet...");
    board.setRGBColor(255, 255, 0);  // Yellow = initializing
    
    // Initialize Ethernet
    ETH.begin();
    
    Serial.println("⏳ Waiting for Ethernet connection...");
}

void loop() {
    // Network operations
    if (eth_connected) {
        static uint32_t lastPing = 0;
        
        // Ping gateway every 10 seconds
        if (millis() - lastPing > 10000) {
            lastPing = millis();
            
            Serial.println("\n📊 Network Status:");
            Serial.print("  IP Address: ");
            Serial.println(ETH.localIP());
            Serial.print("  Gateway: ");
            Serial.println(ETH.gatewayIP());
            Serial.print("  DNS: ");
            Serial.println(ETH.dnsIP());
            
            // Beep feedback
            board.buzzerBeep(1, 50, 50);
        }
    } else {
        // Show connection status via LED
        static uint32_t lastBlink = 0;
        if (millis() - lastBlink > 500) {
            lastBlink = millis();
            board.setRGBColor(255, 0, 0);  // Red
        } else if (millis() - lastBlink > 250) {
            board.LEDOff();
        }
    }
    
    delay(100);
}

/*
 * TODO: Add TCP/UDP communication examples:
 * 
 * 1. TCP Client:
 *    - Connect to remote server
 *    - Send/receive data
 * 
 * 2. TCP Server:
 *    - Listen for incoming connections
 *    - Handle multiple clients
 * 
 * 3. UDP Communication:
 *    - Send/receive UDP packets
 *    - Multicast support
 * 
 * 4. MQTT over Ethernet:
 *    - Connect to MQTT broker
 *    - Publish/subscribe to topics
 */
