/***********************************************************************
 * File         :     ESP32S3_8DI8DO.h
 * Description  :     Unified library for ESP32-S3-POE-ETH-8DI-8DO board
 * Author       :     Tenergy Innovation Co., Ltd.
 * Date         :     28 Jun 2026
 * Revision     :     1.2.0     
 * Rev1.0.0     :     - Consolidated library for ESP32-S3-POE-ETH-8DI-8DO [28-06-2026]
 * Rev1.0.1     :     - Fixed Bug TickxxxLED(-1), _writeRegister [28-06-2026 16:52]
 * Rev1.2.0     :     - Added RTC Real Time Clock support [28-06-2026 17:24]
 * Email        :     uten.boonliam@tenergyinnovation.co.th
 ***********************************************************************/

#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

/**
 * ============================================================
 * SECTION 1: DIGITAL INPUT (DI) CONTROLLER - 8 Channels
 * ============================================================
 */
class ESP32S3_DI {
public:
    // GPIO pin definitions for DI channels
    static constexpr uint8_t DI_PINS[8] = {4, 5, 6, 7, 8, 9, 10, 11};
    static constexpr uint8_t CHANNELS = 8;
    static constexpr uint8_t DEFAULT_DEBOUNCE_MS = 20;

    // Input mode definitions
    enum InputMode {
        INPUT_NORMAL = 0,      // Normal input (HIGH = on)
        INPUT_INVERTED = 1     // Inverted input (LOW = on)
    };

    // Constructor
    ESP32S3_DI();

    // Initialization
    bool begin();

    // Read operations
    bool read(uint8_t channel);
    uint8_t readAll();
    bool getState(uint8_t channel);

    // Callback management
    void setChangeCallback(uint8_t channel, void (*callback)(uint8_t, bool));

    // Debounce configuration
    void enableDebounce(uint8_t channel, uint8_t milliseconds = 20);
    void disableDebounce(uint8_t channel);
    uint8_t getDebounceTime(uint8_t channel);

    // Mode configuration
    void setInputMode(uint8_t channel, InputMode mode);

    // Interrupt support
    bool enableInterrupt(uint8_t channel);

    // Update routine (call in main loop)
    void update();

private:
    // State variables
    uint8_t _currentState;
    uint8_t _previousState;
    uint8_t _debounceTime[CHANNELS];
    unsigned long _lastChangeTime[CHANNELS];
    bool _debounceEnabled[CHANNELS];
    InputMode _inputMode[CHANNELS];
    void (*_callbacks[CHANNELS])(uint8_t, bool);

    // Helper methods
    void _initializeGPIO();
    bool _validateChannel(uint8_t channel);
    uint8_t _normalizeChannel(uint8_t channel);
};

/**
 * ============================================================
 * SECTION 2: DIGITAL OUTPUT (DO) CONTROLLER - 8 Channels
 * ============================================================
 */
class ESP32S3_DO {
public:
    // I2C Configuration
    static constexpr uint8_t I2C_ADDRESS = 0x20;
    static constexpr uint8_t SDA_PIN = 42;
    static constexpr uint8_t SCL_PIN = 41;
    static constexpr uint8_t CHANNELS = 8;

    // Output state definitions
    enum OutputState {
        OUTPUT_OFF = 0,
        OUTPUT_ON = 1,
        OUTPUT_TOGGLE = 2
    };

    // TCA9554PWR register addresses
    static constexpr uint8_t REG_INPUT_PORT = 0x00;
    static constexpr uint8_t REG_OUTPUT_PORT = 0x01;
    static constexpr uint8_t REG_POLARITY = 0x02;
    static constexpr uint8_t REG_CONFIG = 0x03;

    // Constructor
    ESP32S3_DO();

    // Initialization
    bool begin();

    // Output control
    bool set(uint8_t channel, OutputState state);
    bool on(uint8_t channel);
    bool off(uint8_t channel);
    bool toggle(uint8_t channel);

    // State query
    bool getState(uint8_t channel);
    uint8_t getMask();
    uint8_t getRawState();

    // Batch operations
    bool setMask(uint8_t mask);
    bool allOn();
    bool allOff();

    // I2C communication
    bool isConnected();

    // Polarity control
    bool setPolarity(uint8_t channel, bool inverted);

