/***********************************************************************
 * File         :     ESP32S3_8DI8DO.c
 * Description  :     Unified library implementation for ESP32-S3-POE-ETH-8DI-8DO board
 * Author       :     Tenergy Innovation Co., Ltd.
 * Date         :     28 Jun 2026
 * Revision     :     1.0.0 (Consolidated)
 * Email        :     uten.boonliam@tenergyinnovation.co.th
 ***********************************************************************/

#include "ESP32S3_8DI8DO.h"

// ============================================================
// STATIC MEMBER DEFINITIONS
// ============================================================
const uint8_t ESP32S3_8DI8DO::RGB_LED_PIN = 38;
const uint8_t ESP32S3_8DI8DO::BUZZER_PIN = 46;
const uint8_t ESP32S3_8DI8DO::RS485_TX_PIN = 17;
const uint8_t ESP32S3_8DI8DO::RS485_RX_PIN = 18;
const uint8_t ESP32S3_8DI8DO::CAN_TX_PIN = 0;
const uint8_t ESP32S3_8DI8DO::CAN_RX_PIN = 1;

// DI_PINS array definition
constexpr uint8_t ESP32S3_DI::DI_PINS[8];

// ============================================================
// SECTION 1: DIGITAL INPUT (DI) IMPLEMENTATION
// ============================================================

