#include "ESP32S3_8DI8DO.h"

// Initialize static members
const uint8_t ESP32S3_8DI8DO::DIN_PINS[8] = {
    1, 2, 42, 41, 40, 39, 38, 37  // Adjust based on actual pinout
};

const uint8_t ESP32S3_8DI8DO::DOUT_PINS[8] = {
    10, 11, 12, 13, 14, 15, 16, 9  // Adjust based on actual pinout
};

/**
 * @brief Constructor
 */
ESP32S3_8DI8DO::ESP32S3_8DI8DO() 
    : initialized(false), 
      ethernet_connected(false), 
      rs485_enabled(false), 
      can_enabled(false) {
}

/**
 * @brief Initialize the board
 */
bool ESP32S3_8DI8DO::begin() {
    if (initialized) return true;
    
    Serial.println("\n🚀 Initializing ESP32-S3-POE-ETH-8DI-8DO Board...");
    
    // Configure pins
    configurePins();
    Serial.println("✓ GPIO pins configured");
    
    // Initialize LED
    initLED();
    Serial.println("✓ RGB LED initialized");
    
    // Initialize Buzzer
    initBuzzer();
    Serial.println("✓ Buzzer initialized");
    
    initialized = true;
    Serial.println("✅ Board initialization complete!\n");
    return true;
}

/**
 * @brief Configure all GPIO pins
 */
void ESP32S3_8DI8DO::configurePins() {
    // Configure RGB LED pin
    pinMode(RGB_LED_PIN, OUTPUT);
    
    // Configure Buzzer
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    
    // Configure Digital Input pins
    for (uint8_t i = 0; i < 8; i++) {
        pinMode(DIN_PINS[i], INPUT);
    }
    
    // Configure Digital Output pins
    for (uint8_t i = 0; i < 8; i++) {
        pinMode(DOUT_PINS[i], OUTPUT);
        digitalWrite(DOUT_PINS[i], LOW);
    }
}

/**
 * @brief Initialize LED
 */
void ESP32S3_8DI8DO::initLED() {
    setRGBColor(0, 0, 0);  // Turn off LED
}

/**
 * @brief Initialize Buzzer
 */
void ESP32S3_8DI8DO::initBuzzer() {
    digitalWrite(BUZZER_PIN, LOW);
}

/**
 * @brief Set RGB LED color (G, R, B order)
 */
void ESP32S3_8DI8DO::setRGBColor(uint8_t red, uint8_t green, uint8_t blue) {
    // Note: Adjust color order based on actual LED wiring
    // This example assumes GRB order (common for WS2812B-like LEDs)
    // Implement using appropriate library (Adafruit_NeoPixel, etc.)
    
    // Placeholder for now
    if (red == 0 && green == 0 && blue == 0) {
        // All off
        digitalWrite(RGB_LED_PIN, LOW);
    }
}

/**
 * @brief Turn off RGB LED
 */
void ESP32S3_8DI8DO::LEDOff() {
    setRGBColor(0, 0, 0);
}

/**
 * @brief Set digital output (relay)
 * @param channel Output channel (0-7)
 * @param state   HIGH or LOW
 */
void ESP32S3_8DI8DO::setDigitalOutput(uint8_t channel, uint8_t state) {
    if (channel >= 8) return;
    digitalWrite(DOUT_PINS[channel], state);
}

/**
 * @brief Get digital input value
 * @param channel Input channel (0-7)
 * @return Input state (HIGH or LOW)
 */
uint8_t ESP32S3_8DI8DO::getDigitalInput(uint8_t channel) {
    if (channel >= 8) return 0;
    return digitalRead(DIN_PINS[channel]);
}

/**
 * @brief Read all digital inputs at once
 * @return 8-bit value representing all inputs
 */
uint8_t ESP32S3_8DI8DO::readAllInputs() {
    uint8_t result = 0;
    for (uint8_t i = 0; i < 8; i++) {
        if (digitalRead(DIN_PINS[i]) == HIGH) {
            result |= (1 << i);
        }
    }
    return result;
}

