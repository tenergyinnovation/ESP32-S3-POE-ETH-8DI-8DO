/***********************************************************************
 * Project      :     SX1262-LoRa-DTU-xF AT Command Example
 * Description  :     Simple example for LoRa DTU module communication
 *                    using AT commands over RS485 interface
 * Hardware     :     ESP32-S3-POE-ETH-8DI-8DO + SX1262-LoRa-DTU-xF
 * Author       :     Uten Boonliam (Tenergy Innovation Co., Ltd.)
 * Date         :     29/06/2026
 * Revision     :     1.0.0
 * ***********************************************************************/

#include <Arduino.h>

/***********************************************************************
 * LoRa DTU GPIO DEFINITIONS
 * Connect SX1262-LoRa-DTU-xF RS485 to ESP32-S3 UART2
 ***********************************************************************/
#define LORA_DTU_RX_PIN 18           // GPIO18 - RS485 RX (UART2 RX)
#define LORA_DTU_TX_PIN 17           // GPIO17 - RS485 TX (UART2 TX)
#define LORA_DTU_DE_RE_PIN 21        // GPIO21 - RS485 direction control (DE/RE)
#define LORA_DTU_BAUD 9600           // LoRa DTU baud rate: 9600 bps (MUST be 9600)

// Timeout and buffer settings
#define LORA_RX_BUFFER_SIZE 256      // Buffer for receiving data from LoRa module
#define LORA_RX_TIMEOUT 100          // ms - timeout for complete reception
#define LORA_AT_MODE_TIMEOUT 2000    // ms - timeout for AT command response

/***********************************************************************
 * LoRa STATUS STRUCTURE
 ***********************************************************************/
struct LoRaStatus {
  bool isConnected;                  // Module connection state
  uint16_t packetsReceived;          // Statistics
  uint16_t packetsSent;
  uint16_t packetErrors;
  int16_t lastRSSI;                  // Received Signal Strength Indicator
  uint32_t lastActivityTime;         // Last activity timestamp
  char lastErrorMsg[100];            // Last error message
  bool streamMode;                   // Mode: true=Stream, false=Packet
};

/***********************************************************************
 * GLOBAL VARIABLES
 ***********************************************************************/
LoRaStatus loraStatus = {
  false,    // isConnected
  0,        // packetsReceived
  0,        // packetsSent
  0,        // packetErrors
  0,        // lastRSSI
  0,        // lastActivityTime
  "",       // lastErrorMsg
  true      // streamMode (Stream Mode is default)
};

char loraRxBuffer[LORA_RX_BUFFER_SIZE];
uint16_t loraRxIndex = 0;
uint32_t loraLastRxTime = 0;
bool atCommandModeActive = false;    // Track AT mode state

/***********************************************************************
 * FUNCTION PROTOTYPES
 ***********************************************************************/
void initLoRaDTU(void);              // Initialize UART2 for LoRa DTU
void loraATCommand(String cmd);      // Send AT command to LoRa DTU module
void loraConfigureStream(void);      // Configure module for Stream Mode
void printLoRaStatus(void);          // Display module status
void handleSerialInput(void);        // Handle user input from Serial Monitor
void printHelpMenu(void);            // Print help menu
void printPrompt(void);              // Print command prompt
void processCommand(String cmd);     // Process user command

/***********************************************************************
 * SETUP FUNCTION
 ***********************************************************************/
void setup() {
    // Initialize Serial for USB-CDC communication (115200 baud)
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n╔══════════════════════════════════════════════════════════╗");
    Serial.println("║     SX1262-LoRa-DTU-xF AT Command Example                ║");
    Serial.println("║     Interactive LoRa module configuration tool            ║");
    Serial.println("╚══════════════════════════════════════════════════════════╝\n");
    
    // Initialize LoRa DTU module (UART2 RS485)
    Serial.println("🔧 Initializing LoRa DTU module...");
    initLoRaDTU();
    
    // Configure module for Stream Mode
    Serial.println("⚙️  Configuring LoRa DTU for Stream Mode...");
    delay(500);
    loraConfigureStream();
    delay(500);
    
    // Display help menu
    printHelpMenu();
    printPrompt();
}

/***********************************************************************
 * MAIN LOOP FUNCTION
 ***********************************************************************/
void loop() {
    // Handle user input from Serial Monitor
    handleSerialInput();
    
    delay(10);  // Small delay to prevent overwhelming CPU
}

