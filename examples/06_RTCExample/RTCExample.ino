/***********************************************************************
 * File         :     RTCExample.ino
 * Description  :     Real-Time Clock (RTC) Example for ESP32-S3-POE-ETH-8DI-8DO
 * Author       :     Tenergy Innovation Co., Ltd.
 * Date         :     28 Jun 2026
 * Version      :     1.0.0
 * Email        :     uten.boonliam@tenergyinnovation.co.th
 * 
 * Features:
 *   - Set RTC time with custom date and time values
 *   - Read and display RTC time periodically in Serial Monitor
 *   - Display time in format: YYYY-MM-DD HH:MM:SS
 *   - Handle RTC operations with error checking
 * 
 * Board:       ESP32-S3-POE-ETH-8DI-8DO
 * Wiring:      Connect to USB for power and Serial communication
 * Baud Rate:   115200
 ***********************************************************************/

#include <ESP32S3_8DI8DO.h>

// ============================================================
// GLOBAL VARIABLES
// ============================================================

ESP32S3_8DI8DO board;                    // Create board instance
unsigned long lastDisplayTime = 0;       // Track last display time
const unsigned long DISPLAY_INTERVAL = 1000;  // Display RTC every 1 second

// ============================================================
// SETUP FUNCTION
// ============================================================

void setup() {
    // Initialize Serial communication
    Serial.begin(115200);
    delay(1000);  // Wait for Serial Monitor connection
    
    // Print header
    Serial.println();
    Serial.println("╔════════════════════════════════════════════════════╗");
    Serial.println("║     ESP32-S3-POE-ETH-8DI-8DO - RTC Example        ║");
    Serial.println("║          Real-Time Clock Test Demo v1.0           ║");
    Serial.println("╚════════════════════════════════════════════════════╝");
    Serial.println();
    
    // Initialize the board
    Serial.print("🔧 Initializing board... ");
    if (board.begin() == board.STATUS_OK) {
        Serial.println("✅ SUCCESS");
    } else {
        Serial.println("❌ FAILED");
        while (1) {
            delay(1000);
            Serial.println("⚠️  Board initialization failed. Please check wiring.");
        }
    }
    
    // Set RTC time
    // Example: Set to 2026-06-28 15:30:45
    Serial.println();
    Serial.println("📅 Setting RTC Time...");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    board.setRTC(2026, 6, 28, 15, 30, 45);
    delay(500);
    Serial.println();
    
    // Display initial RTC value
    Serial.println("📊 Current RTC Time:");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    String rtcTime = board.getRTC();
    Serial.printf("  🕐 Time: %s\n", rtcTime.c_str());
    Serial.println();
    
    // Show available commands
    printHelp();
    
    Serial.println();
    Serial.println("✅ Setup complete! RTC is running...");
    Serial.println();
}

// ============================================================
// MAIN LOOP
// ============================================================

void loop() {
    // Display RTC time periodically
    if (millis() - lastDisplayTime >= DISPLAY_INTERVAL) {
        lastDisplayTime = millis();
        
        String currentTime = board.getRTC();
        Serial.printf("⏰ RTC: %s\n", currentTime.c_str());
    }
    
    // Check for Serial input commands
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.length() > 0) {
            handleSerialCommand(command);
        }
    }
    
    delay(10);  // Small delay to prevent blocking
}

// ============================================================
// HELPER FUNCTIONS
// ============================================================

/***********************************************************************
 * FUNCTION:    handleSerialCommand
 * DESCRIPTION: Parse and execute Serial commands from user
 * PARAMETERS:  command - Command string entered by user
 * RETURNED:    None
 ***********************************************************************/
void handleSerialCommand(String command) {
    command.toUpperCase();
    
    if (command == "HELP") {
        printHelp();
    }
    else if (command == "NOW") {
        String currentTime = board.getRTC();
        Serial.println();
        Serial.println("📊 Current RTC Time:");
        Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
        Serial.printf("  🕐 %s\n", currentTime.c_str());
        Serial.println();
    }
    else if (command.startsWith("SET ")) {
        // Parse SET command: SET YYYY MM DD HH MM SS
        // Example: SET 2026 6 28 15 30 45
        handleSetCommand(command);
    }
    else if (command == "DEMO") {
        runRTCDemo();
    }
    else if (command == "CLEAR") {
        Serial.write(27);  // ESC
        Serial.print("[2J");  // Clear screen
        Serial.write(27);
        Serial.print("[H");  // Cursor home
        Serial.println("✅ Screen cleared!");
    }
    else {
        Serial.println();
        Serial.println("❌ Unknown command: " + command);
        Serial.println("   Type 'HELP' for available commands");
        Serial.println();
    }
}

/***********************************************************************
 * FUNCTION:    handleSetCommand
 * DESCRIPTION: Parse and execute SET command for RTC time
 * PARAMETERS:  command - SET command string with date/time parameters
 * RETURNED:    None
 ***********************************************************************/
