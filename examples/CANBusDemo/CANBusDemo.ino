/*
 * CANBusDemo Example
 * 
 * Demonstrates CAN 2.0B communication on the ESP32-S3-POE-ETH-8DI-8DO board
 * 
 * This example shows how to:
 * - Initialize CAN 2.0B interface
 * - Send CAN frames
 * - Receive and parse CAN messages
 * - Handle CAN errors and status
 * 
 * Hardware Setup:
 * - CAN High (CAN H) → External CAN transceiver
 * - CAN Low (CAN L) → External CAN transceiver
 * - GND → ESP32 GND
 * 
 * Note: Add 120Ω termination resistor across CANH-CANL at bus ends
 * For single-device testing, you can use a CAN transceiver module
 * (e.g., SN65HVD230, TJA1050)
 */

#include <ESP32S3_8DI8DO.h>
#include <driver/twai.h>  // ESP32 CAN driver

// Create board instance
ESP32S3_8DI8DO board;

// CAN configuration
#define CAN_BAUDRATE 500  // 500 Kbps

// CAN frame buffer
twai_message_t txMessage;
twai_message_t rxMessage;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n╔════════════════════════════════════════╗");
    Serial.println("║   ESP32-S3-POE-ETH-8DI-8DO            ║");
    Serial.println("║   CAN Bus Demo                        ║");
    Serial.println("╚════════════════════════════════════════╝\n");
    
    // Initialize board
    if (!board.begin()) {
        Serial.println("❌ Board initialization failed!");
        while (1) delay(1000);
    }
    
    Serial.println(board.getBoardInfo());
    
    // Setup CAN
    Serial.printf("\n🚗 Initializing CAN at %d Kbps...\n", CAN_BAUDRATE);
    if (board.setupCAN(CAN_BAUDRATE)) {
        Serial.println("✓ CAN initialized successfully");
        board.setRGBColor(0, 0, 255);  // Blue
        board.buzzerBeep(2, 100, 100);
    } else {
        Serial.println("❌ CAN initialization failed!");
        board.setRGBColor(255, 0, 0);  // Red
    }
    
    Serial.println("\n📝 CAN Bus Information:");
    Serial.printf("  Baudrate: %d Kbps\n", CAN_BAUDRATE);
    Serial.println("  TX GPIO: GPIO42");
    Serial.println("  RX GPIO: GPIO41");
    Serial.println("\n💡 Standard CAN IDs:");
    Serial.println("  0x123 - Digital IO Status");
    Serial.println("  0x200 - Sensor Data");
    Serial.println("  0x300 - Control Commands");
}

void loop() {
    // Send CAN message every 2 seconds
    static uint32_t lastSend = 0;
    if (millis() - lastSend > 2000) {
        lastSend = millis();
        sendCANMessage();
    }
    
    // Receive CAN messages
    if (twai_receive(&rxMessage, pdMS_TO_TICKS(10)) == ESP_OK) {
        receiveCANMessage();
    }
    
    delay(100);
}

/**
 * @brief Send a test CAN message
 */
void sendCANMessage() {
    // Prepare CAN frame
    txMessage.identifier = 0x123;  // CAN ID
    txMessage.flags = TWAI_MSG_FLAG_NONE;
    txMessage.data_length_code = 8;  // 8 bytes
    
    // Fill data
    static uint8_t counter = 0;
    txMessage.data[0] = counter++;
    txMessage.data[1] = board.readAllInputs();  // Digital input states
    txMessage.data[2] = 0x42;
    txMessage.data[3] = 0x43;
    txMessage.data[4] = (uint8_t)(millis() >> 24);
    txMessage.data[5] = (uint8_t)(millis() >> 16);
    txMessage.data[6] = (uint8_t)(millis() >> 8);
    txMessage.data[7] = (uint8_t)(millis() & 0xFF);
    
    // Send message
    if (twai_transmit(&txMessage, pdMS_TO_TICKS(10)) == ESP_OK) {
        Serial.print("📤 CAN TX [0x");
        Serial.print(txMessage.identifier, HEX);
        Serial.print("]: ");
        printCANData(txMessage);
        
        board.setRGBColor(0, 255, 0);  // Green = sent
        board.buzzerBeep(1, 50, 100);
    } else {
        Serial.println("❌ CAN TX failed!");
        board.setRGBColor(255, 0, 0);  // Red
    }
}

