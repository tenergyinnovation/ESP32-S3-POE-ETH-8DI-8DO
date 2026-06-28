/***********************************************************************
 * File         :     ClassUsageExamples.ino
 * Description  :     Examples showing how to use each class from the
 *                    consolidated ESP32S3_8DI8DO library
 * Author       :     Tenergy Innovation Co., Ltd.
 * Date         :     28 Jun 2026
 ***********************************************************************/

#include <ESP32S3_8DI8DO.h>

// ============================================================
// GLOBAL INSTANCES
// ============================================================

// Create main board controller instance
ESP32S3_8DI8DO board;

// You can also access subsystems directly through the board instance:
// board.getDI()      → ESP32S3_DI subsystem
// board.getDO()      → ESP32S3_DO subsystem
// board.getRS485()   → ESP32S3_RS485 subsystem
// board.getRGB()     → ESP32S3_RGB subsystem
// board.getBuzzer()  → ESP32S3_Buzzer subsystem

// ============================================================
// SETUP
// ============================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=== CLASS USAGE EXAMPLES ===\n");
    
    // Initialize the board
    if (board.begin() == ESP32S3_8DI8DO::STATUS_OK) {
        Serial.println("✓ Board initialized successfully!\n");
    } else {
        Serial.println("✗ Board initialization failed!\n");
    }
}

// ============================================================
// LOOP - CYCLE THROUGH EXAMPLES
// ============================================================

void loop() {
    example_DI();
    delay(2000);
    
    example_DO();
    delay(2000);
    
    example_RS485();
    delay(2000);
    
    example_RGB();
    delay(5000);
    
    example_Buzzer();
    delay(2000);
    
    example_integrated();
    delay(3000);
}

// ============================================================
// EXAMPLE 1: DIGITAL INPUT (DI) CLASS
// ============================================================

void example_DI() {
    Serial.println("\n--- EXAMPLE 1: DIGITAL INPUT (DI) ---");
    Serial.println("Reading all 8 input channels:\n");
    
    // Access DI subsystem through board
    ESP32S3_DI& di = board.getDI();
    
    // Method 1: Read all inputs at once (returns 8-bit value)
    uint8_t allInputs = di.readAll();
    Serial.printf("All inputs (binary): ");
    for (int i = 7; i >= 0; i--) {
        Serial.printf("%d", (allInputs >> i) & 1);
    }
    Serial.println();
    
    // Method 2: Read individual channels
    Serial.println("\nIndividual channel readings:");
    for (int ch = 0; ch < 8; ch++) {
        bool state = di.read(ch);
        Serial.printf("  DI%d: %s\n", ch+1, state ? "HIGH" : "LOW");
    }
    
    // Method 3: Enable debouncing for specific channel
    di.enableDebounce(0, 20);  // 20ms debounce on channel 0
    Serial.println("\n✓ Debouncing enabled on DI1 (20ms)");
}

// ============================================================
// EXAMPLE 2: DIGITAL OUTPUT (DO) CLASS
// ============================================================

void example_DO() {
    Serial.println("\n--- EXAMPLE 2: DIGITAL OUTPUT (DO) ---");
    Serial.println("Controlling 8 relay outputs via TCA9554:\n");
    
    // Access DO subsystem through board
    ESP32S3_DO& do_ctrl = board.getDO();
    
    // Method 1: Control individual relay
    Serial.println("Turning ON relays 1-4:");
    do_ctrl.on(0);   // DO1 ON
    do_ctrl.on(1);   // DO2 ON
    do_ctrl.on(2);   // DO3 ON
    do_ctrl.on(3);   // DO4 ON
    delay(500);
    
    Serial.println("Turning OFF relays 1-4:");
    do_ctrl.off(0);
    do_ctrl.off(1);
    do_ctrl.off(2);
    do_ctrl.off(3);
    delay(500);
    
    // Method 2: Toggle relay
    Serial.println("Toggling relay 1:");
    for (int i = 0; i < 3; i++) {
        do_ctrl.toggle(0);
        delay(200);
    }
    
    // Method 3: Control all outputs at once with bitmask
    Serial.println("Setting pattern: 10101010");
    do_ctrl.setMask(0b10101010);  // Alternating pattern
    delay(500);
    
    // Method 4: Quick on/off all
    do_ctrl.allOff();
    Serial.println("All relays OFF");
}

// ============================================================
// EXAMPLE 3: RS485 COMMUNICATION CLASS
// ============================================================

void example_RS485() {
    Serial.println("\n--- EXAMPLE 3: RS485 COMMUNICATION ---");
    Serial.println("Communication setup and basic operations:\n");
    
    // Access RS485 subsystem through board
    ESP32S3_RS485& rs485 = board.getRS485();
    
    // Method 1: Send AT command and wait for response
    Serial.println("Sending AT command to LoRa DTU:");
    rs485.setCommMode(ESP32S3_RS485::MODE_AT_COMMAND);
    String response = rs485.sendATCommand("AT+VER", 1000);
    
    if (response.length() > 0) {
        Serial.printf("Response: %s\n", response.c_str());
    } else {
        Serial.println("No response (timeout or no module connected)");
    }
    
    // Method 2: Write raw data
    Serial.println("\nSending raw data:");
    const uint8_t data[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01};
    rs485.write(data, 6);
    
    // Method 3: Check available data
    Serial.printf("Available bytes to read: %u\n", rs485.available());
    
    // Method 4: Get communication statistics
    Serial.printf("Bytes sent: %u\n", rs485.getBytesSent());
    Serial.printf("Bytes received: %u\n", rs485.getBytesReceived());
}

// ============================================================
// EXAMPLE 4: RGB LED CLASS
// ============================================================

