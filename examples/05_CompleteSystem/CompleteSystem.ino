/**
 * @file CompleteSystem.ino
 * @brief Complete system integration example
 * 
 * This example demonstrates:
 * - Full board initialization (DI/DO, RS485, RGB, Buzzer)
 * - Multi-tasking using FreeRTOS
 * - Data collection from multiple sources
 * - Status indication via LED and buzzer
 * - Serial command interface
 */

#include <ESP32S3_8DI8DO.h>

ESP32S3_8DI8DO board;

// System state
typedef struct {
    uint8_t diState;
    uint8_t doState;
    uint32_t messageCount;
    uint32_t errors;
    uint32_t uptime;
} SystemState;

SystemState sysState = {0, 0, 0, 0, 0};

// Task handles
TaskHandle_t taskMonitorHandle = NULL;
TaskHandle_t taskCommunicationHandle = NULL;
TaskHandle_t taskLEDStatusHandle = NULL;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n╔════════════════════════════════════╗");
    Serial.println("║  ESP32-S3-POE-ETH-8DI-8DO System  ║");
    Serial.println("║       Complete Integration         ║");
    Serial.println("╚════════════════════════════════════╝\n");
    
    // Initialize main board
    Serial.print("Initializing board...");
    ESP32S3_8DI8DO::Status status = board.begin();
    
    if (status != ESP32S3_8DI8DO::STATUS_OK) {
        Serial.println(" ✗ FAILED");
        board.setRGB(255, 0, 0);
        board.buzzerOn();
        return;
    }
    Serial.println(" ✓ OK");
    
    // Initialize RS485
    Serial.print("Initializing RS485...");
    status = board.initRS485(9600);
    if (status == ESP32S3_8DI8DO::STATUS_OK) {
        Serial.println(" ✓ OK");
    } else {
        Serial.println(" ✗ FAILED");
    }
    
    // Enable debouncing
    board.enableDIDebounc(20);
    
    // Set initial status
    board.setRGB(255, 255, 0);  // Yellow = Initializing
    board.beep(100);
    
    // Create background tasks
    Serial.println("\nCreating tasks...");
    
    xTaskCreatePinnedToCore(
        taskMonitorInputs,
        "MonitorInputs",
        2048,
        NULL,
        1,
        &taskMonitorHandle,
        0
    );
    
    xTaskCreatePinnedToCore(
        taskHandleCommunication,
        "Communication",
        2048,
        NULL,
        1,
        &taskCommunicationHandle,
        1
    );
    
    xTaskCreatePinnedToCore(
        taskUpdateLEDStatus,
        "LEDStatus",
        1024,
        NULL,
        0,
        &taskLEDStatusHandle,
        0
    );
    
    Serial.println("✓ All tasks created");
    Serial.println("\n┌─────────────────────────────────┐");
    Serial.println("│ System ready! Commands:         │");
    Serial.println("│ 'S' - System status             │");
    Serial.println("│ '1-8' - Toggle DO               │");
    Serial.println("│ 'A' - All outputs ON            │");
    Serial.println("│ 'O' - All outputs OFF           │");
    Serial.println("│ 'T' - Send test LoRa data       │");
    Serial.println("│ 'R' - Reset system              │");
    Serial.println("└─────────────────────────────────┘\n");
    
    board.setRGB(0, 255, 0);  // Green = Ready
}

void loop() {
    board.loop();
    
    // Handle serial commands
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        handleSerialCommand(cmd);
    }
    
    // Update system state
    sysState.uptime = board.getUptime();
    
    delay(100);
}

/**
 * Task: Monitor digital inputs
 */
void taskMonitorInputs(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    
    static uint8_t lastState = 0;
    
    while (1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        uint8_t currentState = 0;
        for (uint8_t ch = 1; ch <= 8; ch++) {
            if (board.readDI(ch)) {
                currentState |= (1 << (ch - 1));
            }
        }
        
        if (currentState != lastState) {
            lastState = currentState;
            sysState.diState = currentState;
            
            Serial.print("📥 DI Changed: ");
            for (uint8_t i = 0; i < 8; i++) {
                if (currentState & (1 << i)) {
                    Serial.print("DI");
                    Serial.print(i + 1);
                    Serial.print(" ");
                }
            }
            Serial.println();
        }
    }
}

