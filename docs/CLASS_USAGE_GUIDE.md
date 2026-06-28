# Class Usage Quick Reference Guide

## Overview
ทั้ง 5 classes (`ESP32S3_DI`, `ESP32S3_DO`, `ESP32S3_RS485`, `ESP32S3_RGB`, `ESP32S3_Buzzer`) ถูก **consolidate** เข้าไปใน:
- **Header:** `ESP32S3_8DI8DO.h`
- **Implementation:** `ESP32S3_8DI8DO.cpp`

เมื่อ include ไฟล์เดียว (`#include <ESP32S3_8DI8DO.h>`) คุณจะได้ classes ทั้งหมด

---

## วิธีการเข้าถึง Subsystems

### วิธี 1: ผ่าน Main Board Instance (แนะนำ)
```cpp
#include <ESP32S3_8DI8DO.h>

ESP32S3_8DI8DO board;

void setup() {
    board.begin();
    
    // เข้าถึง subsystems ผ่าน getter methods:
    board.getDI();       // -> ESP32S3_DI&
    board.getDO();       // -> ESP32S3_DO&
    board.getRS485();    // -> ESP32S3_RS485&
    board.getRGB();      // -> ESP32S3_RGB&
    board.getBuzzer();   // -> ESP32S3_Buzzer&
}
```

### วิธี 2: Reference Style (More Convenient)
```cpp
void setup() {
    board.begin();
}

void loop() {
    // Get reference once
    ESP32S3_DI& di = board.getDI();
    ESP32S3_DO& do_ctrl = board.getDO();
    ESP32S3_RS485& rs485 = board.getRS485();
    ESP32S3_RGB& rgb = board.getRGB();
    ESP32S3_Buzzer& buzzer = board.getBuzzer();
    
    // Now use them:
    bool input0 = di.read(0);
    do_ctrl.on(1);
    rgb.setColor(255, 0, 0);
    buzzer.beep(100);
}
```

---

## Class Details & Methods

### 📥 **1. ESP32S3_DI (Digital Input - 8 Channels)**

```cpp
ESP32S3_DI& di = board.getDI();

// Read single channel (0-7)
bool ch0 = di.read(0);           // Returns: true/false

// Read all channels at once
uint8_t all = di.readAll();      // Returns: 0x00-0xFF (bitmask)

// Get cached state (after update())
bool state = di.getState(0);

// Debounce (ms)
di.enableDebounce(0, 20);        // Channel 0, 20ms debounce
di.disableDebounce(0);

// Input mode
di.setInputMode(0, ESP32S3_DI::INPUT_INVERTED);

// Callback for state changes
void myCallback(uint8_t ch, bool newState) { }
di.setChangeCallback(0, myCallback);

// IMPORTANT: Call update() in main loop
di.update();
```

**GPIO Pins:** 4, 5, 6, 7, 8, 9, 10, 11

---

### 📤 **2. ESP32S3_DO (Digital Output - 8 Relays via TCA9554)**

```cpp
ESP32S3_DO& do_ctrl = board.getDO();

// Control individual relay
do_ctrl.on(0);                   // Relay 1 ON
do_ctrl.off(0);                  // Relay 1 OFF
do_ctrl.toggle(0);               // Toggle relay 1

// Check state
bool isOn = do_ctrl.getState(0);

// Control all with bitmask (0=ON, 1=OFF)
do_ctrl.setMask(0b10101010);     // Pattern: all alternating

// Quick on/off all
do_ctrl.allOn();
do_ctrl.allOff();

// I2C address: 0x20
// Check connection:
bool connected = do_ctrl.isConnected();
```

**I2C Configuration:** GPIO41(SCL), GPIO42(SDA), Address 0x20

---

### 📡 **3. ESP32S3_RS485 (Serial Communication)**

```cpp
ESP32S3_RS485& rs485 = board.getRS485();

// Initialize (already done in board.begin())
rs485.begin(9600, ESP32S3_RS485::MODE_AT_COMMAND);

// Send AT command and wait for response
String resp = rs485.sendATCommand("AT+VER", 1000);  // 1s timeout

// Send raw data
uint8_t data[] = {0x01, 0x03, 0x00, 0x00};
rs485.write(data, 4);

// Send string
rs485.writeString("Hello\r\n");

// Read response
uint8_t buffer[256];
size_t bytes_read = rs485.read(buffer, 256);

// Check available data
if (rs485.available()) {
    int byte = rs485.peek();  // Peek without removing
}

// Change mode/baud
rs485.setCommMode(ESP32S3_RS485::MODE_STREAM);
rs485.changeBaudrate(115200);

// Statistics
rs485.getBytesSent();
rs485.getBytesReceived();
rs485.resetStats();
```

**UART Configuration:** GPIO17(TX), GPIO18(RX), GPIO21(RTS)

---

### 🌈 **4. ESP32S3_RGB (LED Status Indicator)**

