/**
 * File: Hardware_Diagnostics_LED_Buzzer.ino
 * Description: Comprehensive test program for RGB LED and Buzzer functionality
 * Purpose: Verify each LED color and buzzer independently before full system tests
 * Created: 28 Jun 2026
 * 
 * Test Sequence:
 * 1. Test each LED color individually (solid on, 2 seconds each)
 * 2. Test LED blinking (0.5s on/off)
 * 3. Test buzzer with different beep patterns
 * 4. Cycle through all tests continuously
 */

#include <Arduino.h>
#include <ESP32S3_8DI8DO.h>

// Create board instance
ESP32S3_8DI8DO board;

// Test mode enumeration
enum TestMode {
    TEST_OFF = 0,           // All off
    TEST_RED_SOLID = 1,     // Red LED solid on
    TEST_GREEN_SOLID = 2,   // Green LED solid on
    TEST_BLUE_SOLID = 3,    // Blue LED solid on
    TEST_YELLOW_SOLID = 4,  // Yellow LED solid on
    TEST_PURPLE_SOLID = 5,  // Purple LED solid on
    TEST_ORANGE_SOLID = 6,  // Orange LED solid on
    TEST_WHITE_SOLID = 7,   // White LED solid on
    TEST_RED_BLINK = 8,     // Red LED blinking (0.5s)
    TEST_BUZZER_1 = 9,      // Single beep
    TEST_BUZZER_2 = 10,     // Double beep
    TEST_BUZZER_3 = 11,     // Triple beep
    TEST_BUZZER_5 = 12      // 5 beeps
};

// Test configuration
const int TOTAL_TESTS = 12;
const unsigned long TEST_DURATION_MS = 3000;  // 3 seconds per test
unsigned long testStartTime = 0;
int currentTest = TEST_OFF;

// Forward declarations
void printTestMenu();
void runDiagnosticTest(int testMode);
void printTestHeader(int testMode);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("╔═══════════════════════════════════════════════════════════╗");
    Serial.println("║                                                           ║");
    Serial.println("║    ESP32-S3-POE-ETH-8DI-8DO                              ║");
    Serial.println("║    RGB LED & BUZZER HARDWARE DIAGNOSTICS TEST             ║");
    Serial.println("║                                                           ║");
    Serial.println("║    Version 1.0 - NeoPixel GRB Color Order Fix             ║");
    Serial.println("╚═══════════════════════════════════════════════════════════╝\n");
    
    // Initialize board
    Serial.println("[INIT] Initializing ESP32S3_8DI8DO board...");
    if (!board.begin()) {
        Serial.println("❌ FAILED: Board initialization error!");
        while (1) {
            delay(100);
        }
    }
    Serial.println("✅ Board initialized successfully\n");
    
    // Print test information
    printTestMenu();
    
    testStartTime = millis();
}

void loop() {
    // Call board loop to update LED animations
    board.loop();
    
    // Check if test duration has elapsed
    unsigned long elapsed = millis() - testStartTime;
    if (elapsed >= TEST_DURATION_MS) {
        currentTest++;
        if (currentTest > TEST_BUZZER_5) {
            currentTest = TEST_OFF;
        }
        testStartTime = millis();
    }
    
    // Execute current test
    runDiagnosticTest(currentTest);
    
    delay(50);  // Update every 50ms for smooth LED animations
}

/**
 * Print the test menu and sequence
 */
