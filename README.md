# Smart Environmental & Water Tank Monitoring System

A wireless IoT system for monitoring environmental conditions (temperature, humidity, CO2) and water tank levels using ESP32 microcontrollers with ESP-NOW communication.




## Features
- **Sensor Node (Transmitter)**
  - DHT11 temperature/humidity sensor
  - HC-SR04 ultrasonic distance sensor (water level)
  - MQ-135 air quality sensor (CO2 levels)
  - RGB LED status indicator
  - Buzzer alerts for warnings
  - Button calibration system

- **Display Unit (Receiver)**
  - ILI9341 TFT display interface
  - Real-time data visualization
  - Warning system for critical conditions
  - Auto-calibration prompts
  - Connection status monitoring

## Hardware Requirements

### Sensor Node
- ESP32 microcontroller
- DHT11 sensor
- HC-SR04 ultrasonic sensor
- MQ-135 gas sensor
- Passive buzzer
- Tactile button
- Common cathode RGB LED
- Breadboard & jumper wires

### Display Unit
- ESP32 microcontroller
- ILI9341 2.8" TFT display
- Breadboard & jumper wires

## Installation

1. **Arduino IDE Setup**
   - Install ESP32 board support
   - Required libraries:
     - `DHT sensor library`
     - `ESP32 ESP-NOW`
     - `Adafruit SSD1306`
     - `Adafruit GFX Library`
     - `Adafruit ILI9341`

2. **Hardware Connections**

   **Sensor Node**
   
   DHT11      -> GPIO17
   HC-SR04:
     Trig     -> GPIO15
     Echo     -> GPIO18
   Buzzer     -> GPIO19
   Button     -> GPIO13 (pull-up)
   RGB LED:
     Red      -> GPIO14
     Green    -> GPIO12
     Blue     -> GPIO27
   MQ-135     -> GPIO35


 **Display Unit**

Copy
TFT_CS     -> GPIO5
TFT_RST    -> GPIO4
TFT_DC     -> GPIO16
TFT_MOSI   -> GPIO23
TFT_CLK    -> GPIO18


Configuration

ESP-NOW Setup
Update MAC addresses in both sketches:

// Transmitter code (replace with receiver MAC)
uint8_t receiverMacAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

// Receiver code (replace with transmitter MAC)
uint8_t expectedMac[] = {0x80,0x7D,0x3A,0xF8,0x1A,0x24};



Calibration

Initial calibration on first boot:
Hold button for 3 seconds
System measures empty tank distance
RGB LED confirms calibration
Usage

Sensor Node
Automatically sends data every 500ms
Button press for recalibration
Visual alerts:
White: Normal operation
Blinking blue: Calibration needed
Blinking red: Warning condition
Display Unit
Shows real-time sensor data
Warning displays for:
Tank overflow/empty
Critical CO2 levels
Connection status monitoring

Customization

Adjust thresholds in waterTankStatus()
Modify update intervals:
const unsigned long updateInterval = 500;  // Transmitter
const unsigned long displayTimeout = 5000; // Receiver
Enhance MQ-135 calibration (current implementation uses simple linear conversion)
Troubleshooting

Ensure MAC addresses match between devices
Check sensor wiring connections
Monitor serial output (115200 baud)
Reset both devices if connection drops

Note: This system requires line-of-sight between ESP32 devices for reliable ESP-NOW communication (range up to 200m in open space).


______________________________


## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE.md) file for details.


______________________________



