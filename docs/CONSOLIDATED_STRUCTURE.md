# Library Structure & Class Access Guide

## 📦 Consolidated Library Structure

```
lib/ESP32-S3-POE-ETH-8DI-8DO/src/
├── ESP32S3_8DI8DO.h       ← Header with ALL class definitions
└── ESP32S3_8DI8DO.cpp     ← Implementation of ALL classes
```

**All 5 classes defined in ONE header file:**
- `ESP32S3_DI`
- `ESP32S3_DO`
- `ESP32S3_RS485`
- `ESP32S3_RGB`
- `ESP32S3_Buzzer`

---

## 🎯 How to Access Each Class

### Simple Answer:
```cpp
#include <ESP32S3_8DI8DO.h>          // Include once = get all classes!

ESP32S3_8DI8DO board;

void setup() {
    board.begin();
}

void loop() {
    // Access each subsystem:
    board.getDI().read(0);           // Digital Input
    board.getDO().on(0);             // Digital Output
    board.getRS485().writeString("AT"); // RS485
    board.getRGB().setColor(255,0,0);// RGB LED
    board.getBuzzer().beep(100);     // Buzzer
}
```

---

## 📋 Class Methods Summary

### 1️⃣ DI (Digital Input)
```cpp
board.getDI().read(channel);         // Read single channel
board.getDI().readAll();             // Read all 8 at once
board.getDI().getState(channel);     // Get last state
board.getDI().enableDebounce(ch, ms);// Anti-bounce
board.getDI().update();              // Call in loop!
```

### 2️⃣ DO (Digital Output)
```cpp
board.getDO().on(channel);           // Turn ON relay
board.getDO().off(channel);          // Turn OFF relay
board.getDO().toggle(channel);       // Toggle relay
board.getDO().allOn();               // All ON
board.getDO().allOff();              // All OFF
board.getDO().setMask(0b10101010);   // Pattern
```

### 3️⃣ RS485 (Serial Communication)
```cpp
String resp = board.getRS485().sendATCommand("AT", 1000);
board.getRS485().writeString("Hello\r\n");
board.getRS485().write(data, length);
if (board.getRS485().available()) { }
board.getRS485().update();           // Call in loop!
```

### 4️⃣ RGB (LED)
```cpp
board.getRGB().setColor(255, 0, 0);  // Red
board.getRGB().blink(0xFF0000, 500, 500);   // Blink
board.getRGB().pulse(0x00FF00, 2000);       // Pulse
board.getRGB().setBrightness(128);   // Brightness
board.getRGB().stop();               // Stop animation
board.getRGB().update();             // Call in loop!
```

### 5️⃣ Buzzer (Speaker)
```cpp
board.getBuzzer().beep(100);         // Simple beep
board.getBuzzer().setVolume(255);    // Set volume
board.getBuzzer().setFrequency(440); // Note frequency
board.getBuzzer().playPattern(PATTERN_SHORT);
board.getBuzzer().update();          // Call in loop!
```

---

## ✨ Key Differences from Old Structure

| Aspect | Old Way | New Way (Consolidated) |
|--------|---------|----------------------|
| **Include files** | 6 separate includes | Single `#include <ESP32S3_8DI8DO.h>` |
| **Files to manage** | Multiple .h/.cpp files | Just 2 files (h + cpp) |
| **Complexity** | Need to link subsystems | Automatic via main class |
| **Learning curve** | Harder (multiple files) | Simpler (unified approach) |
| **Compilation** | More overhead | Faster, cleaner |

---

## 🔄 Typical Usage Pattern

```cpp
#include <ESP32S3_8DI8DO.h>

ESP32S3_8DI8DO board;

void setup() {
    Serial.begin(115200);
    board.begin();  // Initialize everything at once
}

void loop() {
    // 1. Check inputs
    if (board.getDI().read(0) == HIGH) {
        Serial.println("Input 0 is HIGH");
        
        // 2. Control outputs
        board.getDO().on(0);
        
        // 3. Indicate with LED
        board.getRGB().setColor(0, 255, 0);  // Green
        
        // 4. Sound buzzer
        board.getBuzzer().beep(100);
        
        // 5. Send via RS485
        board.getRS485().writeString("Triggered\r\n");
    }
    
    // 6. Update all subsystems (IMPORTANT!)
    board.getDI().update();
    board.getRGB().update();
    board.getBuzzer().update();
    
    delay(50);
}
```

---

## 📚 File Locations

**Usage Examples:** `lib/ESP32-S3-POE-ETH-8DI-8DO/examples/00_ClassUsageExamples/`

**Reference Guide:** `lib/ESP32-S3-POE-ETH-8DI-8DO/docs/CLASS_USAGE_GUIDE.md`

**Full Documentation:** `lib/ESP32-S3-POE-ETH-8DI-8DO/README.md`

---

## ✅ Benefits of Consolidation

1. **Single include** → Less confusion, fewer mistakes
2. **Unified interface** → All classes accessed through `board` object
3. **Easier to maintain** → Changes in one place affect all
4. **Cleaner code** → Less boilerplate, more readable
5. **Better for beginners** → Simple, consistent API

จบ! ตอนนี้คุณสามารถใช้ library ได้ด้วยไฟล์เพียง 2 ไฟล์เท่านั้น 🎉
