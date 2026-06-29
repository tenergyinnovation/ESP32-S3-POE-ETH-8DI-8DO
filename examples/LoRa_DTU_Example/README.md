# SX1262-LoRa-DTU-xF AT Command Example

This example demonstrates how to use the **SX1262-LoRa-DTU-xF** LoRa module with the **ESP32-S3-POE-ETH-8DI-8DO** board through RS485 communication using AT commands.

## 📋 Overview

The SX1262-LoRa-DTU-xF is a long-range wireless communication module that:
- Communicates via **RS485** half-duplex interface
- Operates at **9600 baud** (UART interface)
- Uses **AT commands** for configuration and operation
- Supports **Stream Mode** (continuous data) and **Packet Mode** (buffered data)
- Provides **30+ AT commands** for flexible configuration
- Range: **5km+** line-of-sight transmission

## 🔧 Hardware Connection

Connect the SX1262-LoRa-DTU-xF module to the ESP32-S3 board:

| LoRa Module | Pin | ESP32-S3 | Purpose |
|-------------|-----|----------|---------|
| GND | GND | GND | Ground |
| 5V | VCC | 5V | Power supply |
| A (RS485+) | TX | GPIO17 | UART2 TX |
| B (RS485-) | RX | GPIO18 | UART2 RX |
| DE/RE | - | GPIO21 | Direction control |

**Note:** This example uses **UART2** (GPIO17/GPIO18) configured for **RS485 half-duplex** mode.

## 🚀 Quick Start

### 1. **Open the Example**
   - In PlatformIO: `lib/ESP32-S3-POE-ETH-8DI-8DO/examples/LoRa_DTU_Example/LoRa_DTU_Example.ino`
   - Or copy to your `src/main.cpp`

### 2. **Upload to Board**
   ```bash
   platformio run --target upload
   ```

### 3. **Open Serial Monitor**
   - Baud rate: **115200 bps**
   - Launch monitor: `platformio device monitor`

### 4. **Start Testing**
   - Type `HELP` to see all available commands
   - Type `INIT` to initialize the module
   - Type `CONFIG` to configure Stream Mode (14 AT commands)
   - Type `AT+VER` to get firmware version

## 📝 Available Commands

### Configuration Commands
```
INIT              Initialize LoRa DTU (UART2 RS485)
CONFIG            Configure Stream Mode with 14 AT commands
STATUS            Display module status and statistics
```

### Direct AT Commands
```
AT+VER            Get firmware version
AT+SF=7           Set Spreading Factor (7-12: higher=longer range)
AT+BW=0           Set Bandwidth (0=125K, 1=250K, 2=500K)
AT+CR=1           Set Coding Rate (1-4: higher=more robust)
AT+PWR=22         Set TX Power (10-22 dBm: higher=longer range)
AT+MODE=1         Stream Mode (continuous data)
AT+MODE=2         Packet Mode (Modbus RTU compatible)
AT+TXCH=18        Set TX Channel (0-80: ≈868MHz at 18)
AT+RXCH=18        Set RX Channel (must match sender)
AT+ADDR=0         Set Device Address (0-65535)
AT+PORT=2         Set RS485 (2=RS485, default is RS232!)
AT+BAUD=9600      Set Baud Rate
```

### AT Mode Control
```
+++               Enter AT Command Mode
AT+EXIT           Exit AT Command Mode
HELP              Show help menu
```

## 🔌 GPIO Pin Configuration

```cpp
#define LORA_DTU_RX_PIN 18           // GPIO18 - RS485 RX (UART2 RX)
#define LORA_DTU_TX_PIN 17           // GPIO17 - RS485 TX (UART2 TX)
#define LORA_DTU_DE_RE_PIN 21        // GPIO21 - RS485 direction control (DE/RE)
#define LORA_DTU_BAUD 9600           // 9600 bps (MUST be 9600!)
```

## 📡 LoRa Configuration Parameters

The `loraConfigureStream()` function sets these parameters:

| Parameter | Value | Description |
|-----------|-------|-------------|
| Mode | 1 (Stream) | Continuous data transmission |
| Port | 2 (RS485) | RS485 interface (NOT RS232!) |
| Baud | 9600 | UART baud rate |
| Spreading Factor | 7 | Fast mode (higher=slower but longer range) |
| Bandwidth | 0 (125KHz) | Narrow band (lower=longer range) |
| Coding Rate | 1 (4/5) | Forward error correction ratio |
| TX Power | 22 dBm | Maximum power (higher=longer range) |
| TX Channel | 18 | ≈868MHz ISM band center |
| RX Channel | 18 | Receive on same channel as TX |
| Device Address | 0 | Receive from all addresses |

## ⚙️ Configuration Sequence

The example sends **14 AT commands** in sequence:

```
1. AT                  Test connectivity
2. AT+VER             Get firmware version
3. AT+PORT=2          Set RS485 (CRITICAL: default is RS232)
4. AT+BAUD=9600       Set baud rate to 9600
5. AT+COMM="8N1"      Set 8 bits, no parity, 1 stop bit
6. AT+MODE=1          Stream Mode (continuous data)
7. AT+TXCH=18         TX Channel 18 (≈868MHz)
8. AT+RXCH=18         RX Channel 18 (must match)
9. AT+ADDR=0          Device Address 0 (any sender)
10. AT+SF=7           Spreading Factor 7 (fast)
11. AT+BW=0           Bandwidth 125KHz (long range)
12. AT+CR=1           Coding Rate 1 (4/5)
13. AT+PWR=22         TX Power 22dBm (max)
14. AT+EXIT           Exit AT Command Mode
```

## 🧪 Testing the Module

### Step 1: Initialize Module
```
📝 > INIT
✅ LoRa DTU initialized successfully
   GPIO17=TX, GPIO18=RX, GPIO21=DE/RE, Baud=9600
```