/**
 * Task: Handle RS485 communication
 */
void taskHandleCommunication(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(200);
    
    while (1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        // Check for incoming data
        if (board.availableRS485() > 0) {
            uint8_t buffer[256];
            size_t len = board.readRS485(buffer, sizeof(buffer));
            
            if (len > 0) {
                sysState.messageCount++;
                Serial.print("📨 RS485 Message (");
                Serial.print(len);
                Serial.print(" bytes): ");
                Serial.write(buffer, len);
                Serial.println();
            }
        }
    }
}

/**
 * Task: Update LED status indicator
 */
void taskUpdateLEDStatus(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000);
    
    while (1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        // Pulse green LED to indicate system is running
        board.setRGB(0, 200, 0);
        delay(100);
        board.setRGB(0, 255, 0);
    }
}

/**
 * Handle serial commands
 */
void handleSerialCommand(char cmd) {
    cmd = toupper(cmd);
    
    if (cmd >= '1' && cmd <= '8') {
        // Toggle DO
        uint8_t ch = cmd - '0';
        board.toggleDO(ch);
        sysState.doState ^= (1 << (ch - 1));
        
        Serial.print("✓ DO");
        Serial.print(ch);
        Serial.print(" = ");
        Serial.println(board.getDOState(ch) ? "ON" : "OFF");
        
    } else if (cmd == 'A') {
        // All ON
        for (uint8_t ch = 1; ch <= 8; ch++) {
            board.turnOnDO(ch);
        }
        sysState.doState = 0xFF;
        Serial.println("✓ All outputs ON");
        board.setRGB(0, 255, 0);
        board.beep(100);
        
    } else if (cmd == 'O') {
        // All OFF
        for (uint8_t ch = 1; ch <= 8; ch++) {
            board.turnOffDO(ch);
        }
        sysState.doState = 0;
        Serial.println("✓ All outputs OFF");
        board.setRGB(255, 255, 0);
        board.beep(100);
        
    } else if (cmd == 'S') {
        // Print system status
        printSystemStatus();
        
    } else if (cmd == 'T') {
        // Send test LoRa data
        sendTestData();
        
    } else if (cmd == 'R') {
        // Reset system
        Serial.println("\n🔄 Resetting system...");
        board.softReset();
        
    } else {
        Serial.println("? Unknown command");
    }
}

/**
 * Print detailed system status
 */
void printSystemStatus() {
    Serial.println("\n╔═══════════════════════════════════╗");
    Serial.println("║      System Status Report         ║");
    Serial.println("╠═══════════════════════════════════╣");
    
    // Digital Inputs
    Serial.println("║ Digital Inputs:                   ║");
    for (uint8_t ch = 1; ch <= 8; ch++) {
        Serial.print("║   DI");
        Serial.print(ch);
        Serial.print(": ");
        Serial.print(board.readDI(ch) ? "HIGH ✓" : "LOW  ✗");
        Serial.println("                 ║");
    }
    
    // Digital Outputs
    Serial.println("║ Digital Outputs:                  ║");
    for (uint8_t ch = 1; ch <= 8; ch++) {
        Serial.print("║   DO");
        Serial.print(ch);
        Serial.print(": ");
        Serial.print(board.getDOState(ch) ? "ON ✓ " : "OFF ✗");
        Serial.println("                  ║");
    }
    
    // System Statistics
    Serial.println("║ System Statistics:                ║");
    Serial.print("║   Uptime: ");
    Serial.print(sysState.uptime);
    Serial.println(" seconds              ║");
    
    Serial.print("║   Messages: ");
    Serial.print(sysState.messageCount);
    Serial.println("                        ║");
    
    Serial.print("║   Free RAM: ");
    Serial.print(board.getFreeMemory() / 1024);
    Serial.println(" KB                     ║");
    
    Serial.println("╚═══════════════════════════════════╝\n");
}

/**
 * Send test data via RS485
 */
void sendTestData() {
    const char* testMsg = "TEST_DATA";
    board.sendRS485((uint8_t*)testMsg, strlen(testMsg));
    
    Serial.print("📤 Sent: ");
    Serial.println(testMsg);
    board.beep(150);
}