void printTestMenu() {
    Serial.println("TEST SEQUENCE (Each test runs for 3 seconds):\n");
    Serial.println("┌─────────────────────────────────────────────────────┐");
    Serial.println("│ Test  │ Description              │ Expected Output   │");
    Serial.println("├─────────────────────────────────────────────────────┤");
    Serial.println("│  0    │ All OFF                  │ LED off, silent   │");
    Serial.println("│  1    │ Red LED (solid)          │ 🔴 Red glow       │");
    Serial.println("│  2    │ Green LED (solid)        │ 🟢 Green glow     │");
    Serial.println("│  3    │ Blue LED (solid)         │ 🔵 Blue glow      │");
    Serial.println("│  4    │ Yellow LED (solid)       │ 🟡 Yellow glow    │");
    Serial.println("│  5    │ Purple LED (solid)       │ 🟣 Purple glow    │");
    Serial.println("│  6    │ Orange LED (solid)       │ 🟠 Orange glow    │");
    Serial.println("│  7    │ White LED (solid)        │ ⚪ White glow     │");
    Serial.println("│  8    │ Red LED (blinking)       │ 🔴 Blink on/off   │");
    Serial.println("│  9    │ Buzzer (1 beep)          │ 🔊 Single beep    │");
    Serial.println("│ 10    │ Buzzer (2 beeps)         │ 🔊🔊 Two beeps    │");
    Serial.println("│ 11    │ Buzzer (3 beeps)         │ 🔊🔊🔊 3 beeps   │");
    Serial.println("│ 12    │ Buzzer (5 beeps)         │ 🔊 Five beeps     │");
    Serial.println("└─────────────────────────────────────────────────────┘\n");
    
    Serial.println("TROUBLESHOOTING GUIDE:");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    Serial.println("If LED does NOT change color:");
    Serial.println("  1. Check GPIO38 is connected to WS2812B NeoPixel LED");
    Serial.println("  2. Verify 5V power supply to LED (not GPIO!!)");
    Serial.println("  3. Check ground connection between board and LED");
    Serial.println("  4. LED may be defective - test with known good LED");
    Serial.println("  5. Verify Adafruit_NeoPixel library is installed\n");
    
    Serial.println("If Buzzer does NOT make sound:");
    Serial.println("  1. Check GPIO46 is connected to buzzer positive pin");
    Serial.println("  2. Verify buzzer GND connected to board GND");
    Serial.println("  3. Test buzzer with direct 3.3V power first");
    Serial.println("  4. Buzzer may require different PWM frequency");
    Serial.println("  5. Check PWM channel configuration (LEDC channel)\n");
    
    Serial.println("Starting diagnostic tests...\n");
    Serial.println("═══════════════════════════════════════════════════════\n");
}

/**
 * Run individual diagnostic test
 */
void runDiagnosticTest(int testMode) {
    static int lastTest = -1;
    
    // Print test header when test changes
    if (testMode != lastTest) {
        printTestHeader(testMode);
        lastTest = testMode;
        
        // Turn off everything first
        board.getRGB().setColor(0, 0, 0);
        board.getBuzzer().stop();
    }
    
    // Execute test
    switch (testMode) {
        case TEST_OFF:
            board.getRGB().setColor(0, 0, 0);
            board.getBuzzer().stop();
            break;
            
        case TEST_RED_SOLID:
            board.getRGB().setColor(255, 0, 0);  // Red
            break;
            
        case TEST_GREEN_SOLID:
            board.getRGB().setColor(0, 255, 0);  // Green
            break;
            
        case TEST_BLUE_SOLID:
            board.getRGB().setColor(0, 0, 255);  // Blue
            break;
            
        case TEST_YELLOW_SOLID:
            board.getRGB().setColor(255, 255, 0);  // Yellow
            break;
            
        case TEST_PURPLE_SOLID:
            board.getRGB().setColor(255, 0, 255);  // Purple
            break;
            
        case TEST_ORANGE_SOLID:
            board.getRGB().setColor(255, 128, 0);  // Orange
            break;
            
        case TEST_WHITE_SOLID:
            board.getRGB().setColor(255, 255, 255);  // White
            break;
            
        case TEST_RED_BLINK:
            // Start blinking if not already
            if (millis() - testStartTime < 100) {  // First 100ms of test
                board.getRGB().blink(0xFF0000, 500, 500);  // 0.5s on/off
            }
            break;
            
        case TEST_BUZZER_1:
            // Single beep
            if (millis() - testStartTime < 200) {  // First 200ms
                board.buzzer_beep(1);
            }
            break;
            
        case TEST_BUZZER_2:
            // Double beep
            if (millis() - testStartTime < 400) {  // First 400ms
                board.buzzer_beep(2);
            }
            break;
            
        case TEST_BUZZER_3:
            // Triple beep
            if (millis() - testStartTime < 600) {  // First 600ms
                board.buzzer_beep(3);
            }
            break;
            
        case TEST_BUZZER_5:
            // Five beeps
            if (millis() - testStartTime < 1000) {  // First 1 second
                board.buzzer_beep(5);
            }
            break;
    }
}

/**
 * Print test header information
 */