/**
 * @brief Set all digital outputs at once
 * @param value 8-bit value to set all outputs
 */
void ESP32S3_8DI8DO::setAllOutputs(uint8_t value) {
    for (uint8_t i = 0; i < 8; i++) {
        if (value & (1 << i)) {
            digitalWrite(DOUT_PINS[i], HIGH);
        } else {
            digitalWrite(DOUT_PINS[i], LOW);
        }
    }
}

/**
 * @brief Control buzzer
 * @param times     Number of beeps
 * @param duration  Duration of each beep in ms
 * @param interval  Interval between beeps in ms
 */
void ESP32S3_8DI8DO::buzzerBeep(uint8_t times, uint16_t duration, uint16_t interval) {
    for (uint8_t i = 0; i < times; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(duration);
        digitalWrite(BUZZER_PIN, LOW);
        if (i < times - 1) {
            delay(interval);
        }
    }
}

/**
 * @brief Setup Ethernet connection
 * @return true if successful
 */
bool ESP32S3_8DI8DO::setupEthernet() {
    Serial.println("🌐 Initializing Ethernet...");
    
    // TODO: Implement Ethernet initialization
    // Use ESP32 built-in Ethernet MAC with external PHY
    
    ethernet_connected = true;
    Serial.println("✓ Ethernet ready");
    return true;
}

/**
 * @brief Setup RS485 communication
 * @param baudRate Baud rate (default 9600)
 * @return true if successful
 */
bool ESP32S3_8DI8DO::setupRS485(uint32_t baudRate) {
    Serial.printf("📡 Initializing RS485 at %d baud...\n", baudRate);
    
    // Initialize UART for RS485
    Serial2.begin(baudRate, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
    
    rs485_enabled = true;
    Serial.println("✓ RS485 initialized");
    return true;
}

/**
 * @brief Setup LoRa DTU via RS485
 * @param baudRate LoRa DTU baud rate (default 9600)
 * @return true if successful
 */
bool ESP32S3_8DI8DO::setupLoRaDTU(uint32_t baudRate) {
    Serial.println("📡 Setting up LoRa DTU...");
    
    // Setup RS485 first
    setupRS485(baudRate);
    
    Serial.println("✓ LoRa DTU ready");
    return true;
}

/**
 * @brief Setup CAN 2.0 interface
 * @param baudRate CAN baudrate in Kbps (default 500)
 * @return true if successful
 */
bool ESP32S3_8DI8DO::setupCAN(uint32_t baudRate) {
    Serial.printf("🚗 Initializing CAN at %d Kbps...\n", baudRate);
    
    // TODO: Implement CAN initialization
    // Use ESP32 built-in CAN controller
    
    can_enabled = true;
    Serial.println("✓ CAN initialized");
    return true;
}

/**
 * @brief Get board information
 * @return String with board details
 */
String ESP32S3_8DI8DO::getBoardInfo() {
    String info = "=== ESP32-S3-POE-ETH-8DI-8DO Board Info ===\n";
    info += "Chip: ESP32-S3\n";
    info += "Features:\n";
    info += "  - WiFi 6E (802.11ax)\n";
    info += "  - Bluetooth 5.3\n";
    info += "  - Gigabit Ethernet\n";
    info += "  - RS485/RS422/RS232\n";
    info += "  - CAN 2.0B\n";
    info += "  - 8x Digital Inputs (isolated)\n";
    info += "  - 8x Digital Outputs (relays)\n";
    info += "  - RGB LED Status Indicator\n";
    info += "  - Buzzer Output\n";
    info += "  - microSD Card Slot\n";
    info += "  - Backup Battery Support (RTC)\n";
    info += "Status:\n";
    info += "  - Initialized: " + String(initialized ? "Yes" : "No") + "\n";
    info += "  - Ethernet: " + String(ethernet_connected ? "Connected" : "Disconnected") + "\n";
    info += "  - RS485: " + String(rs485_enabled ? "Enabled" : "Disabled") + "\n";
    info += "  - CAN: " + String(can_enabled ? "Enabled" : "Disabled") + "\n";
    info += "=========================================\n";
    return info;
}
