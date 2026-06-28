/**
 * @file BasicDigitalIO.ino
 * @brief Example: Basic Digital Input/Output control
 * 
 * This example demonstrates:
 * - Reading 8 Digital Input channels
 * - Controlling 8 Digital Output relay channels
 * - LED status indicator
 * - Serial monitoring
 */

#include <ESP32S3_8DI8DO.h>

// Global instance of the library
ESP32S3_8DI8DO board;

// Timing variables
unsigned long lastReadTime = 0;
unsigned long lastPrintTime = 0;
const uint16_t READ_INTERVAL = 100;      // Read inputs every 100ms
const uint16_t PRINT_INTERVAL = 1000;    // Print status every 1 second

void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n===== ESP32-S3-POE-ETH-8DI-8DO =====");
    Serial.println("Example: Basic Digital I/O");
    Serial.println("=====================================\n");
    
    // Initialize the board
    Serial.print("Initializing board...");
    ESP32S3_8DI8DO::Status status = board.begin();
    
    if (status == ESP32S3_8DI8DO::STATUS_OK) {
        Serial.println(" ✓ OK");
        board.setRGBColor(0, 255, 0);  // Green = Ready
    } else {
        Serial.println(" ✗ FAILED");
        board.setRGBColor(255, 0, 0);  // Red = Error
        while (1) {
            board.buzzer_beep(1);
            delay(500);
        }
    }
    
    // Enable debouncing for all DI channels
    for (uint8_t ch = 0; ch < 8; ch++) {
        board.getDI().enableDebounce(ch, 20);  // 20ms debounce
    }
    
    Serial.println("\nBoard initialized successfully!");
    Serial.println("Monitoring Digital Inputs (DI1-DI8)");
    Serial.println("Available commands:");
    Serial.println("  '1'-'8'  : Toggle DO channel 1-8");
    Serial.println("  'A'      : Turn ON all outputs");
    Serial.println("  'O'      : Turn OFF all outputs");
    Serial.println("  'R'      : Read inputs");
    Serial.println("=====================================\n");
}

void loop() {
    // Update board systems (important for debouncing, LED animation, etc.)
    board.loop();
    
    // Read inputs periodically
    unsigned long currentTime = millis();
    
    if (currentTime - lastReadTime >= READ_INTERVAL) {
        lastReadTime = currentTime;
        
        // Read all input channels and display changes
        static uint8_t previousDIState = 0;
        uint8_t currentDIState = 0;
        
        for (uint8_t ch = 1; ch <= 8; ch++) {
            if (board.getDigitalInput(ch)) {
                currentDIState |= (1 << (ch - 1));
            }
        }
        
        // Only print if state changed
        if (currentDIState != previousDIState) {
            previousDIState = currentDIState;
            printDIStatus(currentDIState);
        }
    }
    
    // Print full status periodically
    if (currentTime - lastPrintTime >= PRINT_INTERVAL) {
        lastPrintTime = currentTime;
        printFullStatus();
    }
    
    // Handle serial input for controlling outputs
    if (Serial.available() > 0) {
        char command = Serial.read();
        handleCommand(command);
    }
    
    delay(10);  // Small delay to prevent watchdog timeout
}

/**
 * Print Digital Input status
 */
void printDIStatus(uint8_t state) {
    Serial.print("🔹 Digital Inputs: ");
    for (uint8_t i = 0; i < 8; i++) {
        if (state & (1 << i)) {
            Serial.print("DI");
            Serial.print(i + 1);
            Serial.print("=ON ");
        }
    }
    Serial.println();
}

/**
 * Print full system status
 */
void printFullStatus() {
    Serial.println("\n┌─── Digital Input Status ───┐");
    for (uint8_t ch = 1; ch <= 8; ch++) {
        Serial.print("  DI");
        Serial.print(ch);
        Serial.print(": ");
        board.getDigitalInput(ch) ? Serial.println("HIGH ✓") : Serial.println("LOW  ✗");
    }
    
    Serial.println("├─── Digital Output Status ──┤");
    for (uint8_t ch = 1; ch <= 8; ch++) {
        Serial.print("  DO");
        Serial.print(ch);
        Serial.print(": ");
        board.getDO().getState(ch) ? Serial.println("ON  ✓") : Serial.println("OFF ✗");
    }
    Serial.println("└─────────────────────────────┘\n");
}

/**
 * Handle serial commands for output control
 */
void handleCommand(char cmd) {
    cmd = toupper(cmd);
    
    if (cmd >= '1' && cmd <= '8') {
        // Toggle DO 1-8
        uint8_t ch = cmd - '0';
        board.getDO().toggle(ch);
        
        Serial.print("▶ DO");
        Serial.print(ch);
        Serial.print(" toggled: ");
        Serial.println(board.getDO().getState(ch) ? "ON" : "OFF");
        
        // Short beep feedback
        board.buzzer_beep(1);
        
    } else if (cmd == 'A') {
        // All ON
        for (uint8_t ch = 1; ch <= 8; ch++) {
            board.getDO().on(ch);
        }
        Serial.println("▶ All outputs turned ON");
        board.setRGBColor(0, 255, 0);
        board.buzzer_beep(1);
        
    } else if (cmd == 'O') {
        // All OFF
        for (uint8_t ch = 1; ch <= 8; ch++) {
            board.getDO().off(ch);
        }
        Serial.println("▶ All outputs turned OFF");
        board.setRGBColor(255, 255, 0);
        board.buzzer_beep(1);
        
    } else if (cmd == 'R') {
        // Print status
        printFullStatus();
        
    } else {
        Serial.println("? Unknown command");
    }
}
