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
   ```plaintext
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