```cpp
ESP32S3_RGB& rgb = board.getRGB();

// Solid colors (R, G, B: 0-255)
rgb.setColor(255, 0, 0);         // Red
rgb.setColor(0, 255, 0);         // Green
rgb.setColor(0, 0, 255);         // Blue
rgb.setColor(255, 255, 0);       // Yellow

// Predefined status colors
rgb.setStatusColor(ESP32S3_RGB::COLOR_RED);
rgb.setStatusColor(ESP32S3_RGB::COLOR_GREEN);
rgb.setStatusColor(ESP32S3_RGB::COLOR_YELLOW);

// Brightness (0-255)
rgb.setBrightness(128);          // 50%

// Animations
rgb.blink(0xFF0000, 500, 500);   // Red, 500ms on/off
rgb.pulse(0x00FF00, 2000);       // Green pulse, 2s period
rgb.strobe(0x0000FF, 10);        // Blue strobe, speed 10
rgb.breathe(0xFFFFFF, 3000);     // White breathe, 3s period

// Stop animation
rgb.stop();

// Turn off
rgb.setColor(0, 0, 0);

// IMPORTANT: Call update() in main loop
rgb.update();
```

**GPIO Pin:** 38 (NeoPixel protocol)

---

### 🔊 **5. ESP32S3_Buzzer (PWM Speaker Control)**

```cpp
ESP32S3_Buzzer& buzzer = board.getBuzzer();

// Simple beep
buzzer.beep(100);                // 100ms beep at full volume
buzzer.beep(200, 128);           // 200ms at 50% volume

// Control volume
buzzer.setVolume(255);           // 100%
buzzer.setVolume(128);           // 50%
buzzer.on();                     // Full volume continuously
buzzer.off();                    // Silence

// Patterns
buzzer.playPattern(ESP32S3_Buzzer::PATTERN_SHORT);
buzzer.playPattern(ESP32S3_Buzzer::PATTERN_LONG);
buzzer.playPattern(ESP32S3_Buzzer::PATTERN_ALARM);

// Frequency (musical notes)
buzzer.setFrequency(buzzer.FREQ_C4);  // 262 Hz
buzzer.setFrequency(buzzer.FREQ_A4);  // 440 Hz
buzzer.setFrequency(buzzer.FREQ_G4);  // 392 Hz

// Preset frequencies:
// FREQ_C4 (262), FREQ_D4 (294), FREQ_E4 (330), FREQ_F4 (349)
// FREQ_G4 (392), FREQ_A4 (440), FREQ_B4 (494)

// Status
if (buzzer.isPlaying()) { }
String status = buzzer.getStatus();

// IMPORTANT: Call update() in main loop
buzzer.update();
```

**GPIO Pin:** 46 (PWM output, 1kHz default frequency)

---

## Main Board Control Methods

```cpp
ESP32S3_8DI8DO board;

// Initialize (returns Status)
auto status = board.begin();

// RGB LED quick control
board.setRGBColor(255, 0, 0);    // Set red
board.LEDOff();                  // Turn off

// Digital I/O quick control
board.setDigitalOutput(0, HIGH);  // DO1 on
board.getDigitalInput(0);         // Read DI1
board.readAllInputs();            // Read all DI
board.setAllOutputs(0xFF);        // All DO on

// Buzzer quick control
board.buzzerBeep(3, 200, 100);   // 3x beeps: 200ms on, 100ms interval

// System setup (placeholders - expand as needed)
board.setupEthernet();
board.setupRS485(9600);
board.setupLoRaDTU(9600);
board.setupCAN(500);

// Get info
String info = board.getBoardInfo();

// Call in main loop
board.loop();
```

---

## Complete Example

```cpp
#include <ESP32S3_8DI8DO.h>

ESP32S3_8DI8DO board;

void setup() {
    Serial.begin(115200);
    board.begin();
}

void loop() {
    // Get subsystem references
    auto& di = board.getDI();
    auto& do_ctrl = board.getDO();
    auto& rgb = board.getRGB();
    auto& buzzer = board.getBuzzer();
    
    // Check if input 0 is high
    if (di.read(0)) {
        // Turn on relay 0 and set LED green
        do_ctrl.on(0);
        rgb.setColor(0, 255, 0);
        buzzer.beep(50);
    } else {
        // Turn off and set LED red
        do_ctrl.off(0);
        rgb.setColor(255, 0, 0);
    }
    
    // Update all subsystems (important!)
    di.update();
    rgb.update();
    buzzer.update();
    
    delay(50);
}
```

---

## Key Points

✓ **Single include:** `#include <ESP32S3_8DI8DO.h>`  
✓ **Access via:** `board.getDI()`, `board.getDO()`, etc.  
✓ **Call update():** DI, RGB, and Buzzer need `update()` in loop  
✓ **GPIO verified:** All pins match Waveshare datasheet  
✓ **I2C for DO:** Uses TCA9554 expander @ 0x20  
✓ **RS485 ready:** Can talk to LoRa DTU modules  

---

เพิ่มเติม: ดูไฟล์ `ClassUsageExamples.ino` ในโฟลเดอร์ examples/ สำหรับตัวอย่างที่ครบถ้วน!