void example_RGB() {
    Serial.println("\n--- EXAMPLE 4: RGB LED CONTROL ---");
    Serial.println("LED color and animation modes:\n");
    
    // Access RGB subsystem through board
    ESP32S3_RGB& rgb = board.getRGB();
    
    // Method 1: Solid colors
    Serial.println("Solid colors:");
    
    Serial.println("  RED");
    rgb.setColor(255, 0, 0);
    delay(500);
    
    Serial.println("  GREEN");
    rgb.setColor(0, 255, 0);
    delay(500);
    
    Serial.println("  BLUE");
    rgb.setColor(0, 0, 255);
    delay(500);
    
    Serial.println("  YELLOW");
    rgb.setColor(255, 255, 0);
    delay(500);
    
    // Method 2: Status colors (predefined)
    Serial.println("\nStatus colors:");
    
    Serial.println("  STATUS_OK (Green)");
    rgb.setStatusColor(ESP32S3_RGB::COLOR_GREEN);
    delay(500);
    
    Serial.println("  WARNING (Yellow)");
    rgb.setStatusColor(ESP32S3_RGB::COLOR_YELLOW);
    delay(500);
    
    Serial.println("  ERROR (Red)");
    rgb.setStatusColor(ESP32S3_RGB::COLOR_RED);
    delay(500);
    
    // Method 3: Blinking animation
    Serial.println("\nBlinking animation:");
    rgb.blink(0xFF0000, 250, 250);  // Red, 250ms on/off
    delay(3000);
    
    // Method 4: Pulse animation
    Serial.println("Pulse animation:");
    rgb.pulse(0x00FF00, 2000);  // Green, 2s period
    delay(3000);
    
    // Method 5: Brightness control
    Serial.println("Brightness adjustment:");
    rgb.setColor(255, 255, 255);  // White
    rgb.setBrightness(64);         // 25% brightness
    delay(1000);
    rgb.setBrightness(255);        // 100% brightness
    
    rgb.stop();  // Stop animation, turn off
}

// ============================================================
// EXAMPLE 5: BUZZER CLASS
// ============================================================

void example_Buzzer() {
    Serial.println("\n--- EXAMPLE 5: BUZZER CONTROL ---");
    Serial.println("Beep patterns and sequences:\n");
    
    // Access Buzzer subsystem through board
    ESP32S3_Buzzer& buzzer = board.getBuzzer();
    
    // Method 1: Simple beep
    Serial.println("Single beep (100ms):");
    buzzer.beep(100);
    delay(300);
    
    // Method 2: Multiple beeps
    Serial.println("Double beep:");
    buzzer.beep(100);
    delay(200);
    buzzer.beep(100);
    delay(300);
    
    // Method 3: Pattern-based beeping
    Serial.println("Short pattern (Morse SOS):");
    buzzer.playPattern(ESP32S3_Buzzer::PATTERN_SHORT);
    delay(300);
    buzzer.playPattern(ESP32S3_Buzzer::PATTERN_SHORT);
    delay(300);
    buzzer.playPattern(ESP32S3_Buzzer::PATTERN_SHORT);
    delay(500);
    
    // Method 4: Frequency control (musical notes)
    Serial.println("Playing musical notes:");
    
    Serial.println("  Note C4 (262 Hz)");
    buzzer.setFrequency(ESP32S3_Buzzer::FREQ_C4);
    buzzer.beep(200);
    delay(300);
    
    Serial.println("  Note E4 (330 Hz)");
    buzzer.setFrequency(ESP32S3_Buzzer::FREQ_E4);
    buzzer.beep(200);
    delay(300);
    
    Serial.println("  Note G4 (392 Hz)");
    buzzer.setFrequency(ESP32S3_Buzzer::FREQ_G4);
    buzzer.beep(200);
    delay(300);
    
    // Method 5: Volume control
    Serial.println("Volume control:");
    buzzer.setVolume(128);  // 50%
    buzzer.beep(100);
    delay(300);
    buzzer.setVolume(255);  // 100%
    buzzer.beep(100);
}

// ============================================================
// EXAMPLE 6: INTEGRATED CONTROL
// ============================================================

void example_integrated() {
    Serial.println("\n--- EXAMPLE 6: INTEGRATED SYSTEM CONTROL ---");
    Serial.println("Using multiple subsystems together:\n");
    
    // Scenario: Warning system
    Serial.println("Scenario: Device warning alert\n");
    
    // Set yellow warning LED
    board.setRGBColor(255, 255, 0);
    Serial.println("✓ LED: Yellow warning");
    
    // Activate all digital outputs (relay control)
    board.setAllOutputs(0xFF);
    Serial.println("✓ DO: All relays ON");
    
    // Sound buzzer alarm
    board.buzzerBeep(3, 200, 100);  // 3 beeps, 200ms each, 100ms interval
    Serial.println("✓ Buzzer: 3 beep alarm\n");
    
    // Check input status
    uint8_t inputs = board.readAllInputs();
    Serial.printf("Input status: 0x%02X\n", inputs);
    
    // Send status message via RS485
    board.getRS485().writeString("WARNING: System Alert Active\r\n");
    Serial.println("✓ RS485: Status message sent\n");
    
    // Turn off outputs
    board.setAllOutputs(0x00);
    board.LEDOff();
    Serial.println("✓ System reset to safe state");
}

// ============================================================
// OPTIONAL: CALLBACK EXAMPLE FOR DI
// ============================================================

// Uncomment to use DI change callback
/*
void diChangeCallback(uint8_t channel, bool newState) {
    Serial.printf("DI%d changed to: %s\n", channel+1, newState ? "HIGH" : "LOW");
}

void setupDICallbacks() {
    board.getDI().setChangeCallback(0, diChangeCallback);
    board.getDI().setChangeCallback(1, diChangeCallback);
}
*/
