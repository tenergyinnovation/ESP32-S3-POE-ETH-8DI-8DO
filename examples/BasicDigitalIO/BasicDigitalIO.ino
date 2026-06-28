/*
 * BasicDigitalIO Example
 * 
 * Demonstrates digital input/output control on the ESP32-S3-POE-ETH-8DI-8DO board
 * 
 * This example shows how to:
 * - Read digital inputs
 * - Control digital outputs
 * - Use RGB LED for status indication
 * - Control buzzer for feedback
 * 
 * Hardware Setup:
 * - Connect switches to DIN[0-7]
 * - Connect LEDs/loads to DOUT[0-7]
 */

#include <ESP32S3_8DI8DO.h>

// Create board instance
ESP32S3_8DI8DO board;

// State tracking
uint32_t lastUpdate = 0;
const uint32_t UPDATE_INTERVAL = 500;  // 500ms update interval

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n╔════════════════════════════════════════╗");
    Serial.println("║   ESP32-S3-POE-ETH-8DI-8DO            ║");
    Serial.println("║   BasicDigitalIO Example              ║");
    Serial.println("╚════════════════════════════════════════╝\n");
    
    // Initialize board
    if (!board.begin()) {
        Serial.println("❌ Board initialization failed!");
        while (1) {
            delay(1000);
        }
    }
    
    // Show board info
    Serial.println(board.getBoardInfo());
    
    // Initial LED color (blue = ready)
    board.setRGBColor(0, 0, 255);
    Serial.println("🟦 LED set to BLUE (Ready)");
    
    // Welcome beep
    board.buzzerBeep(2, 100, 100);
    Serial.println("🔔 Buzzer beep (Ready)\n");
    
    Serial.println("📝 Controls:");
    Serial.println("  - DIN[0-7] connected to switches");
    Serial.println("  - DOUT[0-7] will mirror DIN inputs\n");
}

void loop() {
    uint32_t now = millis();
    
    // Update at regular interval
    if (now - lastUpdate >= UPDATE_INTERVAL) {
        lastUpdate = now;
        
        // Read all inputs
        uint8_t inputs = board.readAllInputs();
        
        // Display input status
        Serial.print("📥 Inputs: [");
        for (uint8_t i = 0; i < 8; i++) {
            Serial.print((inputs & (1 << i)) ? "1" : "0");
        }
        Serial.print("] | ");
        
        // Set outputs to mirror inputs (demonstration)
        board.setAllOutputs(inputs);
        
        // Display output status
        Serial.print("📤 Outputs: [");
        for (uint8_t i = 0; i < 8; i++) {
            Serial.print((inputs & (1 << i)) ? "1" : "0");
        }
        Serial.println("]");
        
        // Change LED color based on input state
        if (inputs > 0) {
            board.setRGBColor(255, 0, 0);  // Red = input detected
        } else {
            board.setRGBColor(0, 255, 0);  // Green = no input
        }
    }
    
    // Example: Toggle output 0 every 2 seconds
    static uint32_t toggleTimer = 0;
    if (millis() - toggleTimer > 2000) {
        toggleTimer = millis();
        
        // Read current state of output 0
        uint8_t currentState = board.getDigitalInput(0);
        
        // Toggle output 0
        board.setDigitalOutput(0, !currentState);
        
        // Beep feedback
        board.buzzerBeep(1, 50, 50);
        Serial.println("🔄 Toggled output 0");
    }
    
    delay(100);
}
