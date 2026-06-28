/**
 * ============================================================
 * LED Ticker Animation & Buzzer Demo
 * ============================================================
 * Description: Demonstrates the new TickRedLED, TickGreenLED, 
 *              TickBlueLED, and buzzer_beep methods for quick 
 *              LED blinking and sound control.
 * 
 * Hardware: ESP32-S3-POE-ETH-8DI-8DO
 * ============================================================
 */

#include <Arduino.h>
#include <ESP32S3_8DI8DO.h>

// Create board instance
ESP32S3_8DI8DO board;

// Demo mode counter
int demoMode = 0;
unsigned long lastModeChangeTime = 0;
const unsigned long MODE_CHANGE_INTERVAL = 5000;  // Change demo every 5 seconds

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║  LED Ticker & Buzzer Demo              ║");
    Serial.println("║  ESP32-S3-POE-ETH-8DI-8DO Board        ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println();
    
    // Initialize board
    Serial.println("[1] Initializing board...");
    ESP32S3_8DI8DO::Status status = board.begin();
    
    if (status != ESP32S3_8DI8DO::STATUS_OK) {
        Serial.println("❌ Board initialization failed!");
        while (1) delay(1000);
    }
    
    Serial.println("✅ Board initialized successfully!");
    Serial.println();
    Serial.println("Demo sequence:");
    Serial.println("  • RED LED blinking (0.5s interval)");
    Serial.println("  • GREEN LED blinking (0.5s interval)");
    Serial.println("  • BLUE LED blinking (0.5s interval)");
    Serial.println("  • YELLOW LED blinking (0.5s interval)");
    Serial.println("  • PURPLE LED blinking (0.5s interval)");
    Serial.println("  • ORANGE LED blinking (0.5s interval)");
    Serial.println("  • WHITE LED blinking (0.5s interval)");
    Serial.println("  • Multiple buzzer beeps");
    Serial.println();
}

void loop() {
    // Update board (required for LED animations)
    board.loop();
    
    // Check if it's time to change demo mode
    if (millis() - lastModeChangeTime >= MODE_CHANGE_INTERVAL) {
        lastModeChangeTime = millis();
        demoMode++;
        if (demoMode > 8) demoMode = 0;
        
        runDemo(demoMode);
    }
}

void runDemo(int mode) {
    Serial.print("📝 Demo Mode ");
    Serial.print(mode + 1);
    Serial.println("/9");
    Serial.println();
    
    switch (mode) {
        case 0:
            Serial.println("🔴 RED LED - Blinking at 0.5s interval");
            board.TickRedLED(0.5);
            break;
            
        case 1:
            Serial.println("🟢 GREEN LED - Blinking at 0.5s interval");
            board.TickGreenLED(0.5);
            break;
            
        case 2:
            Serial.println("🔵 BLUE LED - Blinking at 0.5s interval");
            board.TickBlueLED(0.5);
            break;
            
        case 3:
            Serial.println("🟡 YELLOW LED - Blinking at 0.5s interval");
            board.TickYellowLED(0.5);
            break;
            
        case 4:
            Serial.println("🟣 PURPLE LED - Blinking at 0.5s interval");
            board.TickPurpleLED(0.5);
            break;
            
        case 5:
            Serial.println("🟠 ORANGE LED - Blinking at 0.5s interval");
            board.TickOrangeLED(0.5);
            break;
            
        case 6:
            Serial.println("⚪ WHITE LED - Blinking at 0.5s interval");
            board.TickWhiteLED(0.5);
            break;
            
        case 7:
            Serial.println("🔊 BUZZER - 3 rapid beeps");
            board.buzzer_beep(3);
            board.LEDOff();  // Turn off LED
            break;
            
        case 8:
            Serial.println("🔊 BUZZER - 5 rapid beeps");
            board.buzzer_beep(5);
            board.LEDOff();  // Turn off LED
            break;
    }
    Serial.println();
}