    // Enabled state
    bool setEnabled(uint8_t channel, bool enabled);

private:
    // I2C operations
    bool _writeRegister(uint8_t reg, uint8_t value);
    bool _readRegister(uint8_t reg, uint8_t &value);

    // State cache
    uint8_t _outputState;  // Cached output state
};

/**
 * ============================================================
 * SECTION 3: RS485 COMMUNICATION INTERFACE
 * ============================================================
 */
class ESP32S3_RS485 {
public:
    // Pin definitions for RS485 (UART1)
    static constexpr uint8_t TX_PIN = 17;
    static constexpr uint8_t RX_PIN = 18;
    static constexpr uint8_t RTS_PIN = 21;
    static constexpr uint8_t UART_NUM = 1;
    static constexpr size_t BUFFER_SIZE = 256;

    // Communication modes
    enum CommMode {
        MODE_AT_COMMAND = 0,
        MODE_STREAM = 1
    };

    // Constructor
    ESP32S3_RS485();

    // Initialization
    bool begin(unsigned long baudrate = 9600, CommMode mode = MODE_AT_COMMAND);
    void end();

    // Data transmission
    size_t write(const uint8_t* data, size_t length);
    size_t writeString(const char* str);

    // Data reception
    size_t read(uint8_t* buffer, size_t maxLength);
    size_t available();
    int peek();

    // Buffer control
    void flush();
    void clearRxBuffer();

    // Callbacks
    void setDataCallback(void (*callback)(uint8_t*, size_t));

    // Mode and baud control
    void setCommMode(CommMode mode);
    CommMode getCommMode() { return _commMode; }
    bool changeBaudrate(unsigned long baudrate);
    unsigned long getBaudrate() { return _baudrate; }

    // AT command support
    String sendATCommand(const char* command, uint16_t timeout = 1000);

    // Statistics
    uint32_t getBytesSent() { return _bytesSent; }
    uint32_t getBytesReceived() { return _bytesReceived; }
    void resetStats();

    // Update routine (call in main loop)
    void update();

private:
    // Serial configuration
    unsigned long _baudrate;
    CommMode _commMode;
    HardwareSerial *_serial;

    // RX buffer
    uint8_t _rxBuffer[BUFFER_SIZE];
    size_t _rxIndex;
    unsigned long _lastRxTime;

    // Callbacks
    void (*_dataCallback)(uint8_t*, size_t);

    // Statistics
    uint32_t _bytesSent;
    uint32_t _bytesReceived;

    // Helper methods
    void _initializeUART();
    void _processRxBuffer();
};

/**
 * ============================================================
 * SECTION 4: RGB LED INDICATOR CONTROLLER
 * ============================================================
 */
class ESP32S3_RGB {
public:
    // Pin definition
    static constexpr uint8_t RGB_PIN = 38;
    static constexpr uint8_t NUM_LEDS = 1;

    // RGB status indicator colors
    enum StatusColor {
        COLOR_OFF = 0x000000,
        COLOR_RED = 0xFF0000,
        COLOR_YELLOW = 0xFFFF00,
        COLOR_GREEN = 0x00FF00,
        COLOR_BLUE = 0x0000FF,
        COLOR_CYAN = 0x00FFFF,
        COLOR_MAGENTA = 0xFF00FF,
        COLOR_WHITE = 0xFFFFFF
    };

    // Animation modes
    enum AnimationMode {
        ANIM_SOLID = 0,
        ANIM_BLINK = 1,
        ANIM_PULSE = 2,
        ANIM_STROBE = 3,
        ANIM_BREATHE = 4
    };

    // Constructor
    ESP32S3_RGB();

    // Initialization
    bool begin();
    void end();

    // Color control
    void setColor(uint8_t red, uint8_t green, uint8_t blue);
    void setColor(uint32_t color);
    void setStatusColor(StatusColor status);
    uint32_t getColor() { return _currentColor; }

    // Animation effects
    void blink(uint32_t color, uint16_t onTime, uint16_t offTime, uint16_t count = 0);
    void blinkStatus(StatusColor status, uint16_t interval, uint16_t count = 0);
    void pulse(uint32_t color, uint16_t period = 1000);
    void strobe(uint32_t color, uint8_t speed = 5);
    void breathe(uint32_t color, uint16_t period = 2000);
    void stop();

