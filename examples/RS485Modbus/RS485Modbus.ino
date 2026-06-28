/*
 * RS485Modbus Example
 * 
 * Demonstrates RS485 communication with Modbus protocol support
 * (Compatible with LoRa DTU modules and other RS485 devices)
 * 
 * This example shows how to:
 * - Initialize RS485 interface
 * - Send AT commands to LoRa DTU modules
 * - Receive and parse responses
 * - Handle communication timeouts
 * 
 * Hardware Setup:
 * - LoRa DTU RS485 A+ → ESP32 GPIO17 (TX)
 * - LoRa DTU RS485 B- → ESP32 GPIO18 (RX)
 * - LoRa DTU GND → ESP32 GND
 * - LoRa DTU 5V → ESP32 5V
 * 
 * Note: Add 120Ω termination resistor across RS485 A-B for long cables
 */

#include <ESP32S3_8DI8DO.h>

// Create board instance
ESP32S3_8DI8DO board;

// RS485 configuration
#define RS485_BAUD 9600
#define RS485_TIMEOUT 1000  // 1 second timeout
#define BUFFER_SIZE 256

// Response buffer
char rxBuffer[BUFFER_SIZE];
uint16_t rxIndex = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n╔════════════════════════════════════════╗");
    Serial.println("║   ESP32-S3-POE-ETH-8DI-8DO            ║");
    Serial.println("║   RS485 Modbus Demo                   ║");
    Serial.println("╚════════════════════════════════════════╝\n");
    
    // Initialize board
    if (!board.begin()) {
        Serial.println("❌ Board initialization failed!");
        while (1) delay(1000);
    }
    
    Serial.println(board.getBoardInfo());
    
    // Setup RS485
    Serial.println("\n📡 Initializing RS485...");
    if (board.setupRS485(RS485_BAUD)) {
        Serial.printf("✓ RS485 initialized at %d baud\n", RS485_BAUD);
        board.setRGBColor(0, 255, 0);  // Green
    } else {
        Serial.println("❌ RS485 initialization failed!");
        board.setRGBColor(255, 0, 0);  // Red
    }
    
    Serial.println("\n📝 Available Commands:");
    Serial.println("  AT           - Test connection");
    Serial.println("  AT+VER       - Get firmware version");
    Serial.println("  AT+MODE=1    - Set stream mode");
    Serial.println("  AT+SF=7      - Set spreading factor to 7");
    Serial.println("  AT+PWR=22    - Set RF power to 22dBm");
    Serial.println("  AT+EXIT      - Exit AT command mode\n");
    
    Serial.println("💬 Type AT commands in Serial Monitor and press Enter");
}

void loop() {
    // Check for user input from Serial Monitor
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.length() > 0) {
            Serial.printf("\n📤 Sending: %s\n", command.c_str());
            
            // Send command via RS485
            Serial2.println(command);
            
            // Wait for response
            board.setRGBColor(255, 255, 0);  // Yellow = waiting
            String response = readRS485Response(RS485_TIMEOUT);
            
            if (response.length() > 0) {
                board.setRGBColor(0, 255, 0);  // Green = success
                Serial.print("📥 Response: ");
                printResponse(response);
                board.buzzerBeep(1, 100, 50);
            } else {
                board.setRGBColor(255, 0, 0);  // Red = timeout
                Serial.println("⏱️  Response: TIMEOUT (No data received)");
                board.buzzerBeep(3, 50, 50);
            }
        }
    }
    
    delay(100);
}

/**
 * @brief Read response from RS485 device
 * @param timeout Timeout in milliseconds
 * @return Response string
 */
String readRS485Response(uint16_t timeout) {
    memset(rxBuffer, 0, BUFFER_SIZE);
    rxIndex = 0;
    uint32_t startTime = millis();
    
    while (millis() - startTime < timeout) {
        if (Serial2.available()) {
            char ch = Serial2.read();
            
            // Add to buffer
            if (rxIndex < BUFFER_SIZE - 1) {
                rxBuffer[rxIndex++] = ch;
            }
            
            // Check for end of response (newline or carriage return)
            if (ch == '\n' || ch == '\r') {
                // Reset timeout to wait for additional data
                startTime = millis();
            }
        }
        
        // Check if we have complete response
        if (rxIndex > 0 && millis() - startTime > 100) {
            break;  // Got response with 100ms silence
        }
    }
    
    rxBuffer[rxIndex] = '\0';  // Null terminate
    return String(rxBuffer);
}

/**
 * @brief Print response with hex display
 * @param response Response string
 */
void printResponse(String response) {
    // Print as text
    Serial.println(response);
    
    // Also print as hex
    if (response.length() > 0) {
        Serial.print("        [HEX]: ");
        for (uint16_t i = 0; i < response.length(); i++) {
            Serial.printf("%02X ", (uint8_t)response[i]);
        }
        Serial.println();
    }
}

/*
 * AT Command Reference for SX1262-LoRa-DTU:
 * 
 * Connection Setup:
 *   AT+PORT=2        - Select RS485 port
 *   AT+BAUD=9600     - Set baud rate
 *   AT+COMM="8N1"    - Set serial parameters (8 data, 1 stop, no parity)
 * 
 * LoRa Configuration:
 *   AT+MODE=1        - Stream mode (1=stream, 2=packet, 3=relay)
 *   AT+SF=7          - Spreading factor (7-12)
 *   AT+BW=0          - Bandwidth (0=125kHz, 1=250kHz, 2=500kHz)
 *   AT+CR=1          - Coding rate (1=4/5, 2=4/6, 3=4/7, 4=4/8)
 *   AT+PWR=22        - RF power (10-22 dBm)
 *   AT+TXCH=18       - TX channel (0-80)
 *   AT+RXCH=18       - RX channel (0-80)
 * 
 * Address & Network:
 *   AT+ADDR=0        - Device address (0-65535)
 *   AT+NETID=0       - Network ID (0-255)
 *   AT+LBT=0         - LBT mode (0=disable, 1=enable)
 * 
 * System:
 *   AT+VER           - Get firmware version
 *   AT+HELP          - Show help
 *   AT+RSSI=0        - RSSI output (0=disable, 1=enable)
 *   AT+EXIT          - Exit AT mode and return to stream mode
 *   AT+RESTORE=1     - Factory reset
 * 
 * Quick Setup (Stream Mode):
 *   +++              - Enter AT command mode
 *   AT+MODE=1        - Stream mode
 *   AT+PORT=2        - RS485
 *   AT+BAUD=9600     - 9600 baud
 *   AT+SF=7          - Spreading factor
 *   AT+PWR=22        - Max power
 *   AT+EXIT          - Exit and start operation
 */
