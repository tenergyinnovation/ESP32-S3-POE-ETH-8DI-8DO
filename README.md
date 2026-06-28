# ESP32-S3-POE-ETH-8DI-8DO Board Library

Professional Arduino library for the **Waveshare ESP32-S3-POE-ETH-8DI-8DO** industrial-grade board, featuring comprehensive support for Ethernet, RS485/RS422/RS232 communication, CAN bus, and isolated digital I/O.

![ESP32-S3-POE-ETH-8DI-8DO](https://www.waveshare.com/img/devkit/accBoard/ESP32-S3-POE-ETH-8DI-8DO/ESP32-S3-POE-ETH-8DI-8DO-details-1.jpg)

## Features

### Core Features
- **ESP32-S3 Microcontroller** with dual-core processor (240 MHz)
- **WiFi 6E (802.11ax)** connectivity
- **Bluetooth 5.3** LE/Classic
- **Gigabit Ethernet** (integrated MAC + external PHY)
- **PoE Support** (Power over Ethernet)

### Communication Interfaces
- **RS485/RS422/RS232** industrial serial interface
- **CAN 2.0B** interface
- **LoRa Support** via external DTU module
- **MQTT** client built-in

### Digital I/O
- **8x Digital Inputs** with optocoupler isolation (3-30V, 5mA)
- **8x Digital Outputs** (relay contacts, 2A @ 30V DC)
- **Isolated Design** for noise immunity and safety

### Additional Features
- **RGB LED** status indicator
- **Buzzer** output for audio alerts
- **microSD Card Slot** for logging
- **Backup Battery Support** (RTC with battery connector)
- **Industrial Operating Temperature** (-20°C to +60°C)

## Board Pinout

### UART/Serial Pins
- `GPIO17` - RS485/RS422/RS232 TX
- `GPIO18` - RS485/RS422/RS232 RX

### CAN Bus Pins
- `GPIO41` - CAN RX
- `GPIO42` - CAN TX

### LED & Buzzer
- `GPIO48` - RGB LED (NeoPixel)
- `GPIO21` - Buzzer output

### Digital I/O
- `DIN[0-7]` - Digital Input channels 0-7
- `DOUT[0-7]` - Digital Output channels 0-7

## Installation

### Arduino IDE
1. Download the library as ZIP
2. Sketch → Include Library → Add .ZIP Library
3. Select the downloaded ZIP file

### PlatformIO
Add to `platformio.ini`:
```ini
lib_deps = 
    ESP32-S3-POE-ETH-8DI-8DO Board Library
```

## Quick Start

### Basic Initialization
```cpp
#include <ESP32S3_8DI8DO.h>

ESP32S3_8DI8DO board;

void setup() {
    Serial.begin(115200);
    board.begin();  // Initialize board
    
    // Set LED to green
    board.setRGBColor(0, 255, 0);
    
    // Beep buzzer 3 times
    board.buzzerBeep(3, 100, 100);
}

void loop() {
    // Read digital input 0
    uint8_t input = board.getDigitalInput(0);
    
    // Set digital output 0
    board.setDigitalOutput(0, HIGH);
    
    delay(1000);
}
```

### RS485 Communication
```cpp
#include <ESP32S3_8DI8DO.h>

ESP32S3_8DI8DO board;

void setup() {
    Serial.begin(115200);
    board.begin();
    board.setupRS485(9600);  // Initialize RS485 at 9600 baud
}

void loop() {
    // Send data via RS485
    if (Serial2.availableForWrite()) {
        Serial2.println("AT");
    }
    
    // Receive data from RS485
    if (Serial2.available()) {
        String response = Serial2.readStringUntil('\n');
        Serial.println(response);
    }
    
    delay(100);
}
```

### Ethernet Connection
```cpp
#include <ESP32S3_8DI8DO.h>

ESP32S3_8DI8DO board;

void setup() {
    Serial.begin(115200);
    board.begin();
    board.setupEthernet();  // Initialize Ethernet
}

void loop() {
    // Add your Ethernet code here
}
```

## API Reference

### Initialization
- `bool begin()` - Initialize board and all peripherals
- `String getBoardInfo()` - Get board information and status

### LED Control
- `void setRGBColor(uint8_t red, uint8_t green, uint8_t blue)` - Set LED color (0-255)
- `void LEDOff()` - Turn off LED

### Digital I/O
- `void setDigitalOutput(uint8_t channel, uint8_t state)` - Set output (channel 0-7, state HIGH/LOW)
- `uint8_t getDigitalInput(uint8_t channel)` - Read input (channel 0-7)
- `uint8_t readAllInputs()` - Read all 8 inputs at once (returns 8-bit value)
- `void setAllOutputs(uint8_t value)` - Set all 8 outputs at once

### Buzzer
- `void buzzerBeep(uint8_t times=1, uint16_t duration=100, uint16_t interval=100)` - Control buzzer

### Communication Setup
- `bool setupEthernet()` - Initialize Ethernet interface
- `bool setupRS485(uint32_t baudRate=9600)` - Setup RS485 communication
- `bool setupLoRaDTU(uint32_t baudRate=9600)` - Setup LoRa DTU module
- `bool setupCAN(uint32_t baudRate=500)` - Setup CAN 2.0 interface

## Examples

The library includes comprehensive examples:
- **BasicDigitalIO** - Digital input/output control
- **EthernetDemo** - Ethernet connectivity
- **RS485Modbus** - RS485 Modbus communication
- **CANBusDemo** - CAN bus communication

## Technical Specifications

| Parameter | Value |
|-----------|-------|
| Microcontroller | ESP32-S3 (Dual-core 240MHz) |
| Flash | 16MB (configurable) |
| SRAM | 8MB |
| WiFi | 802.11 a/b/g/n/ax |
| Ethernet | 10/100/1000 Mbps |
| Input Voltage | 5V DC or PoE 48V |
| Operating Temp | -20°C to +60°C |
| DIN Voltage Range | 3V to 30V DC |
| DOUT Contact Rating | 2A @ 30V DC |
| Isolation | Optocoupler 3kV |

## Pin Definitions (Adjustable)

Default GPIO assignments (may vary by board version):

```
Digital Inputs (8):   GPIO1, GPIO2, GPIO42, GPIO41, GPIO40, GPIO39, GPIO38, GPIO37
Digital Outputs (8):  GPIO10, GPIO11, GPIO12, GPIO13, GPIO14, GPIO15, GPIO16, GPIO9
RS485 TX:             GPIO17
RS485 RX:             GPIO18
CAN TX:               GPIO42
CAN RX:               GPIO41
RGB LED:              GPIO48
Buzzer:               GPIO21
```

## Troubleshooting

### No Serial Output
- Ensure HWCDC is enabled in board settings
- Check USB connection and drivers
- Try holding BOOT button while uploading

### RS485 Communication Issues
- Verify correct TX/RX pin connections
- Check baud rate matches device (9600 typical for DTU)
- Ensure 120Ω termination resistors on RS485 bus
- Confirm power supply (5V for module)

### Ethernet Connection Issues
- Verify PoE injector or Ethernet power source
- Check cable quality and connections
- Confirm network settings

## References

- [Official Waveshare Wiki](https://www.waveshare.com/wiki/ESP32-S3-POE-ETH-8DI-8DO)
- [Product Page](https://www.waveshare.com/esp32-s3-poe-eth-8di-8do.htm)
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)

## License

MIT License - See LICENSE file for details

## Support

For issues, questions, or contributions:
- GitHub Issues: [Report Bug](https://github.com/tenergyinnovation/ESP32-S3-POE-ETH-8DI-8DO_template)
- Email: tenergy.innovation@gmail.com

## Version History

### v1.0.0 (2026-06-28)
- Initial release
- Core board initialization
- Digital I/O control
- LED and Buzzer control
- RS485 support
- Ethernet foundation
- CAN bus foundation

---

**Made with ❤️ by Tenergy Innovation**