    // Animation control
    void setAnimationMode(AnimationMode mode) { _animMode = mode; }
    AnimationMode getAnimationMode() { return _animMode; }

    // Brightness control
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() { return _brightness; }

    // Update routine (call in main loop)
    void update();

private:
    // State variables
    uint32_t _currentColor;
    uint32_t _targetColor;
    uint8_t _brightness;
    AnimationMode _animMode;
    unsigned long _lastUpdateTime;
    uint16_t _blinkOnTime;
    uint16_t _blinkOffTime;
    uint16_t _blinkCount;

    // NeoPixel object pointer for WS2812B LED
    Adafruit_NeoPixel *_neoPixel;

    // Helper methods
    void _initializeGPIO();
    void _updateLED();
    void _applyBrightness(uint8_t &r, uint8_t &g, uint8_t &b);
    
    // RGB to GRB conversion for NeoPixel
    uint32_t _rgbToGrb(uint32_t rgbColor);
};

/**
 * ============================================================
 * SECTION 5: BUZZER CONTROLLER
 * ============================================================
 */
class ESP32S3_Buzzer {
public:
    // Pin definition
    static constexpr uint8_t BUZZER_PIN = 46;
    static constexpr uint8_t PWM_CHANNEL = 0;      // Use LEDC channel 0 (primary PWM channel)
    static constexpr uint16_t DEFAULT_FREQ = 1000; // 1000 Hz buzzer frequency
    static constexpr uint8_t PWM_RESOLUTION = 8;   // 8-bit resolution (0-255)
    static constexpr uint8_t MAX_DUTY = 255;

    // Buzzer patterns
    enum BuzzerPattern {
        PATTERN_SHORT = 0,
        PATTERN_LONG = 1,
        PATTERN_DOUBLE = 2,
        PATTERN_TRIPLE = 3,
        PATTERN_ALARM = 4,
        PATTERN_WARNING = 5
    };

    // Preset frequencies (musical notes)
    static constexpr uint16_t FREQ_C4 = 262;
    static constexpr uint16_t FREQ_D4 = 294;
    static constexpr uint16_t FREQ_E4 = 330;
    static constexpr uint16_t FREQ_F4 = 349;
    static constexpr uint16_t FREQ_G4 = 392;
    static constexpr uint16_t FREQ_A4 = 440;
    static constexpr uint16_t FREQ_B4 = 494;

    // Constructor
    ESP32S3_Buzzer();

    // Initialization
    bool begin(uint16_t frequency = DEFAULT_FREQ);
    void end();

    // Basic control
    void on();
    void off();
    void setVolume(uint8_t volume);
    uint8_t getVolume() { return _currentVolume; }

    // Beeping operations
    void beep(uint16_t duration = 100, uint8_t volume = 255);
    void playPattern(BuzzerPattern pattern);
    void playSequence(const uint16_t* sequence, uint8_t count, uint8_t volume = 255);

    // Frequency control
    void setFrequency(uint16_t frequency);
    uint16_t getFrequency() { return _frequency; }

    // Status
    bool isPlaying() { return _isPlaying; }
    String getStatus();

    // Control
    void stop();

    // Update routine (call in main loop)
    void update();

private:
    // State variables
    uint16_t _frequency;
    uint8_t _currentVolume;
    bool _isPlaying;
    unsigned long _playStartTime;
    uint16_t _playDuration;

    // Sequence playback
    const uint16_t* _sequence;
    uint8_t _sequenceCount;
    uint8_t _sequenceIndex;
    unsigned long _sequenceStartTime;

    // Helper methods
    void _initializeGPIO();
    void _updatePlayback();
};

/**
 * ============================================================
 * SECTION 6: MAIN BOARD CONTROLLER CLASS
 * ============================================================
 */
class ESP32S3_8DI8DO {
public:
    // Board information constants
    static constexpr const char* BOARD_NAME = "ESP32-S3-POE-ETH-8DI-8DO";
    static constexpr uint8_t DI_CHANNELS = 8;
    static constexpr uint8_t DO_CHANNELS = 8;
    static constexpr uint16_t MAX_BUFFER_SIZE = 256;