/***********************************************************************
 * INITIALIZE LoRa DTU MODULE
 * 
 * Configures UART2 with RS485 half-duplex mode
 * - GPIO17: UART2 TX (output to RS485 driver)
 * - GPIO18: UART2 RX (input from RS485 driver)
 * - GPIO21: RS485 direction control (DE/RE pin)
 ***********************************************************************/
void initLoRaDTU(void) {
    // Configure GPIO for RS485 direction control (output)
    pinMode(LORA_DTU_DE_RE_PIN, OUTPUT);
    digitalWrite(LORA_DTU_DE_RE_PIN, LOW);  // Set to receive mode initially
    
    // Initialize Serial2 (UART2) with specified pins and baud rate
    // Parameters: baud rate, config, RX pin, TX pin
    Serial2.begin(LORA_DTU_BAUD, SERIAL_8N1, LORA_DTU_RX_PIN, LORA_DTU_TX_PIN);
    
    // Configure for RS485 half-duplex mode (automatic direction control)
    Serial2.setPins(-1, -1, -1, LORA_DTU_DE_RE_PIN);
    Serial2.setMode(UART_MODE_RS485_HALF_DUPLEX);
    
    // Clear any pending data in receive buffer
    while (Serial2.available()) {
        Serial2.read();
    }
    
    loraRxIndex = 0;
    loraStatus.isConnected = true;
    
    Serial.println("✅ LoRa DTU initialized successfully");
    Serial.println("   GPIO17=TX, GPIO18=RX, GPIO21=DE/RE, Baud=9600");
}

/***********************************************************************
 * SEND AT COMMAND TO LoRa DTU MODULE
 * 
 * Sends an AT command and receives the response
 * - Adds \r\n terminator if not present
 * - Waits 300ms for RS485 turnaround time
 * - Times out after 2 seconds if no response
 * - Detects completion by 200ms of silence
 ***********************************************************************/
void loraATCommand(String cmd) {
    // Ensure command has line ending
    if (!cmd.endsWith("\r\n")) {
        cmd += "\r\n";
    }
    
    // Clear receive buffer
    loraRxIndex = 0;
    memset(loraRxBuffer, 0, LORA_RX_BUFFER_SIZE);
    
    // Send command via Serial2 (LoRa DTU UART)
    Serial.print("📤 Sending: ");
    Serial.println(cmd.c_str());
    
    Serial2.print(cmd);
    Serial2.flush();
    
    // Wait for RS485 turnaround time (300ms for half-duplex)
    delay(300);
    
    // Receive response with timeout handling
    uint32_t startTime = millis();
    uint32_t lastByteTime = millis();
    bool dataReceived = false;
    
    while (millis() - startTime < LORA_AT_MODE_TIMEOUT) {
        if (Serial2.available()) {
            char ch = Serial2.read();
            loraRxBuffer[loraRxIndex++] = ch;
            lastByteTime = millis();
            dataReceived = true;
            
            // Exit early if complete line received
            if (ch == '\n') {
                break;
            }
            
            // Prevent buffer overflow
            if (loraRxIndex >= LORA_RX_BUFFER_SIZE - 1) {
                break;
            }
        }
        
        // Timeout if 200ms of silence after receiving data
        if (dataReceived && (millis() - lastByteTime > 200)) {
            break;
        }
    }
    
    // Null terminate the response buffer
    loraRxBuffer[loraRxIndex] = '\0';
    
    // Display response
    if (loraRxIndex > 0) {
        Serial.print("   📥 Response: ");
        
        // Print response with hex codes for non-printable characters
        for (uint16_t i = 0; i < loraRxIndex; i++) {
            char ch = loraRxBuffer[i];
            if (ch >= 32 && ch < 127) {
                Serial.print(ch);
            } else if (ch == '\r') {
                Serial.print("\\r");
            } else if (ch == '\n') {
                Serial.print("\\n");
            } else {
                Serial.printf("[0x%02X]", ch);
            }
        }
        Serial.println();
        
        // Parse response
        String response = String(loraRxBuffer);
        response.toUpperCase();
        
        if (response.indexOf("OK") >= 0) {
            Serial.println("   ✅ Response: OK");
            loraStatus.lastActivityTime = millis();
        } else if (response.indexOf("ERROR") >= 0) {
            Serial.println("   ❌ Response: ERROR");
            loraStatus.packetErrors++;
        } else if (response.indexOf("SX1262") >= 0 || response.indexOf("XF") >= 0) {
            Serial.print("   ✅ Device ID: ");
            Serial.println(loraRxBuffer);
            loraStatus.lastActivityTime = millis();
        }
    } else {
        Serial.println("   ❌ No response from module (timeout)");
        Serial.println("   ⚠️  Check: Wiring, power supply, baud rate (must be 9600)");
        loraStatus.packetErrors++;
    }
    
    Serial.println();
}