/**
 * @brief Handle received CAN message
 */
void receiveCANMessage() {
    Serial.print("📥 CAN RX [0x");
    Serial.print(rxMessage.identifier, HEX);
    Serial.print("]: ");
    printCANData(rxMessage);
    
    // Process based on CAN ID
    switch (rxMessage.identifier) {
        case 0x123:
            handleDigitalIOMessage();
            break;
        case 0x200:
            handleSensorDataMessage();
            break;
        case 0x300:
            handleControlMessage();
            break;
        default:
            Serial.println("  (Unknown CAN ID)");
    }
}

/**
 * @brief Handle digital I/O status message
 */
void handleDigitalIOMessage() {
    if (rxMessage.data_length_code >= 2) {
        uint8_t inputs = rxMessage.data[1];
        Serial.println("  → Digital Input Status");
        Serial.print("    Inputs: [");
        for (uint8_t i = 0; i < 8; i++) {
            Serial.print((inputs & (1 << i)) ? "1" : "0");
        }
        Serial.println("]");
    }
}

/**
 * @brief Handle sensor data message
 */
void handleSensorDataMessage() {
    Serial.println("  → Sensor Data");
    if (rxMessage.data_length_code >= 4) {
        int16_t temp = (int16_t)((rxMessage.data[0] << 8) | rxMessage.data[1]);
        uint16_t humidity = (uint16_t)((rxMessage.data[2] << 8) | rxMessage.data[3]);
        Serial.printf("    Temperature: %d°C, Humidity: %d%%\n", temp, humidity);
    }
}

/**
 * @brief Handle control command message
 */
void handleControlMessage() {
    Serial.println("  → Control Command");
    if (rxMessage.data_length_code >= 1) {
        uint8_t cmd = rxMessage.data[0];
        switch (cmd) {
            case 0x01:
                Serial.println("    Command: Turn ON");
                board.setDigitalOutput(0, HIGH);
                board.buzzerBeep(1, 100, 50);
                break;
            case 0x02:
                Serial.println("    Command: Turn OFF");
                board.setDigitalOutput(0, LOW);
                board.buzzerBeep(2, 50, 50);
                break;
            case 0x03:
                Serial.println("    Command: Reset");
                board.buzzerBeep(3, 50, 50);
                break;
            default:
                Serial.printf("    Command: 0x%02X (unknown)\n", cmd);
        }
    }
}

/**
 * @brief Print CAN data in hex format
 */
void printCANData(const twai_message_t& msg) {
    Serial.print("[");
    for (uint8_t i = 0; i < msg.data_length_code; i++) {
        if (i > 0) Serial.print(" ");
        Serial.printf("%02X", msg.data[i]);
    }
    Serial.println("]");
}

/*
 * Common CAN Bus Configurations:
 * 
 * 125 Kbps  - Industrial automation (long distance)
 * 250 Kbps  - Industrial automation (standard)
 * 500 Kbps  - Automotive and industrial (most common)
 * 1 Mbps    - High-speed industrial and automotive
 * 
 * Standard CAN Frame Format:
 * - SOF (Start of Frame) - 1 bit
 * - Identifier - 11 bits (Standard) or 29 bits (Extended)
 * - Data Length Code (DLC) - 4 bits (0-8 bytes)
 * - Data Field - 0-8 bytes (64 bits max)
 * - CRC - 15 bits
 * - EOF (End of Frame) - 7 bits
 * 
 * CAN Message Priority:
 * - Lower CAN ID = Higher priority
 * - Arbitration field determines bus access
 * 
 * Example CAN IDs:
 * 0x001-0x0FF - High priority (real-time)
 * 0x100-0x1FF - Medium priority
 * 0x200-0x2FF - Low priority (informational)
 * 0x300-0x3FF - Diagnostic
 * 0x400-0x7FF - Application specific
 */