    //library version
    static constexpr const char* LIBRARY_VERSION = "1.2.0";

    // Status codes
    enum Status : uint8_t {
        STATUS_OK = 0,
        STATUS_INIT_FAILED = 1,
        STATUS_DI_ERROR = 2,
        STATUS_DO_ERROR = 3,
        STATUS_RS485_ERROR = 4,
        STATUS_RGB_ERROR = 5,
        STATUS_SYSTEM_ERROR = 255
    };

    // Constructor
    ESP32S3_8DI8DO();

    // Initialization
    Status begin();

    // RGB LED control
    void setRGBColor(uint8_t red, uint8_t green, uint8_t blue);
    void LEDOff();

    // Digital I/O control
    void setDigitalOutput(uint8_t channel, uint8_t state);
    uint8_t getDigitalInput(uint8_t channel);
    uint8_t readAllInputs();
    void setAllOutputs(uint8_t value);

    // Buzzer control
    void buzzerBeep(uint8_t times = 1, uint16_t duration = 100, uint16_t interval = 100);
    void buzzer_beep(int times);

    // RGB LED Ticker Animations
    void TickRedLED(float seconds);
    void TickGreenLED(float seconds);
    void TickBlueLED(float seconds);
    void TickYellowLED(float seconds);
    void TickPurpleLED(float seconds);
    void TickOrangeLED(float seconds);
    void TickWhiteLED(float seconds);

    // RTC (Real-Time Clock) control
    /***********************************************************************
     * FUNCTION:    setRTC
     * DESCRIPTION: Sets the RTC (Real-Time Clock) time on the ESP32-S3 board.
     *              This writes the time to the internal RTC chip.
     * PARAMETERS:  year   - Year (2000-2099)
     *              month  - Month (1-12)
     *              day    - Day of month (1-31)
     *              hour   - Hour (0-23)
     *              minute - Minute (0-59)
     *              second - Second (0-59)
     * RETURNED:    None
     ***********************************************************************/
    void setRTC(int year, int month, int day, int hour, int minute, int second);

    /***********************************************************************
     * FUNCTION:    getRTC
     * DESCRIPTION: Reads the current time from the RTC (Real-Time Clock) chip
     *              and returns it as a formatted string.
     * PARAMETERS:  None
     * RETURNED:    String in format "YYYY-MM-DD HH:MM:SS"
     *              Example: "2026-06-28 15:30:45"
     ***********************************************************************/
    String getRTC();

    // Interface setup
    bool setupEthernet();
    bool setupRS485(uint32_t baudRate = 9600);
    bool setupLoRaDTU(uint32_t baudRate = 9600);
    bool setupCAN(uint32_t baudRate = 500);

    // Board information
    String getBoardInfo();

    // Subsystem access (for advanced users)
    ESP32S3_DI& getDI() { return _di; }
    ESP32S3_DO& getDO() { return _do; }
    ESP32S3_RS485& getRS485() { return _rs485; }
    ESP32S3_RGB& getRGB() { return _rgb; }
    ESP32S3_Buzzer& getBuzzer() { return _buzzer; }

    // Update routine (call in main loop)
    void loop();

    // Status query
    Status getStatus() { return _status; }
    bool isInitialized() { return initialized; }

private:
    // GPIO Pin Definitions
    static const uint8_t RGB_LED_PIN;
    static const uint8_t BUZZER_PIN;
    static const uint8_t RS485_TX_PIN;
    static const uint8_t RS485_RX_PIN;
    static const uint8_t CAN_TX_PIN;
    static const uint8_t CAN_RX_PIN;

    // Subsystem controllers (internal instances)
    ESP32S3_DI _di;
    ESP32S3_DO _do;
    ESP32S3_RS485 _rs485;
    ESP32S3_RGB _rgb;
    ESP32S3_Buzzer _buzzer;

    // Status flags
    bool initialized;
    bool ethernet_connected;
    bool rs485_enabled;
    bool can_enabled;
    Status _status;

    // LED Animation State
    bool _ledBlinking;
    uint32_t _ledBlinkColor;
    unsigned long _ledBlinkStartTime;
    float _ledBlinkInterval;

    // Helper methods
    void configurePins();
    void initLED();
    void initBuzzer();
};