void handleSetCommand(String command) {
    // Parse: "SET YYYY MM DD HH MM SS"
    // Remove "SET " prefix
    String params = command.substring(4);
    params.trim();
    
    // Extract parameters
    int year, month, day, hour, minute, second;
    int count = sscanf(params.c_str(), "%d %d %d %d %d %d", 
                       &year, &month, &day, &hour, &minute, &second);
    
    if (count != 6) {
        Serial.println();
        Serial.println("❌ Invalid SET command format!");
        Serial.println("   Usage: SET YYYY MM DD HH MM SS");
        Serial.println("   Example: SET 2026 6 28 15 30 45");
        Serial.println();
        return;
    }
    
    // Validate date/time values
    if (!isValidDateTime(year, month, day, hour, minute, second)) {
        Serial.println();
        Serial.println("❌ Invalid date/time values!");
        Serial.println("   Year:   2000-2099");
        Serial.println("   Month:  1-12");
        Serial.println("   Day:    1-31");
        Serial.println("   Hour:   0-23");
        Serial.println("   Minute: 0-59");
        Serial.println("   Second: 0-59");
        Serial.println();
        return;
    }
    
    // Set RTC
    Serial.println();
    board.setRTC(year, month, day, hour, minute, second);
    
    // Verify the set time
    delay(100);
    String newTime = board.getRTC();
    Serial.printf("✅ RTC updated to: %s\n", newTime.c_str());
    Serial.println();
}

/***********************************************************************
 * FUNCTION:    isValidDateTime
 * DESCRIPTION: Validate date and time values
 * PARAMETERS:  year, month, day, hour, minute, second
 * RETURNED:    true if valid, false otherwise
 ***********************************************************************/
bool isValidDateTime(int year, int month, int day, int hour, int minute, int second) {
    // Validate ranges
    if (year < 2000 || year > 2099) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;
    if (hour < 0 || hour > 23) return false;
    if (minute < 0 || minute > 59) return false;
    if (second < 0 || second > 59) return false;
    
    // Additional day validation for specific months
    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // Check for leap year
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        daysInMonth[2] = 29;
    }
    
    if (day > daysInMonth[month]) return false;
    
    return true;
}

/***********************************************************************
 * FUNCTION:    printHelp
 * DESCRIPTION: Display available commands in Serial Monitor
 * PARAMETERS:  None
 * RETURNED:    None
 ***********************************************************************/
void printHelp() {
    Serial.println();
    Serial.println("📋 Available Commands:");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println();
    Serial.println("  NOW");
    Serial.println("    • Display current RTC time immediately");
    Serial.println();
    Serial.println("  SET <YYYY> <MM> <DD> <HH> <MM> <SS>");
    Serial.println("    • Set RTC to specific date and time");
    Serial.println("    • Example: SET 2026 6 28 15 30 45");
    Serial.println();
    Serial.println("  DEMO");
    Serial.println("    • Run interactive RTC demonstration");
    Serial.println();
    Serial.println("  HELP");
    Serial.println("    • Display this help message");
    Serial.println();
    Serial.println("  CLEAR");
    Serial.println("    • Clear Serial Monitor screen");
    Serial.println();
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println();
}

/***********************************************************************
 * FUNCTION:    runRTCDemo
 * DESCRIPTION: Run interactive RTC demonstration
 * PARAMETERS:  None
 * RETURNED:    None
 ***********************************************************************/
void runRTCDemo() {
    Serial.println();
    Serial.println("═══════════════════════════════════════════════════");
    Serial.println("         🎯 RTC INTERACTIVE DEMO");
    Serial.println("═══════════════════════════════════════════════════");
    Serial.println();
    
    // Demo 1: Set time to specific date
    Serial.println("📝 Demo 1: Setting RTC to 2026-01-01 00:00:00");
    board.setRTC(2026, 1, 1, 0, 0, 0);
    delay(200);
    Serial.printf("   Result: %s\n", board.getRTC().c_str());
    delay(2000);
    
    // Demo 2: Set time to another date
    Serial.println();
    Serial.println("📝 Demo 2: Setting RTC to 2026-12-31 23:59:59");
    board.setRTC(2026, 12, 31, 23, 59, 59);
    delay(200);
    Serial.printf("   Result: %s\n", board.getRTC().c_str());
    delay(2000);
    
    // Demo 3: Set to current date
    Serial.println();
    Serial.println("📝 Demo 3: Setting RTC to today (2026-06-28 15:30:00)");
    board.setRTC(2026, 6, 28, 15, 30, 0);
    delay(200);
    Serial.printf("   Result: %s\n", board.getRTC().c_str());
    delay(1000);
    
    // Demo 4: Show time progression
    Serial.println();
    Serial.println("📝 Demo 4: Time progression (5 readings)");
    for (int i = 1; i <= 5; i++) {
        delay(1000);
        Serial.printf("   [%d] %s\n", i, board.getRTC().c_str());
    }
    
    Serial.println();
    Serial.println("═══════════════════════════════════════════════════");
    Serial.println("✅ Demo completed!");
    Serial.println();
}

// ============================================================
// END OF FILE
// ============================================================