### Step 2: Configure Stream Mode
```
📝 > CONFIG
⚙️  Configuring LoRa DTU for Stream Mode...
1️⃣  📤 Sending: AT
   📥 Response: OK
   ✅ Response: OK
[... 12 more commands ...]
✅ Stream Mode Configuration Complete!
```

### Step 3: Check Firmware Version
```
📝 > AT+VER
📤 Sending: AT+VER (Auto-entering AT Command Mode first)
   📥 Response: SX1262-LoRa-DTU-xF
   ✅ Device ID: SX1262-LoRa-DTU-xF
```

### Step 4: Verify Status
```
📝 > STATUS
📊 LoRa DTU Module Status:
───────────────────────────────────
  Connection:       ✅ Connected
  Packets Sent:     0
  Packets Received: 0
  Packet Errors:    0
  Mode:             Stream (Continuous)
  AT Mode Active:   No
───────────────────────────────────
```

## 🔧 Troubleshooting

### "No response from module (timeout)"
1. **Check power supply**: Module needs stable 5V
2. **Verify RS485 wiring**: A↔TX, B↔RX, GND↔GND
3. **Check baud rate**: Must be 9600 (use `AT+BAUD=9600`)
4. **Verify GPIO pins**: TX=17, RX=18, DE/RE=21
5. **Check AT+PORT**: Must be 2 (RS485, NOT RS232)

### Module responds with "ERROR"
- Check command syntax (case-sensitive)
- Verify parameter ranges (e.g., SF 7-12, PWR 10-22)
- Review [SX1262-LoRa-DTU-xF Wiki](https://www.waveshare.com/wiki/SX1262-LoRa-DTU-xF)

### Serial Monitor shows garbage characters
- Change baud rate to 115200 (for Serial, NOT LoRa DTU)
- LoRa module still communicates at 9600 baud

## 📚 LoRa Configuration Tips

### For Long Range (max distance)
```
AT+SF=12          Spreading Factor 12 (slowest, longest range)
AT+BW=0           Bandwidth 125KHz (narrowest)
AT+CR=4           Coding Rate 4 (most robust)
AT+PWR=22         Max power 22dBm
```

### For Fast Data (max speed)
```
AT+SF=7           Spreading Factor 7 (fastest)
AT+BW=2           Bandwidth 500KHz (widest)
AT+CR=1           Coding Rate 1 (least robust)
AT+PWR=22         Max power 22dBm
```

### For Balanced Performance
```
AT+SF=9           Medium Spreading Factor
AT+BW=0           125KHz Bandwidth
AT+CR=1           Coding Rate 1
AT+PWR=20         High Power (22dBm)
```

## 📖 Key Concepts

### **Stream Mode (AT+MODE=1)**
- Continuous data transmission
- Real-time data flow
- Best for sensor data streams
- Lower latency

### **Packet Mode (AT+MODE=2)**
- Modbus RTU compatible
- Buffered message protocol
- Best for discrete commands
- CRC error checking

### **RS485 Half-Duplex**
- Single twisted pair for TX/RX
- Automatic direction switching via GPIO21
- 300ms turnaround time for direction switch
- Saves pins and wiring

## 🔗 Resources

- [SX1262-LoRa-DTU-xF Wiki](https://www.waveshare.com/wiki/SX1262-LoRa-DTU-xF)
- [SX1262-LoRa-DTU-xF Product Page](https://www.waveshare.com/product/iot-communication/long-range-wireless/nb-iot-lora/sx1262-lora-dtu-lf.htm)
- [ESP32-S3-POE-ETH-8DI-8DO Wiki](https://www.waveshare.com/wiki/ESP32-S3-POE-ETH-8DI-8DO)
- [AT Command Reference](../../extras/AT_COMMAND.md)

## 📋 Example Output

```
╔══════════════════════════════════════════════════════════╗
║     SX1262-LoRa-DTU-xF AT Command Example                ║
║     Interactive LoRa module configuration tool            ║
╚══════════════════════════════════════════════════════════╝

🔧 Initializing LoRa DTU module...
✅ LoRa DTU initialized successfully
   GPIO17=TX, GPIO18=RX, GPIO21=DE/RE, Baud=9600

⚙️  Configuring LoRa DTU for Stream Mode...

═══════════════════════════════════════════════════════════
Configuring LoRa DTU for Stream Mode
═══════════════════════════════════════════════════════════

1️⃣  Entering AT Command Mode...
2️⃣  📤 Sending: AT
   📥 Response: OK\r\n
   ✅ Response: OK

3️⃣  📤 Sending: AT+VER
   📥 Response: SX1262-LoRa-DTU-xF\r\n
   ✅ Device ID: SX1262-LoRa-DTU-xF

[... more configuration steps ...]

═══════════════════════════════════════════════════════════
✅ Stream Mode Configuration Complete!
═══════════════════════════════════════════════════════════

╔═══════════════════════════════════════════════════════════╗
║  Available Commands (type in Serial Monitor)             ║
╚═══════════════════════════════════════════════════════════╝

🔹 CONFIGURATION:
  INIT              Initialize LoRa DTU module
  CONFIG            Configure Stream Mode (14 AT commands)
  STATUS            Show module status

🔹 AT COMMANDS (direct):
  AT+VER            Get firmware version
  AT+SF=7           Set Spreading Factor (7-12)
  ...

📝 > _
```

## 📝 License

This example is provided as part of the Tenergy Innovation project for the ESP32-S3-POE-ETH-8DI-8DO board.

---

**Version:** 1.0.0  
**Last Updated:** 29/06/2026  
**Author:** Uten Boonliam (Tenergy Innovation Co., Ltd.)
