#ifndef ESP32S3_8DI8DO_H
#define ESP32S3_8DI8DO_H

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

/**
 * @class ESP32S3_8DI8DO
 * @brief Library for controlling Waveshare ESP32-S3-POE-ETH-8DI-8DO board
 * 
 * This board features:
 * - WiFi & Bluetooth connectivity via ESP32-S3
 * - Gigabit Ethernet (integrated MAC + PHY)
 * - RS485/RS422/RS232 industrial interface
 * - CAN 2.0 interface
 * - 8x Digital Inputs with optocoupler isolation
 * - 8x Digital Outputs (relays) with optocoupler isolation
 * - RGB LED indicator
 * - Buzzer output
 * - microSD card slot
 * - Backup battery connector (RTC)
 */

class ESP32S3_8DI8DO {
public:
    // Constructor
    ESP32S3_8DI8DO();
    
    /**
     * @brief Initialize the board with default settings
     * @return true if successful, false otherwise
     */
    bool begin();
    
    /**
     * @brief Set RGB LED color
     * @param red   Red component (0-255)
     * @param green Green component (0-255)
     * @param blue  Blue component (0-255)
     */
    void setRGBColor(uint8_t red, uint8_t green, uint8_t blue);
    
    /**
     * @brief Turn off RGB LED
     */
    void LEDOff();
    
    /**
     * @brief Set digital output (relay)
     * @param channel Output channel (0-7)
     * @param state   HIGH or LOW
     */
    void setDigitalOutput(uint8_t channel, uint8_t state);
    
    /**
     * @brief Get digital input value
     * @param channel Input channel (0-7)
     * @return Input state (HIGH or LOW)
     */
    uint8_t getDigitalInput(uint8_t channel);
    
    /**
     * @brief Read all digital inputs at once
     * @return 8-bit value representing all 8 input channels
     */
    uint8_t readAllInputs();
    
    /**
     * @brief Set all digital outputs at once
     * @param value 8-bit value to set all outputs
     */
    void setAllOutputs(uint8_t value);
    
    /**
     * @brief Control buzzer
     * @param times Number of beeps
     * @param duration Duration of each beep in ms
     * @param interval Interval between beeps in ms
     */
    void buzzerBeep(uint8_t times = 1, uint16_t duration = 100, uint16_t interval = 100);
    
    /**
     * @brief Setup Ethernet connection
     * @return true if successful
     */
    bool setupEthernet();
    
    /**
     * @brief Setup RS485 communication
     * @param baudRate Baud rate (default 9600)
     * @return true if successful
     */
    bool setupRS485(uint32_t baudRate = 9600);
    
    /**
     * @brief Setup LoRa DTU via RS485
     * @param baudRate LoRa DTU baud rate (default 9600)
     * @return true if successful
     */
    bool setupLoRaDTU(uint32_t baudRate = 9600);
    
    /**
     * @brief Setup CAN 2.0 interface
     * @param baudRate CAN baudrate in Kbps (default 500)
     * @return true if successful
     */
    bool setupCAN(uint32_t baudRate = 500);
    
    /**
     * @brief Get board information
     * @return String with board details
     */
    String getBoardInfo();
    
private:
    // GPIO Pin Definitions
    static const uint8_t RGB_LED_PIN = 48;        // RGB LED pin
    static const uint8_t BUZZER_PIN = 21;         // Buzzer output
    static const uint8_t RS485_TX_PIN = 17;       // RS485 TX (GPIO17)
    static const uint8_t RS485_RX_PIN = 18;       // RS485 RX (GPIO18)
    static const uint8_t CAN_TX_PIN = 42;         // CAN TX
    static const uint8_t CAN_RX_PIN = 41;         // CAN RX
    
    // Digital I/O Pin Mappings
    static const uint8_t DIN_PINS[8];             // Digital Input pins
    static const uint8_t DOUT_PINS[8];            // Digital Output pins
    
    // Status flags
    bool initialized;
    bool ethernet_connected;
    bool rs485_enabled;
    bool can_enabled;
    
    // Helper methods
    void configurePins();
    void initLED();
    void initBuzzer();
};

#endif // ESP32S3_8DI8DO_H