/***********************************************************************
 * CONFIGURE LoRa DTU FOR STREAM MODE
 * 
 * Sends a sequence of AT commands to configure the module:
 * 1. Enter AT Command Mode (+++), wait 500ms
 * 2. Test connectivity (AT)
 * 3. Get firmware version (AT+VER)
 * 4. Set port to RS485 (AT+PORT=2) - CRITICAL!
 * 5. Set baud rate (AT+BAUD=9600)
 * 6. Set communication format (AT+COMM="8N1")
 * 7. Set mode to Stream (AT+MODE=1)
 * 8. Set TX channel (AT+TXCH=18)
 * 9. Set RX channel (AT+RXCH=18)
 * 10. Set device address (AT+ADDR=0)
 * 11. Set spreading factor (AT+SF=7)
 * 12. Set bandwidth (AT+BW=0)
 * 13. Set coding rate (AT+CR=1)
 * 14. Set TX power (AT+PWR=22)
 * 15. Exit AT Command Mode (AT+EXIT)
 ***********************************************************************/
void loraConfigureStream(void) {
    Serial.println("\n═══════════════════════════════════════════════════════════");
    Serial.println("Configuring LoRa DTU for Stream Mode");
    Serial.println("═══════════════════════════════════════════════════════════\n");
    
    // Enter AT Command Mode
    Serial.println("1️⃣  Entering AT Command Mode...");
    Serial2.print("+++\r\n");
    Serial2.flush();
    delay(500);  // Wait for mode switch
    atCommandModeActive = true;
    
    // Clear any pending data
    while (Serial2.available()) {
        Serial2.read();
    }
    
    delay(100);
    
    // Configuration commands sequence
    String commands[] = {
        "AT",                    // Test connectivity
        "AT+VER",               // Get firmware version
        "AT+PORT=2",            // Set RS485 (CRITICAL: default is RS232)
        "AT+BAUD=9600",         // Set baud rate
        "AT+COMM=\"8N1\"",      // 8 data bits, No parity, 1 stop bit
        "AT+MODE=1",            // Stream Mode (continuous data)
        "AT+TXCH=18",           // TX channel (0-80, 18 ≈ 868MHz)
        "AT+RXCH=18",           // RX channel (must match TX for duplex)
        "AT+ADDR=0",            // Device address (0 = receive from all)
        "AT+SF=7",              // Spreading Factor 7 (range: 7-12, higher=longer range)
        "AT+BW=0",              // Bandwidth 0=125KHz (0=125K, 1=250K, 2=500K)
        "AT+CR=1",              // Coding Rate 1 (range: 1-4)
        "AT+PWR=22",            // TX Power 22dBm (max, range: 10-22dBm)
        "AT+EXIT"               // Exit AT Command Mode
    };
    
    int commandCount = sizeof(commands) / sizeof(commands[0]);
    
    // Send each configuration command
    for (int i = 0; i < commandCount; i++) {
        Serial.printf("%d️⃣  ", i + 2);
        loraATCommand(commands[i]);
        
        if (i < commandCount - 1) {  // Small delay between commands
            delay(100);
        }
    }
    
    atCommandModeActive = false;
    loraStatus.streamMode = true;
    
    Serial.println("═══════════════════════════════════════════════════════════");
    Serial.println("✅ Stream Mode Configuration Complete!");
    Serial.println("═══════════════════════════════════════════════════════════\n");
}

/***********************************************************************
 * PRINT LoRa MODULE STATUS
 ***********************************************************************/
void printLoRaStatus(void) {
    Serial.println("\n📊 LoRa DTU Module Status:");
    Serial.println("─────────────────────────────────────");
    Serial.printf("  Connection:      %s\n", loraStatus.isConnected ? "✅ Connected" : "❌ Disconnected");
    Serial.printf("  Packets Sent:    %u\n", loraStatus.packetsSent);
    Serial.printf("  Packets Received:%u\n", loraStatus.packetsReceived);
    Serial.printf("  Packet Errors:   %u\n", loraStatus.packetErrors);
    Serial.printf("  Mode:            %s\n", loraStatus.streamMode ? "Stream (Continuous)" : "Packet (Buffered)");
    Serial.printf("  Last RSSI:       %d dBm\n", loraStatus.lastRSSI);
    Serial.printf("  AT Mode Active:  %s\n", atCommandModeActive ? "Yes" : "No");
    if (strlen(loraStatus.lastErrorMsg) > 0) {
        Serial.printf("  Last Error:      %s\n", loraStatus.lastErrorMsg);
    }
    Serial.println("─────────────────────────────────────\n");
}