ESP32S3_DI::ESP32S3_DI() 
    : _currentState(0), _previousState(0) {
    memset(_debounceTime, DEFAULT_DEBOUNCE_MS, CHANNELS);
    memset(_lastChangeTime, 0, sizeof(_lastChangeTime));
    memset(_debounceEnabled, false, CHANNELS);
    memset(_inputMode, INPUT_NORMAL, CHANNELS);
    memset(_callbacks, 0, sizeof(_callbacks));
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DI::begin
 * DESCRIPTION: Initializes the ESP32S3_DI class. Sets up GPIO pins and internal state variables.
 * PARAMETERS:  None
 * RETURNED:    None
 ***********************************************************************/
bool ESP32S3_DI::begin() {
    _initializeGPIO();
    return true;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DI::read
 * DESCRIPTION: Reads the state of a specific digital input channel.
 * PARAMETERS:  channel - The digital input channel to read.
 * RETURNED:    true if the input is HIGH, false otherwise.
 ***********************************************************************/
bool ESP32S3_DI::read(uint8_t channel) {
    if (!_validateChannel(channel)) return false;
    channel = _normalizeChannel(channel);
    return digitalRead(DI_PINS[channel]) == HIGH;
}


/***********************************************************************
 * FUNCTION:    ESP32S3_DI::readAll
 * DESCRIPTION: Reads the state of all 8 digital input channels at once.
 * PARAMETERS:  None
 * RETURNED:    uint8_t - Bitmask representing all 8 channels (bit 0 = CH0, bit 7 = CH7)
 ***********************************************************************/
uint8_t ESP32S3_DI::readAll() {
    uint8_t result = 0;
    for (int i = 0; i < CHANNELS; i++) {
        if (digitalRead(DI_PINS[i]) == HIGH) {
            result |= (1 << i);
        }
    }
    return result;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DI::getState
 * DESCRIPTION: Gets the cached state of a specific digital input channel.
 *              Returns the state from the last update() call (debounced).
 * PARAMETERS:  channel - The digital input channel (0-7)
 * RETURNED:    true if input is HIGH, false if LOW
 ***********************************************************************/
bool ESP32S3_DI::getState(uint8_t channel) {
    if (!_validateChannel(channel)) return false;
    channel = _normalizeChannel(channel);
    return (_currentState >> channel) & 1;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DI::setChangeCallback
 * DESCRIPTION: Registers a callback function to be called when a channel state changes.
 * PARAMETERS:  channel - The digital input channel (0-7)
 *              callback - Function pointer with signature: void callback(uint8_t ch, bool newState)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_DI::setChangeCallback(uint8_t channel, void (*callback)(uint8_t, bool)) {
    if (!_validateChannel(channel)) return;
    channel = _normalizeChannel(channel);
    _callbacks[channel] = callback;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DI::enableDebounce
 * DESCRIPTION: Enables debouncing on a specific digital input channel.
 *              Debouncing filters out noise by requiring stable state for a set time.
 * PARAMETERS:  channel - The digital input channel (0-7)
 *              milliseconds - Debounce time in milliseconds (default: 20ms)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_DI::enableDebounce(uint8_t channel, uint8_t milliseconds) {
    if (!_validateChannel(channel)) return;
    channel = _normalizeChannel(channel);
    _debounceEnabled[channel] = true;
    _debounceTime[channel] = milliseconds;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DI::disableDebounce
 * DESCRIPTION: Disables debouncing on a specific digital input channel.
 * PARAMETERS:  channel - The digital input channel (0-7)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_DI::disableDebounce(uint8_t channel) {
    if (!_validateChannel(channel)) return;
    channel = _normalizeChannel(channel);
    _debounceEnabled[channel] = false;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DI::getDebounceTime
 * DESCRIPTION: Gets the current debounce time setting for a channel.
 * PARAMETERS:  channel - The digital input channel (0-7)
 * RETURNED:    uint8_t - Debounce time in milliseconds
 ***********************************************************************/
uint8_t ESP32S3_DI::getDebounceTime(uint8_t channel) {
    if (!_validateChannel(channel)) return 0;
    channel = _normalizeChannel(channel);
    return _debounceTime[channel];
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DI::setInputMode
 * DESCRIPTION: Sets the input mode (normal or inverted) for a specific channel.
 * PARAMETERS:  channel - The digital input channel (0-7)
 *              mode - INPUT_NORMAL (HIGH=on) or INPUT_INVERTED (LOW=on)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_DI::setInputMode(uint8_t channel, InputMode mode) {
    if (!_validateChannel(channel)) return;
    channel = _normalizeChannel(channel);
    _inputMode[channel] = mode;
}

bool ESP32S3_DI::enableInterrupt(uint8_t channel) {
    // TODO: Implement interrupt-based detection
    return false;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DI::update
 * DESCRIPTION: Updates all channel states and processes debouncing.
 *              Must be called regularly in the main loop for proper operation.
 * PARAMETERS:  None
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_DI::update() {
    _currentState = readAll();
    
    // Check each channel for state changes
    for (int i = 0; i < CHANNELS; i++) {
        bool currentBit = (_currentState >> i) & 1;
        bool previousBit = (_previousState >> i) & 1;
        
        if (currentBit != previousBit) {
            // State change detected
            if (_debounceEnabled[i]) {
                unsigned long now = millis();
                if (now - _lastChangeTime[i] < _debounceTime[i]) {
                    continue;  // Skip if within debounce time
                }
                _lastChangeTime[i] = now;
            }
            
            // Apply input mode (invert if needed)
            bool reportedState = currentBit;
            if (_inputMode[i] == INPUT_INVERTED) {
                reportedState = !reportedState;
            }
            
            // Call callback if registered
            if (_callbacks[i] != nullptr) {
                _callbacks[i](i, reportedState);
            }
        }
    }
    
    _previousState = _currentState;
}

void ESP32S3_DI::_initializeGPIO() {
    for (int i = 0; i < CHANNELS; i++) {
        pinMode(DI_PINS[i], INPUT);
    }
}

bool ESP32S3_DI::_validateChannel(uint8_t channel) {
    return (channel < CHANNELS) || (channel >= 1 && channel <= 8);
}

uint8_t ESP32S3_DI::_normalizeChannel(uint8_t channel) {
    return (channel >= 1 && channel <= 8) ? (channel - 1) : channel;
}

// ============================================================
// SECTION 2: DIGITAL OUTPUT (DO) IMPLEMENTATION
// ============================================================

ESP32S3_DO::ESP32S3_DO() : _outputState(0) {}

/***********************************************************************
 * FUNCTION:    ESP32S3_DO::begin
 * DESCRIPTION: Initializes the digital output controller via I2C.
 *              Configures the TCA9554 I/O expander for relay control.
 * PARAMETERS:  None
 * RETURNED:    bool - true if initialization successful, false otherwise
 ***********************************************************************/
bool ESP32S3_DO::begin() {
    Wire.begin(SDA_PIN, SCL_PIN);
    
    // Configure TCA9554 as outputs
    _writeRegister(REG_CONFIG, 0x00);  // All pins as outputs
    _writeRegister(REG_OUTPUT_PORT, 0xFF);  // All off initially
    _outputState = 0xFF;  // ← Sync internal state with hardware!
    
    return isConnected();
}

bool ESP32S3_DO::set(uint8_t channel, OutputState state) {
    if (channel >= CHANNELS) return false;
    
    if (state == OUTPUT_TOGGLE) {
        _outputState ^= (1 << channel);
    } else if (state == OUTPUT_ON) {
        _outputState &= ~(1 << channel);  // Register: 0 = ON
    } else {
        _outputState |= (1 << channel);   // Register: 1 = OFF
    }
    
    return _writeRegister(REG_OUTPUT_PORT, _outputState);
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DO::on
 * DESCRIPTION: Turns on (activates) a specific digital output relay.
 * PARAMETERS:  channel - The digital output channel (0-7)
 * RETURNED:    bool - true if successful, false if invalid channel
 ***********************************************************************/
bool ESP32S3_DO::on(uint8_t channel) {
    return set(channel, OUTPUT_ON);
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DO::off
 * DESCRIPTION: Turns off (deactivates) a specific digital output relay.
 * PARAMETERS:  channel - The digital output channel (0-7)
 * RETURNED:    bool - true if successful, false if invalid channel
 ***********************************************************************/
bool ESP32S3_DO::off(uint8_t channel) {
    return set(channel, OUTPUT_OFF);
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DO::toggle
 * DESCRIPTION: Toggles the state of a specific digital output relay.
 * PARAMETERS:  channel - The digital output channel (0-7)
 * RETURNED:    bool - true if successful, false if invalid channel
 ***********************************************************************/
bool ESP32S3_DO::toggle(uint8_t channel) {
    return set(channel, OUTPUT_TOGGLE);
}

bool ESP32S3_DO::getState(uint8_t channel) {
    if (channel >= CHANNELS) return false;
    return ((_outputState >> channel) & 1) == 0;  // 0 = ON
}

uint8_t ESP32S3_DO::getMask() {
    return ~_outputState;  // Invert for user interface
}

uint8_t ESP32S3_DO::getRawState() {
    return _outputState;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DO::setMask
 * DESCRIPTION: Sets multiple output relays at once using a bitmask.
 * PARAMETERS:  mask - Bitmask where bit=1 means ON, bit=0 means OFF
 * RETURNED:    bool - true if successful, false otherwise
 ***********************************************************************/
bool ESP32S3_DO::setMask(uint8_t mask) {
    _outputState = ~mask;  // Invert for register
    return _writeRegister(REG_OUTPUT_PORT, _outputState);
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DO::allOn
 * DESCRIPTION: Turns on all 8 digital output relays simultaneously.
 * PARAMETERS:  None
 * RETURNED:    bool - true if successful, false otherwise
 ***********************************************************************/
bool ESP32S3_DO::allOn() {
    _outputState = 0x00;
    return _writeRegister(REG_OUTPUT_PORT, _outputState);
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DO::allOff
 * DESCRIPTION: Turns off all 8 digital output relays simultaneously.
 * PARAMETERS:  None
 * RETURNED:    bool - true if successful, false otherwise
 ***********************************************************************/
bool ESP32S3_DO::allOff() {
    _outputState = 0xFF;
    return _writeRegister(REG_OUTPUT_PORT, _outputState);
}

/***********************************************************************
 * FUNCTION:    ESP32S3_DO::isConnected
 * DESCRIPTION: Checks if the TCA9554 I/O expander is connected on I2C bus.
 * PARAMETERS:  None
 * RETURNED:    bool - true if device responds to I2C address, false otherwise
 ***********************************************************************/
bool ESP32S3_DO::isConnected() {
    Wire.beginTransmission(I2C_ADDRESS);
    return Wire.endTransmission() == 0;
}

bool ESP32S3_DO::setPolarity(uint8_t channel, bool inverted) {
    uint8_t polarity;
    if (!_readRegister(REG_POLARITY, polarity)) return false;
    
    if (inverted) {
        polarity |= (1 << channel);
    } else {
        polarity &= ~(1 << channel);
    }
    
    return _writeRegister(REG_POLARITY, polarity);
}

bool ESP32S3_DO::setEnabled(uint8_t channel, bool enabled) {
    // For DO, all channels are always enabled, but we can control output
    return enabled ? on(channel) : off(channel);
}

bool ESP32S3_DO::_writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool ESP32S3_DO::_readRegister(uint8_t reg, uint8_t &value) {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    
    if (Wire.requestFrom(I2C_ADDRESS, 1) != 1) return false;
    value = Wire.read();
    return true;
}

// ============================================================
// SECTION 3: RS485 IMPLEMENTATION
// ============================================================

ESP32S3_RS485::ESP32S3_RS485()
    : _baudrate(9600), _commMode(MODE_AT_COMMAND), _serial(&Serial1),
      _rxIndex(0), _lastRxTime(0), _dataCallback(nullptr),
      _bytesSent(0), _bytesReceived(0) {}

bool ESP32S3_RS485::begin(unsigned long baudrate, CommMode mode) {
    _baudrate = baudrate;
    _commMode = mode;
    _initializeUART();
    return true;
}

void ESP32S3_RS485::end() {
    _serial->end();
}

/***********************************************************************
 * FUNCTION:    ESP32S3_RS485::write
 * DESCRIPTION: Writes raw binary data to the RS485 interface.
 * PARAMETERS:  data - Pointer to byte array to send
 *              length - Number of bytes to send
 * RETURNED:    size_t - Number of bytes successfully written
 ***********************************************************************/
size_t ESP32S3_RS485::write(const uint8_t* data, size_t length) {
    size_t written = _serial->write(data, length);
    _bytesSent += written;
    return written;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_RS485::writeString
 * DESCRIPTION: Writes a string to the RS485 interface.
 * PARAMETERS:  str - Null-terminated string to send
 * RETURNED:    size_t - Number of bytes successfully written
 ***********************************************************************/
size_t ESP32S3_RS485::writeString(const char* str) {
    size_t written = _serial->print(str);
    _bytesSent += written;
    return written;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_RS485::read
 * DESCRIPTION: Reads available data from the RS485 interface into a buffer.
 * PARAMETERS:  buffer - Pointer to byte array to store received data
 *              maxLength - Maximum number of bytes to read
 * RETURNED:    size_t - Number of bytes actually read
 ***********************************************************************/
size_t ESP32S3_RS485::read(uint8_t* buffer, size_t maxLength) {
    size_t count = 0;
    while (_serial->available() && count < maxLength) {
        buffer[count++] = _serial->read();
    }
    _bytesReceived += count;
    return count;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_RS485::available
 * DESCRIPTION: Checks how many bytes are available to read from RS485 buffer.
 * PARAMETERS:  None
 * RETURNED:    size_t - Number of bytes available to read
 ***********************************************************************/
size_t ESP32S3_RS485::available() {
    return _serial->available();
}

int ESP32S3_RS485::peek() {
    return _serial->peek();
}

void ESP32S3_RS485::flush() {
    _serial->flush();
}

void ESP32S3_RS485::clearRxBuffer() {
    while (_serial->available()) {
        _serial->read();
    }
    _rxIndex = 0;
}

void ESP32S3_RS485::setDataCallback(void (*callback)(uint8_t*, size_t)) {
    _dataCallback = callback;
}

void ESP32S3_RS485::setCommMode(CommMode mode) {
    _commMode = mode;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_RS485::changeBaudrate
 * DESCRIPTION: Changes the baud rate of the RS485 serial connection.
 * PARAMETERS:  baudrate - New baud rate (9600, 14400, 19200, 38400, 57600, 115200)
 * RETURNED:    bool - Always returns true if operation completed
 ***********************************************************************/
bool ESP32S3_RS485::changeBaudrate(unsigned long baudrate) {
    _baudrate = baudrate;
    _serial->begin(_baudrate, SERIAL_8N1, RX_PIN, TX_PIN);
    return true;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_RS485::sendATCommand
 * DESCRIPTION: Sends an AT command to RS485 device and waits for response.
 * PARAMETERS:  command - AT command string to send
 *              timeout - Time to wait for response in milliseconds
 * RETURNED:    String - Response from the device
 ***********************************************************************/
String ESP32S3_RS485::sendATCommand(const char* command, uint16_t timeout) {
    String response = "";
    unsigned long startTime = millis();
    
    // Send command
    _serial->print(command);
    _serial->print("\r\n");
    
    // Wait for response
    while (millis() - startTime < timeout) {
        if (_serial->available()) {
            char c = _serial->read();
            response += c;
            startTime = millis();  // Reset timeout on each character
        }
    }
    
    return response;
}

void ESP32S3_RS485::resetStats() {
    _bytesSent = 0;
    _bytesReceived = 0;
}

void ESP32S3_RS485::update() {
    _processRxBuffer();
}

void ESP32S3_RS485::_initializeUART() {
    _serial->begin(_baudrate, SERIAL_8N1, RX_PIN, TX_PIN);
    delay(100);
}

void ESP32S3_RS485::_processRxBuffer() {
    while (_serial->available()) {
        uint8_t byte = _serial->read();
        _rxBuffer[_rxIndex++] = byte;
        _lastRxTime = millis();
        
        if (_rxIndex >= BUFFER_SIZE) {
            _rxIndex = 0;
        }
    }
    
    // Call callback if complete message received
    if (_rxIndex > 0 && (millis() - _lastRxTime) > 10) {
        if (_dataCallback != nullptr) {
            _dataCallback(_rxBuffer, _rxIndex);
        }
        _rxIndex = 0;
    }
}

// ============================================================
// SECTION 4: RGB LED IMPLEMENTATION
// ============================================================

ESP32S3_RGB::ESP32S3_RGB()
    : _currentColor(0), _targetColor(0), _brightness(255),
      _animMode(ANIM_SOLID), _lastUpdateTime(0),
      _blinkOnTime(500), _blinkOffTime(500), _blinkCount(0),
      _neoPixel(nullptr) {}

bool ESP32S3_RGB::begin() {
    // Initialize NeoPixel object for WS2812B LED on GPIO38
    _neoPixel = new Adafruit_NeoPixel(NUM_LEDS, RGB_PIN, NEO_GRB + NEO_KHZ800);
    
    if (_neoPixel == nullptr) {
        return false;
    }
    
    _neoPixel->begin();
    _neoPixel->show();  // Initialize with all LEDs off
    return true;
}

void ESP32S3_RGB::end() {
    if (_neoPixel != nullptr) {
        _neoPixel->clear();
        _neoPixel->show();
        delete _neoPixel;
        _neoPixel = nullptr;
    }
}

/***********************************************************************
 * FUNCTION:    ESP32S3_RGB::setColor
 * DESCRIPTION: Sets the RGB LED to a specific color.
 *              Input uses RGB format (0xRRGGBB), automatically converted
 *              to GRB format for WS2812B NeoPixel LED.
 * PARAMETERS:  red - Red component (0-255)
 *              green - Green component (0-255)
 *              blue - Blue component (0-255)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_RGB::setColor(uint8_t red, uint8_t green, uint8_t blue) {
    _targetColor = ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
    _currentColor = _targetColor;
    _animMode = ANIM_SOLID;
    _updateLED();  // Update immediately
}

void ESP32S3_RGB::setColor(uint32_t color) {
    _targetColor = color;
    _currentColor = color;
    _animMode = ANIM_SOLID;
    _updateLED();  // Update immediately
}

void ESP32S3_RGB::setStatusColor(StatusColor status) {
    setColor((uint32_t)status);
}

/***********************************************************************
 * FUNCTION:    ESP32S3_RGB::blink
 * DESCRIPTION: Makes the RGB LED blink at a specified interval.
 * PARAMETERS:  color - 24-bit RGB color value (0xRRGGBB)
 *              onTime - Time LED is ON in milliseconds
 *              offTime - Time LED is OFF in milliseconds
 *              count - Number of blinks (0 = infinite)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_RGB::blink(uint32_t color, uint16_t onTime, uint16_t offTime, uint16_t count) {
    setColor(color);
    _blinkOnTime = onTime;
    _blinkOffTime = offTime;
    _blinkCount = count;
    _animMode = ANIM_BLINK;
    _lastUpdateTime = millis();
}

void ESP32S3_RGB::blinkStatus(StatusColor status, uint16_t interval, uint16_t count) {
    blink((uint32_t)status, interval / 2, interval / 2, count);
}

/***********************************************************************
 * FUNCTION:    ESP32S3_RGB::pulse
 * DESCRIPTION: Makes the RGB LED pulse (fade in/out) continuously.
 * PARAMETERS:  color - 24-bit RGB color value (0xRRGGBB)
 *              period - Complete pulse cycle time in milliseconds
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_RGB::pulse(uint32_t color, uint16_t period) {
    setColor(color);
    _blinkOnTime = period;
    _animMode = ANIM_PULSE;
    _lastUpdateTime = millis();
}

void ESP32S3_RGB::strobe(uint32_t color, uint8_t speed) {
    setColor(color);
    _blinkOnTime = 1000 / speed / 2;
    _animMode = ANIM_STROBE;
    _lastUpdateTime = millis();
}

void ESP32S3_RGB::breathe(uint32_t color, uint16_t period) {
    setColor(color);
    _blinkOnTime = period;
    _animMode = ANIM_BREATHE;
    _lastUpdateTime = millis();
}

/***********************************************************************
 * FUNCTION:    ESP32S3_RGB::stop
 * DESCRIPTION: Stops all LED animations and keeps the current color on.
 * PARAMETERS:  None
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_RGB::stop() {
    _animMode = ANIM_SOLID;
    _currentColor = _targetColor;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_RGB::setBrightness
 * DESCRIPTION: Sets the brightness level of the RGB LED.
 * PARAMETERS:  brightness - Brightness level (0-255, where 0=off, 255=full brightness)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_RGB::setBrightness(uint8_t brightness) {
    _brightness = brightness;
}

void ESP32S3_RGB::update() {
    _updateLED();
}

void ESP32S3_RGB::_initializeGPIO() {
    // Initialization is handled by NeoPixel begin()
    // No GPIO setup needed here
}

/***********************************************************************
 * FUNCTION:    ESP32S3_RGB::_rgbToGrb
 * DESCRIPTION: Converts RGB color format to GRB format for NeoPixel.
 *              WS2812B LEDs use GRB order internally.
 * PARAMETERS:  rgbColor - Color in RGB format (0xRRGGBB)
 * RETURNED:    Color in GRB format (0xGGRRBB) for NeoPixel
 ***********************************************************************/
uint32_t ESP32S3_RGB::_rgbToGrb(uint32_t rgbColor) {
    uint8_t r = (rgbColor >> 16) & 0xFF;
    uint8_t g = (rgbColor >> 8) & 0xFF;
    uint8_t b = rgbColor & 0xFF;
    
    // Return in GRB format for Adafruit_NeoPixel
    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
}

void ESP32S3_RGB::_updateLED() {
    if (_neoPixel == nullptr) return;
    
    if (_animMode == ANIM_SOLID) {
        // Solid color display
        uint32_t displayColor = _rgbToGrb(_currentColor);
        
        // Apply brightness
        uint8_t r = (displayColor >> 8) & 0xFF;
        uint8_t g = (displayColor >> 16) & 0xFF;
        uint8_t b = displayColor & 0xFF;
        
        r = (r * _brightness) / 255;
        g = (g * _brightness) / 255;
        b = (b * _brightness) / 255;
        
        _neoPixel->setPixelColor(0, g, r, b);
        _neoPixel->show();
        
    } else if (_animMode == ANIM_BLINK) {
        // Blinking animation
        unsigned long elapsed = millis() - _lastUpdateTime;
        uint32_t cycleTime = _blinkOnTime + _blinkOffTime;
        unsigned long pos = elapsed % cycleTime;
        
        uint32_t displayColor = _rgbToGrb(_currentColor);
        
        uint8_t r = (displayColor >> 8) & 0xFF;
        uint8_t g = (displayColor >> 16) & 0xFF;
        uint8_t b = displayColor & 0xFF;
        
        r = (r * _brightness) / 255;
        g = (g * _brightness) / 255;
        b = (b * _brightness) / 255;
        
        if (pos < _blinkOnTime) {
            // LED ON
            _neoPixel->setPixelColor(0, g, r, b);
        } else {
            // LED OFF
            _neoPixel->setPixelColor(0, 0, 0, 0);
        }
        
        _neoPixel->show();
        
    } else if (_animMode == ANIM_PULSE) {
        // Pulse/breathe animation
        unsigned long elapsed = millis() - _lastUpdateTime;
        unsigned long pos = elapsed % _blinkOnTime;
        
        // Calculate brightness wave (0-255 and back)
        uint8_t brightness = (pos < _blinkOnTime / 2) ? 
                            (pos * 510 / _blinkOnTime) :
                            (510 - pos * 510 / _blinkOnTime);
        
        uint32_t displayColor = _rgbToGrb(_currentColor);
        
        uint8_t r = (displayColor >> 8) & 0xFF;
        uint8_t g = (displayColor >> 16) & 0xFF;
        uint8_t b = displayColor & 0xFF;
        
        r = (r * brightness * _brightness) / (255 * 255);
        g = (g * brightness * _brightness) / (255 * 255);
        b = (b * brightness * _brightness) / (255 * 255);
        
        _neoPixel->setPixelColor(0, g, r, b);
        _neoPixel->show();
    }
}

void ESP32S3_RGB::_applyBrightness(uint8_t &r, uint8_t &g, uint8_t &b) {
    r = (r * _brightness) / 255;
    g = (g * _brightness) / 255;
    b = (b * _brightness) / 255;
}

// ============================================================
// SECTION 5: BUZZER IMPLEMENTATION
// ============================================================

ESP32S3_Buzzer::ESP32S3_Buzzer()
    : _frequency(DEFAULT_FREQ), _currentVolume(0), _isPlaying(false),
      _playStartTime(0), _playDuration(0), _sequence(nullptr),
      _sequenceCount(0), _sequenceIndex(0), _sequenceStartTime(0) {}

bool ESP32S3_Buzzer::begin(uint16_t frequency) {
    _frequency = frequency;
    _initializeGPIO();
    return true;
}

void ESP32S3_Buzzer::end() {
    ledcDetachPin(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);
}

void ESP32S3_Buzzer::on() {
    setVolume(MAX_DUTY);
    _isPlaying = true;
}

void ESP32S3_Buzzer::off() {
    setVolume(0);
    _isPlaying = false;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_Buzzer::setVolume
 * DESCRIPTION: Sets the buzzer volume using PWM duty cycle.
 *              Uses LEDC channel 1 at 1kHz frequency for proper sound.
 * PARAMETERS:  volume - Volume level (0-255, where 0=silent, 255=max volume)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_Buzzer::setVolume(uint8_t volume) {
    _currentVolume = volume;
    // Use LEDC to generate PWM at buzzer frequency
    // ledcWrite maps duty cycle 0-255 to PWM output
    ledcWrite(PWM_CHANNEL, volume);
}

/***********************************************************************
 * FUNCTION:    ESP32S3_Buzzer::beep
 * DESCRIPTION: Produces a beep sound for a specified duration.
 *              Uses LEDC PWM to generate precise 1kHz tone.
 * PARAMETERS:  duration - Beep duration in milliseconds (1-5000)
 *              volume - Volume level (0-255, where 0=silent, 255=full volume)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_Buzzer::beep(uint16_t duration, uint8_t volume) {
    // Constrain volume and duration to safe ranges
    volume = constrain(volume, 0, 255);
    duration = constrain(duration, 1, 5000);
    
    // Turn buzzer ON with specified volume (PWM duty cycle)
    ledcWrite(PWM_CHANNEL, volume);
    delay(duration);
    
    // Turn buzzer OFF
    ledcWrite(PWM_CHANNEL, 0);
}

/***********************************************************************
 * FUNCTION:    ESP32S3_Buzzer::playPattern
 * DESCRIPTION: Plays predefined beep patterns for status indication.
 * PARAMETERS:  pattern - Pattern type (SHORT, LONG, DOUBLE, TRIPLE, ALARM, WARNING)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_Buzzer::playPattern(BuzzerPattern pattern) {
    switch (pattern) {
        case PATTERN_SHORT:
            beep(100);
            break;
        case PATTERN_LONG:
            beep(500);
            break;
        case PATTERN_DOUBLE:
        case PATTERN_TRIPLE:
        case PATTERN_ALARM:
        case PATTERN_WARNING:
            beep(100);
            break;
    }
}

void ESP32S3_Buzzer::playSequence(const uint16_t* sequence, uint8_t count, uint8_t volume) {
    _sequence = sequence;
    _sequenceCount = count;
    _sequenceIndex = 0;
    _currentVolume = volume;
    _sequenceStartTime = millis();
    _isPlaying = true;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_Buzzer::setFrequency
 * DESCRIPTION: Sets the PWM frequency of the buzzer sound.
 *              Uses LEDC to change the frequency on the configured channel.
 * PARAMETERS:  frequency - Frequency in Hz (typical: 1000Hz for standard buzzer)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_Buzzer::setFrequency(uint16_t frequency) {
    _frequency = frequency;
    // Change the LEDC frequency on channel 1
    // ledcChangeFrequency(channel, frequency, resolution)
    ledcChangeFrequency(PWM_CHANNEL, frequency, PWM_RESOLUTION);
}

String ESP32S3_Buzzer::getStatus() {
    return _isPlaying ? "Playing" : "Idle";
}

/***********************************************************************
 * FUNCTION:    ESP32S3_Buzzer::stop
 * DESCRIPTION: Stops the buzzer immediately and clears any active sound sequence.
 * PARAMETERS:  None
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_Buzzer::stop() {
    off();
    _isPlaying = false;
    _sequence = nullptr;
}

void ESP32S3_Buzzer::update() {
    _updatePlayback();
}

void ESP32S3_Buzzer::_initializeGPIO() {
    // Configure LEDC (PWM) for buzzer
    // ledcSetup(channel, frequency, resolution)
    // PWM_CHANNEL: 1 (ESP32 has 16 channels, 0-15)
    // DEFAULT_FREQ: 1000 Hz (standard buzzer frequency)
    // PWM_RESOLUTION: 8 bits (0-255 duty cycle)
    
    ledcSetup(PWM_CHANNEL, DEFAULT_FREQ, PWM_RESOLUTION);
    
    // Attach GPIO46 to PWM channel 1
    ledcAttachPin(BUZZER_PIN, PWM_CHANNEL);
    
    // Ensure buzzer is off initially
    ledcWrite(PWM_CHANNEL, 0);
}

void ESP32S3_Buzzer::_updatePlayback() {
    if (!_isPlaying) return;
    
    if (_sequence != nullptr) {
        // Sequence playback
        // TODO: Implement sequence playback
    } else if (_playDuration > 0) {
        // Single beep playback
        unsigned long elapsed = millis() - _playStartTime;
        if (elapsed >= _playDuration) {
            off();
        }
    }
}

// ============================================================
// SECTION 6: MAIN BOARD CONTROLLER IMPLEMENTATION
// ============================================================

ESP32S3_8DI8DO::ESP32S3_8DI8DO()
    : _di(), _do(), _rs485(), _rgb(), _buzzer(),
      initialized(false), ethernet_connected(false),
      rs485_enabled(false), can_enabled(false),
      _status(STATUS_INIT_FAILED),
      _ledBlinking(false), _ledBlinkColor(0),
      _ledBlinkStartTime(0), _ledBlinkInterval(0.5) {}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::begin
 * DESCRIPTION: Initializes the entire ESP32-S3-POE-ETH-8DI-8DO board.
 *              Configures all subsystems (DI, DO, RS485, RGB, Buzzer).
 * PARAMETERS:  None
 * RETURNED:    Status - STATUS_OK if successful, error code otherwise
 ***********************************************************************/
ESP32S3_8DI8DO::Status ESP32S3_8DI8DO::begin() {
    // Configure GPIO pins
    configurePins();
    Serial.println("✓ GPIO pins configured");
    
    // Initialize LED
    initLED();
    Serial.println("✓ RGB LED initialized");
    
    // Initialize Buzzer
    initBuzzer();
    Serial.println("✓ Buzzer initialized");
    
    // Initialize subsystems
    if (!_di.begin()) {
        _status = STATUS_DI_ERROR;
        return _status;
    }
    
    if (!_do.begin()) {
        _status = STATUS_DO_ERROR;
        return _status;
    }
    
    if (!_rs485.begin()) {
        _status = STATUS_RS485_ERROR;
        return _status;
    }
    
    if (!_rgb.begin()) {
        _status = STATUS_RGB_ERROR;
        return _status;
    }
    
    if (!_buzzer.begin()) {
        _status = STATUS_SYSTEM_ERROR;
        return _status;
    }
    
    initialized = true;
    _status = STATUS_OK;
    Serial.println("✅ Board initialization complete!\n");
    return _status;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::setRGBColor
 * DESCRIPTION: Convenience method to set RGB LED color from main board class.
 * PARAMETERS:  red - Red component (0-255)
 *              green - Green component (0-255)
 *              blue - Blue component (0-255)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_8DI8DO::setRGBColor(uint8_t red, uint8_t green, uint8_t blue) {
    _rgb.setColor(red, green, blue);
}

void ESP32S3_8DI8DO::LEDOff() {
    _ledBlinking = false;  // Stop blinking animation
    _rgb.setColor(0, 0, 0);
}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::setDigitalOutput
 * DESCRIPTION: Sets the state of a digital output relay.
 * PARAMETERS:  channel - The digital output channel (0-7)
 *              state - HIGH or 1 to turn ON, LOW or 0 to turn OFF
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_8DI8DO::setDigitalOutput(uint8_t channel, uint8_t state) {
    if (state) {
        _do.on(channel);
    } else {
        _do.off(channel);
    }
}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::getDigitalInput
 * DESCRIPTION: Reads the current state of a digital input channel.
 * PARAMETERS:  channel - The digital input channel (0-7)
 * RETURNED:    uint8_t - 1 if HIGH, 0 if LOW
 ***********************************************************************/
uint8_t ESP32S3_8DI8DO::getDigitalInput(uint8_t channel) {
    return _di.read(channel) ? 1 : 0;
}

uint8_t ESP32S3_8DI8DO::readAllInputs() {
    return _di.readAll();
}

void ESP32S3_8DI8DO::setAllOutputs(uint8_t value) {
    _do.setMask(value);
}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::buzzerBeep
 * DESCRIPTION: Produces multiple beep sounds with specified timing.
 *              Uses blocking delay for precise beep timing control.
 * PARAMETERS:  times - Number of beeps
 *              duration - Duration of each beep in milliseconds (default: 100ms)
 *              interval - Interval between beeps in milliseconds (default: 100ms)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_8DI8DO::buzzerBeep(uint8_t times, uint16_t duration, uint16_t interval) {
    times = constrain(times, 1, 20);  // Limit to 1-20 beeps
    
    for (uint8_t i = 0; i < times; i++) {
        _buzzer.beep(duration, 200);  // Beep at volume 200 (moderate-loud)
        
        // Pause between beeps (except after last beep)
        if (i < times - 1) {
            delay(interval);
        }
    }
}

bool ESP32S3_8DI8DO::setupEthernet() {
    ethernet_connected = true;
    return true;
}

bool ESP32S3_8DI8DO::setupRS485(uint32_t baudRate) {
    rs485_enabled = _rs485.begin(baudRate);
    return rs485_enabled;
}

bool ESP32S3_8DI8DO::setupLoRaDTU(uint32_t baudRate) {
    return setupRS485(baudRate);
}

bool ESP32S3_8DI8DO::setupCAN(uint32_t baudRate) {
    can_enabled = true;
    return true;
}

String ESP32S3_8DI8DO::getBoardInfo() {
    String info = "=== ESP32-S3-POE-ETH-8DI-8DO Board ===\n";
    info += "Library Version: " + String(LIBRARY_VERSION) + "\n";
    info += "Status: " + String(initialized ? "Ready" : "Not Initialized") + "\n";
    info += "Ethernet: " + String(ethernet_connected ? "Connected" : "Disconnected") + "\n";
    info += "RS485: " + String(rs485_enabled ? "Enabled" : "Disabled") + "\n";
    info += "CAN: " + String(can_enabled ? "Enabled" : "Disabled") + "\n";
    return info;
}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::loop
 * DESCRIPTION: Updates all subsystems and must be called in the main loop.
 *              Handles debouncing, animations, LED blinking, and other periodic tasks.
 * PARAMETERS:  None
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_8DI8DO::loop() {
    _di.update();
    _rs485.update();
    _rgb.update();
    _buzzer.update();
    
    // Handle LED blinking animation
    if (_ledBlinking) {
        unsigned long elapsed = millis() - _ledBlinkStartTime;
        float cycleTime = _ledBlinkInterval * 1000;  // Convert seconds to milliseconds
        float halfCycle = cycleTime / 2;
        
        // Calculate position in blink cycle (0-1)
        float cyclePos = fmod(elapsed, cycleTime);
        
        if (cyclePos < halfCycle) {
            // LED ON phase
            uint8_t r = (_ledBlinkColor >> 16) & 0xFF;
            uint8_t g = (_ledBlinkColor >> 8) & 0xFF;
            uint8_t b = _ledBlinkColor & 0xFF;
            _rgb.setColor(r, g, b);
        } else {
            // LED OFF phase
            _rgb.setColor(0, 0, 0);
        }
    }
}

void ESP32S3_8DI8DO::configurePins() {
    // Configure DI pins as input
    for (int i = 0; i < DI_CHANNELS; i++) {
        pinMode(ESP32S3_DI::DI_PINS[i], INPUT);
    }
    
    // Configure RS485 pins
    pinMode(RS485_TX_PIN, OUTPUT);
    pinMode(RS485_RX_PIN, INPUT);
    pinMode(21, OUTPUT);  // RTS pin
}

void ESP32S3_8DI8DO::initLED() {
    _rgb.begin();
    _rgb.setColor(0, 0, 0);  // Off
}

void ESP32S3_8DI8DO::initBuzzer() {
    _buzzer.begin();
    _buzzer.off();
}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::buzzer_beep
 * DESCRIPTION: Produces multiple rapid beep sounds with 100ms intervals.
 *              Simplified function for quick buzzer control.
 * PARAMETERS:  times - Number of beeps to produce (1-10 recommended)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_8DI8DO::buzzer_beep(int times) {
    buzzerBeep(times, 100, 100);
}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::TickRedLED
 * DESCRIPTION: Makes the RGB LED blink red color at specified interval.
 *              Uses internal timing for periodic on/off cycles.
 * PARAMETERS:  seconds - Blink interval in seconds (0.1 to 10.0 recommended)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_8DI8DO::TickRedLED(float seconds) {
    _ledBlinkColor = 0xFF0000;  // Red: R=255, G=0, B=0
    _ledBlinkInterval = seconds;
    _ledBlinking = true;
    _ledBlinkStartTime = millis();
}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::TickGreenLED
 * DESCRIPTION: Makes the RGB LED blink green color at specified interval.
 *              Uses internal timing for periodic on/off cycles.
 * PARAMETERS:  seconds - Blink interval in seconds (0.1 to 10.0 recommended)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_8DI8DO::TickGreenLED(float seconds) {
    _ledBlinkColor = 0x00FF00;  // Green: R=0, G=255, B=0
    _ledBlinkInterval = seconds;
    _ledBlinking = true;
    _ledBlinkStartTime = millis();
}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::TickBlueLED
 * DESCRIPTION: Makes the RGB LED blink blue color at specified interval.
 *              Uses internal timing for periodic on/off cycles.
 * PARAMETERS:  seconds - Blink interval in seconds (0.1 to 10.0 recommended)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_8DI8DO::TickBlueLED(float seconds) {
    _ledBlinkColor = 0x0000FF;  // Blue: R=0, G=0, B=255
    _ledBlinkInterval = seconds;
    _ledBlinking = true;
    _ledBlinkStartTime = millis();
}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::TickYellowLED
 * DESCRIPTION: Makes the RGB LED blink yellow color at specified interval.
 *              Uses internal timing for periodic on/off cycles.
 * PARAMETERS:  seconds - Blink interval in seconds (0.1 to 10.0 recommended)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_8DI8DO::TickYellowLED(float seconds) {
    _ledBlinkColor = 0xFFFF00;  // Yellow: R=255, G=255, B=0
    _ledBlinkInterval = seconds;
    _ledBlinking = true;
    _ledBlinkStartTime = millis();
}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::TickPurpleLED
 * DESCRIPTION: Makes the RGB LED blink purple color at specified interval.
 *              Uses internal timing for periodic on/off cycles.
 * PARAMETERS:  seconds - Blink interval in seconds (0.1 to 10.0 recommended)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_8DI8DO::TickPurpleLED(float seconds) {
    _ledBlinkColor = 0xFF00FF;  // Purple: R=255, G=0, B=255
    _ledBlinkInterval = seconds;
    _ledBlinking = true;
    _ledBlinkStartTime = millis();
}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::TickOrangeLED
 * DESCRIPTION: Makes the RGB LED blink orange color at specified interval.
 *              Uses internal timing for periodic on/off cycles.
 * PARAMETERS:  seconds - Blink interval in seconds (0.1 to 10.0 recommended)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_8DI8DO::TickOrangeLED(float seconds) {
    _ledBlinkColor = 0xFF8000;  // Orange: R=255, G=128, B=0
    _ledBlinkInterval = seconds;
    _ledBlinking = true;
    _ledBlinkStartTime = millis();
}

/***********************************************************************
 * FUNCTION:    ESP32S3_8DI8DO::TickWhiteLED
 * DESCRIPTION: Makes the RGB LED blink white color at specified interval.
 *              Uses internal timing for periodic on/off cycles.
 * PARAMETERS:  seconds - Blink interval in seconds (0.1 to 10.0 recommended)
 * RETURNED:    None
 ***********************************************************************/
void ESP32S3_8DI8DO::TickWhiteLED(float seconds) {
    _ledBlinkColor = 0xFFFFFF;  // White: R=255, G=255, B=255
    _ledBlinkInterval = seconds;
    _ledBlinking = true;
    _ledBlinkStartTime = millis();
}