void printTestHeader(int testMode) {
    Serial.print("\n▶ TEST ");
    Serial.print(testMode);
    Serial.print("/12 ");
    
    // Print progress bar
    Serial.print("  [");
    for (int i = 0; i < 12; i++) {
        if (i < testMode) Serial.print("█");
        else Serial.print("░");
    }
    Serial.println("]");
    
    // Print test description
    switch (testMode) {
        case TEST_OFF:
            Serial.println("╔═══════════════════════════════════════╗");
            Serial.println("║ Status: All OFF                       ║");
            Serial.println("║ Expected: LED off, buzzer silent      ║");
            Serial.println("╚═══════════════════════════════════════╝");
            break;
            
        case TEST_RED_SOLID:
            Serial.println("╔═══════════════════════════════════════╗");
            Serial.println("║ 🔴 Red LED - Solid Color              ║");
            Serial.println("║ Expected: Bright RED glow (3 seconds) ║");
            Serial.println("╚═══════════════════════════════════════╝");
            break;
            
        case TEST_GREEN_SOLID:
            Serial.println("╔═══════════════════════════════════════╗");
            Serial.println("║ 🟢 Green LED - Solid Color            ║");
            Serial.println("║ Expected: Bright GREEN glow           ║");
            Serial.println("╚═══════════════════════════════════════╝");
            break;
            
        case TEST_BLUE_SOLID:
            Serial.println("╔═══════════════════════════════════════╗");
            Serial.println("║ 🔵 Blue LED - Solid Color             ║");
            Serial.println("║ Expected: Bright BLUE glow            ║");
            Serial.println("╚═══════════════════════════════════════╝");
            break;
            
        case TEST_YELLOW_SOLID:
            Serial.println("╔═══════════════════════════════════════╗");
            Serial.println("║ 🟡 Yellow LED - Solid Color           ║");
            Serial.println("║ Expected: Bright YELLOW glow          ║");
            Serial.println("╚═══════════════════════════════════════╝");
            break;
            
        case TEST_PURPLE_SOLID:
            Serial.println("╔═══════════════════════════════════════╗");
            Serial.println("║ 🟣 Purple LED - Solid Color           ║");
            Serial.println("║ Expected: Bright PURPLE glow          ║");
            Serial.println("╚═══════════════════════════════════════╝");
            break;
            
        case TEST_ORANGE_SOLID:
            Serial.println("╔═══════════════════════════════════════╗");
            Serial.println("║ 🟠 Orange LED - Solid Color           ║");
            Serial.println("║ Expected: Bright ORANGE glow          ║");
            Serial.println("╚═══════════════════════════════════════╝");
            break;
            
        case TEST_WHITE_SOLID:
            Serial.println("╔═══════════════════════════════════════╗");
            Serial.println("║ ⚪ White LED - Solid Color            ║");
            Serial.println("║ Expected: Bright WHITE glow           ║");
            Serial.println("╚═══════════════════════════════════════╝");
            break;
            
        case TEST_RED_BLINK:
            Serial.println("╔═══════════════════════════════════════╗");
            Serial.println("║ 🔴 Red LED - Blinking                 ║");
            Serial.println("║ Expected: ON 0.5s, OFF 0.5s (repeat)  ║");
            Serial.println("╚═══════════════════════════════════════╝");
            break;
            
        case TEST_BUZZER_1:
            Serial.println("╔═══════════════════════════════════════╗");
            Serial.println("║ 🔊 Buzzer - Single Beep               ║");
            Serial.println("║ Expected: One short beep (at start)   ║");
            Serial.println("╚═══════════════════════════════════════╝");
            break;
            
        case TEST_BUZZER_2:
            Serial.println("╔═══════════════════════════════════════╗");
            Serial.println("║ 🔊 Buzzer - Double Beep               ║");
            Serial.println("║ Expected: Two short beeps (at start)  ║");
            Serial.println("╚═══════════════════════════════════════╝");
            break;
            
        case TEST_BUZZER_3:
            Serial.println("╔═══════════════════════════════════════╗");
            Serial.println("║ 🔊 Buzzer - Triple Beep               ║");
            Serial.println("║ Expected: Three short beeps           ║");
            Serial.println("╚═══════════════════════════════════════╝");
            break;
            
        case TEST_BUZZER_5:
            Serial.println("╔═══════════════════════════════════════╗");
            Serial.println("║ 🔊 Buzzer - Five Beeps                ║");
            Serial.println("║ Expected: Five short beeps            ║");
            Serial.println("╚═══════════════════════════════════════╝");
            break;
    }
    
    Serial.print("⏱  Duration: 3000ms");
    Serial.println("  [Running...]");
}