/***********************************************************************
 * PRINT HELP MENU
 ***********************************************************************/
void printHelpMenu(void) {
    Serial.println("\n╔═══════════════════════════════════════════════════════════╗");
    Serial.println("║  Available Commands (type in Serial Monitor)             ║");
    Serial.println("╚═══════════════════════════════════════════════════════════╝\n");
    
    Serial.println("🔹 CONFIGURATION:");
    Serial.println("  INIT              Initialize LoRa DTU module");
    Serial.println("  CONFIG            Configure Stream Mode (14 AT commands)");
    Serial.println("  STATUS            Show module status\n");
    
    Serial.println("🔹 AT COMMANDS (direct):");
    Serial.println("  AT+VER            Get firmware version");
    Serial.println("  AT+SF=7           Set Spreading Factor (7-12)");
    Serial.println("  AT+BW=0           Set Bandwidth (0=125K, 1=250K, 2=500K)");
    Serial.println("  AT+PWR=22         Set TX Power (10-22 dBm)");
    Serial.println("  AT+MODE=1         Set Stream Mode");
    Serial.println("  AT+MODE=2         Set Packet Mode");
    Serial.println("  AT+TXCH=18        Set TX Channel (0-80)");
    Serial.println("  AT+RXCH=18        Set RX Channel (0-80)");
    Serial.println("  AT+ADDR=0         Set Device Address (0-65535)\n");
    
    Serial.println("🔹 AT MODE:");
    Serial.println("  +++               Enter AT Command Mode");
    Serial.println("  AT+EXIT           Exit AT Command Mode\n");
    
    Serial.println("🔹 HELP:");
    Serial.println("  HELP              Show this help menu\n");
    
    Serial.println("═════════════════════════════════════════════════════════════\n");
}

/***********************************************************************
 * PRINT COMMAND PROMPT
 ***********************************************************************/
void printPrompt(void) {
    if (atCommandModeActive) {
        Serial.print("📝 LoRa> ");
    } else {
        Serial.print("📝 > ");
    }
}

/***********************************************************************
 * HANDLE SERIAL INPUT FROM USER
 ***********************************************************************/
void handleSerialInput(void) {
    static String inputBuffer = "";
    
    if (Serial.available() > 0) {
        char ch = Serial.read();
        
        // Echo character to user
        Serial.write(ch);
        
        if (ch == '\n' || ch == '\r') {
            // Process command
            if (inputBuffer.length() > 0) {
                Serial.println();
                processCommand(inputBuffer);
                inputBuffer = "";
            }
            printPrompt();
        } else if (ch == '\b' || ch == 127) {
            // Handle backspace
            if (inputBuffer.length() > 0) {
                inputBuffer.remove(inputBuffer.length() - 1);
                Serial.print(" \b");
            }
        } else if (ch >= 32 && ch < 127) {
            // Printable character
            inputBuffer += ch;
        }
    }
}

/***********************************************************************
 * PROCESS USER COMMAND
 ***********************************************************************/
void processCommand(String cmd) {
    cmd.toUpperCase();
    cmd.trim();
    
    if (cmd.length() == 0) {
        return;
    }
    
    Serial.println();
    
    if (cmd == "INIT") {
        Serial.println("📡 Initializing LoRa DTU...");
        initLoRaDTU();
    }
    else if (cmd == "CONFIG") {
        Serial.println("⚙️  Configuring LoRa DTU for Stream Mode...");
        loraConfigureStream();
    }
    else if (cmd == "STATUS") {
        printLoRaStatus();
    }
    else if (cmd == "+++") {
        Serial.println("📤 Entering AT Command Mode...");
        Serial2.print("+++\r\n");
        delay(500);
        atCommandModeActive = true;
    }
    else if (cmd == "AT+EXIT") {
        Serial.println("📤 Exiting AT Command Mode...");
        loraATCommand("AT+EXIT");
        atCommandModeActive = false;
    }
    else if (cmd.startsWith("AT")) {
        // Send AT command to module
        if (!atCommandModeActive) {
            Serial.println("Auto-entering AT Command Mode first...");
            Serial2.print("+++\r\n");
            delay(500);
            atCommandModeActive = true;
        }
        loraATCommand(cmd);
    }
    else if (cmd == "HELP") {
        printHelpMenu();
    }
    else {
        Serial.print("❌ Unknown command: ");
        Serial.println(cmd);
        Serial.println("   Type 'HELP' for available commands\n");
    }
    
    Serial.println();
}
