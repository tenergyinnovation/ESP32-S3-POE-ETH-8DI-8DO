# LED Ticker Animation & Buzzer Methods Reference

## Quick Usage Guide for New ESP32S3_8DI8DO Methods

### Overview
The `ESP32S3_8DI8DO` board class now includes convenient methods for controlling LED blinking animations and buzzer sounds. These methods provide simple, timer-based control without requiring external libraries.

---

## LED Ticker Methods

### RGB LED Color Blinking Functions

All LED ticker methods use the same signature:
```cpp
void Tick[Color]LED(float seconds);
```

**Parameter:**
- `seconds` - Blink interval in seconds (e.g., 0.5 = 500ms on, 500ms off)

### Available Color Methods

| Method | Color | RGB Code | Usage |
|--------|-------|----------|-------|
| `TickRedLED()` | Red | 0xFF0000 | 🔴 Status/Error indication |
| `TickGreenLED()` | Green | 0x00FF00 | 🟢 Success/OK indication |
| `TickBlueLED()` | Blue | 0x0000FF | 🔵 Information/Status |
| `TickYellowLED()` | Yellow | 0xFFFF00 | 🟡 Warning indication |
| `TickPurpleLED()` | Purple | 0xFF00FF | 🟣 Custom status |
| `TickOrangeLED()` | Orange | 0xFF8000 | 🟠 Alert/Attention |
| `TickWhiteLED()` | White | 0xFFFFFF | ⚪ Full brightness |

### LED Method Examples

**Example 1: Simple Red LED Blink**
```cpp
void setup() {
    board.begin();
}

void loop() {
    board.loop();  // IMPORTANT: Must call loop() to update LED
    
    // Make LED blink red every 1 second
    board.TickRedLED(1.0);
}
```

**Example 2: Green LED Blink (Fast)**
```cpp
void loop() {
    board.loop();
    
    // Quick blink at 0.25s interval (on 125ms, off 125ms)
    board.TickGreenLED(0.25);
}
```

**Example 3: Status Indication with Different Colors**
```cpp
void displayStatus(int statusCode) {
    board.loop();
    
    switch (statusCode) {
        case 0:  // Error
            board.TickRedLED(0.5);      // Fast red blink
            break;
        case 1:  // Warning
            board.TickYellowLED(1.0);   // Slow yellow blink
            break;
        case 2:  // OK
            board.TickGreenLED(2.0);    // Slow green blink
            break;
    }
}
```

---

## Buzzer Methods

### Buzzer Control Functions

#### `buzzer_beep(int times)`
**Description:** Produces multiple beep sounds with 100ms intervals.

**Parameter:**
- `times` - Number of beeps (1-10 recommended)

**Usage Example:**
```cpp
// Single beep
board.buzzer_beep(1);

// Double beep (alert)
board.buzzer_beep(2);

// Triple beep (warning)
board.buzzer_beep(3);

// Multiple beeps
board.buzzer_beep(5);
```

#### `buzzerBeep(uint8_t times, uint16_t duration, uint16_t interval)` [Advanced]
**Description:** More detailed buzzer control with custom timing.

**Parameters:**
- `times` - Number of beeps
- `duration` - Each beep duration in milliseconds (default: 100)
- `interval` - Interval between beeps in milliseconds (default: 100)

**Usage Example:**
```cpp
// Quick beep (50ms on, 50ms off)
board.buzzerBeep(1, 50, 50);

// Long beep
board.buzzerBeep(1, 500, 500);

// Custom pattern
board.buzzerBeep(3, 100, 200);  // 100ms beep, 200ms silence, repeated 3 times
```

---

## Practical Usage Patterns

### Pattern 1: Initialization Feedback
```cpp
void setup() {
    Serial.begin(115200);
    
    if (board.begin() == ESP32S3_8DI8DO::STATUS_OK) {
        board.buzzer_beep(2);      // Double beep for success
        board.TickGreenLED(2.0);   // Green LED on
    } else {
        board.buzzer_beep(3);      // Triple beep for error
        board.TickRedLED(0.5);     // Fast red blink
    }
}
```

### Pattern 2: Event Notification
```cpp
void handleEvent(int eventType) {
    board.loop();
    
    if (eventType == EVENT_ERROR) {
        board.buzzer_beep(3);
        board.TickRedLED(0.5);
    } else if (eventType == EVENT_WARNING) {
        board.buzzer_beep(2);
        board.TickYellowLED(1.0);
    } else if (eventType == EVENT_SUCCESS) {
        board.buzzer_beep(1);
        board.TickGreenLED(2.0);
    }
}
```

### Pattern 3: Blinking Update Indicator
```cpp
void performLongOperation() {
    board.TickBlueLED(0.3);  // Quick blue blink during operation
    board.loop();
    
    // Do some work...
    delay(1000);
    
    board.loop();  // Keep updating LED
}
```

### Pattern 4: Alert Pattern
```cpp
void raiseAlert() {
    // Alert pattern: fast red blink + multiple beeps
    board.TickRedLED(0.2);   // Very fast red blink
    board.buzzer_beep(5);     // 5 beeps
    
    // Continue blinking in loop
    for (int i = 0; i < 10; i++) {
        board.loop();
        delay(100);
    }
}
```

---

## Important Notes

### ⚠️ Critical: Must Call `board.loop()`
The LED blinking animation requires calling `board.loop()` in your main loop function. Without this, the LED animations won't update:

```cpp
void loop() {
    board.loop();  // ← REQUIRED for LED animations to work
    
    // Your code here...
}
```

### LED Blink Timing
- The interval parameter is the **full blink cycle time** (on + off)
- For a 0.5s interval: 250ms ON, 250ms OFF (repeating)
- Shorter intervals (0.2-0.3s) create attention-grabbing effects
- Longer intervals (1.0-2.0s) are easier to read for status indication

### Buzzer Duration
- Default beep duration: 100ms
- Minimum: 50ms (barely audible)
- Maximum: 500ms+ (for alarms)
- Interval: Gap between consecutive beeps

---

## Color Combinations & Meanings

**Suggested Status Meanings:**
- 🔴 **RED** - Error/Fault/Alarm
- 🟢 **GREEN** - Success/OK/Ready
- 🔵 **BLUE** - Processing/Waiting/Information
- 🟡 **YELLOW** - Warning/Caution/Low Battery
- 🟣 **PURPLE** - Debug/Custom Status 1
- 🟠 **ORANGE** - Alert/Attention Required
- ⚪ **WHITE** - Full Power/All Subsystems OK

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| LED not blinking | Ensure `board.loop()` is called in main loop |
| Buzzer not working | Check buzzer pin (GPIO46) configuration |
| Irregular blinking | Verify `board.loop()` is called frequently enough |
| LEDs too dim | Use `board.getRGB().setBrightness()` for brightness control |

---

## Code Examples

See the included examples:
- `LED_Ticker_Demo.ino` - Complete LED and buzzer demonstration
- `00_ClassUsageExamples.ino` - Advanced usage patterns
- `BasicDigitalIO.ino` - Basic I/O control with feedback

---

*Version: 1.0.0 | Last Updated: 2026-06-28*
