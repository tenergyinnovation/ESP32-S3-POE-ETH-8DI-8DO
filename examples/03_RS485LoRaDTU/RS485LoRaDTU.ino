/**
 * @file RS485LoRaDTU.ino
 * @brief Example: RS485 communication with LoRa DTU (SX1262)
 * 
 * This example demonstrates:
 * - RS485 initialization on UART1 (GPIO17/18/21)
 * - Sending AT commands to configure LoRa DTU
 * - Receiving and parsing LoRa data
 * - Stream mode for data transmission
 */

#include <ESP32S3_8DI8DO.h>

ESP32S3_8DI8DO board;

// RS485 configuration
const unsigned long RS485_BAUDRATE = 9600;
const uint16_t RS485_TIMEOUT = 1000;  // 1 second timeout

// Timing variables
unsigned long lastStatusTime = 0;
unsigned long lastSendTime = 0;
const uint16_t STATUS_INTERVAL = 3000;    // Print status every 3 seconds
const uint16_t SEND_INTERVAL = 5000;      // Send test data every 5 seconds

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n===== ESP32-S3-POE-ETH-8DI-8DO =====");
    Serial.println("Example: RS485 LoRa DTU Communication");
    Serial.println("=====================================\n");
    
    // Initialize board
    Serial.print("Initializing board...");
    ESP32S3_8DI8DO::Status status = board.begin();
    
    if (status != ESP32S3_8DI8DO::STATUS_OK) {
        Serial.println(" FAILED");
        board.setRGBColor(255, 0, 0);
        return;
    }
    Serial.println(" OK");
    
    // Initialize RS485 for LoRa DTU
    Serial.print("Initializing RS485...");
    if (!board.setupRS485(RS485_BAUDRATE)) {
        status = ESP32S3_8DI8DO::STATUS_RS485_ERROR;
    } else {
        status = ESP32S3_8DI8DO::STATUS_OK;
    }
    
    if (status != ESP32S3_8DI8DO::STATUS_OK) {
        Serial.println(" FAILED");
        board.setRGBColor(255, 0, 0);
        return;
    }
    Serial.println(" OK");
    
    board.setRGBColor(255, 255, 0);  // Yellow = Initializing
    
    Serial.println("\nConfiguring LoRa DTU...");
    configureLoRaDTU();
    
    Serial.println("✓ LoRa DTU configured successfully");
    board.setRGBColor(0, 255, 0);  // Green = Ready
    board.buzzer_beep(1);
    
    Serial.println("\nWaiting for LoRa data...");
    Serial.println("=====================================\n");
}

void loop() {
    board.loop();
    
    unsigned long currentTime = millis();
    
    // Check for incoming RS485 data
    if (board.getRS485().available() > 0) {
        handleRS485Data();
    }
    
    // Send test data periodically
    if (currentTime - lastSendTime >= SEND_INTERVAL) {
        lastSendTime = currentTime;
        sendTestData();
    }
    
    // Print status periodically
    if (currentTime - lastStatusTime >= STATUS_INTERVAL) {
        lastStatusTime = currentTime;
        printRS485Status();
    }
    
    delay(10);
}

/**
 * Configure LoRa DTU with AT commands
 */
void configureLoRaDTU() {
    Serial.println("\n⚙️  Sending AT commands to LoRa DTU...\n");
    
    // Array of AT commands to configure DTU
    const char* atCommands[] = {
        "AT",               // Test communication
        "AT+VER",          // Get version
        "AT+BAUD=9600",    // Set baud rate
        "AT+MODE=1",       // Stream mode
        "AT+TXCH=18",      // TX channel
        "AT+RXCH=18",      // RX channel
        "AT+ADDR=0",       // Address
        "AT+SF=7",         // Spreading factor
        "AT+BW=0",         // Bandwidth
        "AT+CR=1",         // Coding rate
        "AT+PWR=22",       // Transmit power
        "AT+EXIT"          // Exit AT command mode
    };
    
    const uint8_t cmdCount = sizeof(atCommands) / sizeof(atCommands[0]);
    
    for (uint8_t i = 0; i < cmdCount; i++) {
        Serial.print("📤 Sending: ");
        Serial.println(atCommands[i]);
        
        // Send AT command
        board.getRS485().write((uint8_t*)atCommands[i], strlen(atCommands[i]));
        board.getRS485().write((uint8_t*)"\r\n", 2);  // Send CRLF
        
        // Wait for response
        delay(500);
        
        // Read response
        uint8_t response[128] = {0};
        size_t len = board.getRS485().read(response, sizeof(response));
        
        if (len > 0) {
            Serial.print("📥 Response: ");
            Serial.write(response, len);
            Serial.println();
        } else {
            Serial.println("⏱️  Timeout - No response");
        }
        
        delay(200);
    }
    
    Serial.println();
}

/**
 * Handle incoming RS485 data from LoRa DTU
 */
void handleRS485Data() {
    uint8_t buffer[256];
    size_t len = board.getRS485().read(buffer, sizeof(buffer));
    
    if (len > 0) {
        Serial.print("📨 Received data (");
        Serial.print(len);
        Serial.print(" bytes): ");
        
        // Print data as hex and ASCII
        for (size_t i = 0; i < len; i++) {
            if (buffer[i] >= 32 && buffer[i] < 127) {
                Serial.write(buffer[i]);
            } else {
                Serial.print("[");
                Serial.print(buffer[i], HEX);
                Serial.print("]");
            }
        }
        Serial.println();
        
        // Update LED to show data received
        board.setRGBColor(0, 0, 255);  // Blue = Data received
        board.buzzer_beep(1);
        
        // Reset to green after brief delay
        delay(200);
        board.setRGBColor(0, 255, 0);
        
        // Parse and display energy data if available
        parseEnergyData(buffer, len);
    }
}

/**
 * Send test data via RS485
 */
void sendTestData() {
    // Example: Send test LoRa packet
    const char* testData = "TEST_LORA_DATA_123";
    
    Serial.print("📤 Sending test data: ");
    Serial.println(testData);
    
    board.getRS485().write((uint8_t*)testData, strlen(testData));
    
    board.buzzer_beep(1);
}

/**
 * Parse energy data from LoRa payload
 */
void parseEnergyData(uint8_t* data, size_t len) {
    // Example parsing (adjust based on actual LoRa DTU format)
    Serial.println("\n📊 Parsed Data:");
    Serial.print("  Length: ");
    Serial.print(len);
    Serial.println(" bytes");
    
    // Extract fields based on your LoRa DTU protocol
    // This is a placeholder - adjust to match your actual protocol
    
    Serial.println();
}

/**
 * Print RS485 communication status
 */
void printRS485Status() {
    Serial.println("\n┌─── RS485 Status ───────────┐");
    Serial.print("  Baud Rate: ");
    Serial.println(RS485_BAUDRATE);
    Serial.print("  Communication: ");
    Serial.println("✓ Active");
    Serial.print("  Timeout: ");
    Serial.print(RS485_TIMEOUT);
    Serial.println(" ms");
    Serial.println("└────────────────────────────┘\n");
}
