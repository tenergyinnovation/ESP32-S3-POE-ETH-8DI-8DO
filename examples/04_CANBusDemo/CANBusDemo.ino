/**
 * @file CANBusDemo.ino
 * @brief Example: CAN 2.0B communication
 * 
 * This example demonstrates:
 * - CAN bus initialization (TWAI on ESP32)
 * - Sending CAN frames
 * - Receiving and parsing CAN frames
 * - CAN communication status monitoring
 */

#include <ESP32S3_8DI8DO.h>
#include <driver/twai.h>

ESP32S3_8DI8DO board;

// CAN configuration
#define CAN_TX_PIN 2        // GPIO2
#define CAN_RX_PIN 3        // GPIO3
#define CAN_BAUDRATE 500000 // 500 kbps

// Timing variables
unsigned long lastStatusTime = 0;
unsigned long lastSendTime = 0;
const uint16_t STATUS_INTERVAL = 5000;   // Print status every 5 seconds
const uint16_t SEND_INTERVAL = 2000;     // Send CAN frame every 2 seconds

// Message counters
uint32_t messagesReceived = 0;
uint32_t messagesSent = 0;
uint32_t messagesErrors = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n===== ESP32-S3-POE-ETH-8DI-8DO =====");
    Serial.println("Example: CAN Bus Communication");
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
    
    // Initialize CAN bus
    Serial.print("Initializing CAN bus...");
    if (initializeCAN()) {
        Serial.println(" OK");
        board.setRGB(0, 255, 0);  // Green = Ready
    } else {
        Serial.println(" FAILED");
        board.setRGB(255, 0, 0);  // Red = Error
        return;
    }
    
    Serial.println("\nCAN Bus ready!");
    Serial.println("=====================================\n");
}

void loop() {
    board.loop();
    
    unsigned long currentTime = millis();
    
    // Check for incoming CAN messages
    handleCANMessages();
    
    // Send CAN frame periodically
    if (currentTime - lastSendTime >= SEND_INTERVAL) {
        lastSendTime = currentTime;
        sendCANFrame();
    }
    
    // Print status periodically
    if (currentTime - lastStatusTime >= STATUS_INTERVAL) {
        lastStatusTime = currentTime;
        printCANStatus();
    }
    
    delay(10);
}

/**
 * Initialize CAN bus using TWAI driver
 */
bool initializeCAN() {
    // TWAI configuration for 500 kbps
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        CAN_TX_PIN, CAN_RX_PIN,
        TWAI_MODE_NORMAL);
    
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBPS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    
    // Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        return false;
    }
    
    // Start TWAI driver
    if (twai_start() != ESP_OK) {
        return false;
    }
    
    return true;
}

/**
 * Handle incoming CAN messages
 */
void handleCANMessages() {
    twai_message_t message;
    
    // Non-blocking read of CAN messages
    if (twai_receive(&message, 0) != ESP_OK) {
        return;  // No message available
    }
    
    messagesReceived++;
    
    Serial.print("📨 CAN Message Received:");
    Serial.print(" ID=0x");
    Serial.print(message.identifier, HEX);
    Serial.print(" DLC=");
    Serial.print(message.data_length_code);
    Serial.print(" Data: ");
    
    // Print data bytes
    for (int i = 0; i < message.data_length_code; i++) {
        if (message.data[i] < 0x10) Serial.print("0");
        Serial.print(message.data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    
    // Parse and display message content
    parseCANMessage(&message);
    
    // Flash LED to indicate message reception
    board.setRGB(0, 0, 255);  // Blue = Message received
    board.beep(50);
    delay(100);
    board.setRGB(0, 255, 0);  // Back to green
}

/**
 * Send CAN frame with test data
 */
void sendCANFrame() {
    twai_message_t message;
    
    // Create test CAN message
    message.extd = 0;              // Standard ID (11-bit)
    message.rtr = 0;               // Not a remote frame
    message.identifier = 0x123;    // CAN ID
    message.data_length_code = 8;  // Data length
    
    // Fill data with test values
    for (int i = 0; i < 8; i++) {
        message.data[i] = (messagesSent + i) & 0xFF;
    }
    
    // Queue message for transmission
    if (twai_transmit(&message, pdMS_TO_TICKS(100)) == ESP_OK) {
        messagesSent++;
        
        Serial.print("📤 CAN Message Sent:");
        Serial.print(" ID=0x");
        Serial.print(message.identifier, HEX);
        Serial.print(" Data: ");
        
        for (int i = 0; i < message.data_length_code; i++) {
            if (message.data[i] < 0x10) Serial.print("0");
            Serial.print(message.data[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
        
        board.beep(100);
    } else {
        messagesErrors++;
        Serial.println("✗ Failed to send CAN message");
    }
}

/**
 * Parse CAN message content
 */
void parseCANMessage(twai_message_t* msg) {
    // Example parsing based on CAN ID
    if (msg->identifier == 0x100) {
        // Temperature data format
        int16_t temp = (msg->data[0] << 8) | msg->data[1];
        Serial.print("  Temperature: ");
        Serial.print(temp / 100.0);
        Serial.println(" °C");
    } else if (msg->identifier == 0x200) {
        // Voltage data format
        uint16_t voltage = (msg->data[0] << 8) | msg->data[1];
        Serial.print("  Voltage: ");
        Serial.print(voltage / 1000.0);
        Serial.println(" V");
    }
}

/**
 * Print CAN bus status
 */
void printCANStatus() {
    twai_status_info_t info;
    twai_get_status_info(&info);
    
    Serial.println("\n┌─── CAN Bus Status ─────────┐");
    Serial.print("  State: ");
    switch (info.state) {
        case TWAI_STATE_RUNNING:
            Serial.println("Running ✓");
            break;
        case TWAI_STATE_STOPPED:
            Serial.println("Stopped");
            break;
        case TWAI_STATE_BUS_OFF:
            Serial.println("Bus Off ✗");
            break;
        default:
            Serial.println("Unknown");
    }
    
    Serial.print("  Baud Rate: ");
    Serial.print(CAN_BAUDRATE / 1000);
    Serial.println(" kbps");
    
    Serial.print("  TX Errors: ");
    Serial.println(info.tx_error_counter);
    
    Serial.print("  RX Errors: ");
    Serial.println(info.rx_error_counter);
    
    Serial.println("├─── Message Statistics ────┤");
    Serial.print("  Sent: ");
    Serial.println(messagesSent);
    
    Serial.print("  Received: ");
    Serial.println(messagesReceived);
    
    Serial.print("  Errors: ");
    Serial.println(messagesErrors);
    
    Serial.println("└────────────────────────────┘\n");
}
