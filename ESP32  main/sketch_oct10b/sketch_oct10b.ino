#include <DHT.h>
#include <WiFi.h>
#include "SSD1306Wire.h"
#include <esp_now.h>

// DHT11 settings
#define DHTPIN 17
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// HC-SR04 settings
#define TRIG_PIN 15
#define ECHO_PIN 18

// Buzzer settings
#define BUZZER_PIN 19

// Button settings
#define BUTTON_PIN 13

// RGB Light settings
#define RED_PIN 14
#define GREEN_PIN 12
#define BLUE_PIN 27

// MQ-135 Sensor
#define MQ135_PIN 35

unsigned long buttonPressTime = 0;
bool calibrating = false;
float maxDistance = 25.0;  // Default max distance
bool calibrationRequired = true;  // Start with calibration required
unsigned long buttonPressStartTime = 0; // Track the button press time for re-calibration
bool reCalibrationPrompt = false; // Flag to show re-calibration prompt

// ESP-NOW receiver MAC address
uint8_t receiverMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Structure for data to send´
struct SensorData {
  float temperature;
  float humidity;
  float distance;
  float maxDistance;
  float co2; 
};

SensorData myData;

// Timing variables
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 500;  // 0.5 second

// Function to measure distance using the HC-SR04
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

// Initialize the OLED display
SSD1306Wire display(0x3C, 21, 22);


// Function to determine water tank status based on distance percentage
String waterTankStatus(float distance) {
  float percentage = 100 * (1 - (distance / maxDistance));

  if (distance < 3) {
    return "Warning: Tank Overflow";
  } else if (distance <= 5) {
    return "Warning: Tank Full";
  } else if (distance >= maxDistance - 5) {
    return "Warning: Tank Empty";
  }
  return "";  // No warning
}


// Function to handle calibration at startup
void handleStartupCalibration() {
  // Display the calibration prompt
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 8, "Calibration Needed");
  display.drawString(64, 32, "Hold button 3 sec");
  display.display();

  // Blink blue during calibration
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);

  if (millis() % 1000 < 500) {
    digitalWrite(BLUE_PIN, HIGH);  // Blue on for 500ms
  } else {
    digitalWrite(BLUE_PIN, LOW);   // Blue off for 500ms
  }

  // Check if the button is pressed
  if (digitalRead(BUTTON_PIN) == LOW) {
    // Start timing the button press
    if (buttonPressTime == 0) {
      buttonPressTime = millis();
    }

    // If button is held for 3 seconds, start calibration
    if (millis() - buttonPressTime >= 3000) {
      calibrating = true;
      maxDistance = getDistance();  // Measure distance for max distance
      buttonPressTime = 0;  // Reset button press 
      
      // Show max distance and reset RGB to white
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, HIGH);

      display.clear();
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.drawString(64, 8, "Max Distance Set!");
      display.drawString(64, 32, "Water Tank Size: " + String(maxDistance) + " cm");
      display.display();
      Serial.println("Max Distance Set: " + String(maxDistance) + " cm");
      delay(5000);  // Show the max distance for 5 seconds
      calibrationRequired = false;  // Calibration complete, show normal data
    }
  } else {
    // Reset button press time when button is released
    buttonPressTime = 0;
  }
}

// Callback function when data is sent
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Function to Include MQ-135 Readings
float getCO2Concentration() {
  int analogValue = analogRead(MQ135_PIN);
  
  // Convert the analog value to CO2 concentration (ppm)
  // You may need a proper calibration equation based on your setup, here’s a simple approximation:
  float ppm = analogValue * (5000.0 / 4095.0);  // Assuming 5V ADC and 12-bit resolution
  return ppm;
}


// Function to send data via ESP-NOW
void sendData() {
  // Prepare sensor data
  myData.temperature = dht.readTemperature();
  myData.humidity = dht.readHumidity();
  myData.distance = getDistance();
  myData.maxDistance = maxDistance;
//  myData.calibrationRequest = false; 

  // Read CO2 concentration from MQ-135
  float co2Concentration = getCO2Concentration();
  myData.co2 = co2Concentration;

  // Send data via ESP-NOW
  esp_err_t result = esp_now_send(receiverMacAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Error sending data");
  }

  // Print data being sent
  Serial.print("Temperature: ");
  Serial.println(myData.temperature);
  Serial.print("Humidity: ");
  Serial.println(myData.humidity);
  Serial.print("Distance: ");
  Serial.println(myData.distance);
  Serial.print("CO2 (ppm): ");
  Serial.println(co2Concentration);
}

void displaySensorData() {
  String status = waterTankStatus(myData.distance);

  // Calculate the percentage for distance
  float distancePercentage = 100 * (1 - (myData.distance / maxDistance));

  if (status != "") {
    // Blink red during warnings
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);

    if (millis() % 1000 < 500) {
      digitalWrite(RED_PIN, HIGH);  // Red on for 500ms
    } else {
      digitalWrite(RED_PIN, LOW);   // Red off for 500ms
    }

    // Handle the buzzer logic
    if (myData.distance > maxDistance - 5) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);
      delay(200);
    } else if (myData.distance < 5) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(50);
      digitalWrite(BUZZER_PIN, LOW);
      delay(50);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }

    // Update the OLED display
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 8, "WARNING");
    display.drawString(64, 32, status);
    Serial.println("WARNING: " + status);
  } else {
    // Normal display when no warning
    // Set RGB to white
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, HIGH);

    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    //display.drawString(36, 0, "SmartMonitor");
    display.drawString(0, 0, "Temp: " + String(myData.temperature) + " C");
    display.drawString(0, 12, "Hum: " + String(myData.humidity) + " %");
    display.drawString(0, 24, "Tank: " + String(distancePercentage, 1) + " %");
    display.drawString(0, 36, "CO2: " + String(myData.co2) + " ppm");
    display.drawString(0, 48, "Tank Size: " + String(myData.maxDistance) + " CM");

    Serial.print("Displaying Data -> Temp: ");
    Serial.print(myData.temperature);
    Serial.print(" C, Hum: ");
    Serial.print(myData.humidity);
    Serial.print(" %, Dist: ");
    Serial.print(myData.distance);

    Serial.print(", CO2: ");
    Serial.println(myData.co2);
  }

  display.display();
}

void setup() {
  Serial.begin(115200);

  // Initialize sensors and pins
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Button pin setup

  // Set RGB pins as outputs
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Turn on white light to indicate the device is working
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, HIGH);

  // Initialize the OLED display
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();

  // Initialize Wi-Fi in Station Mode for ESP-NOW
  WiFi.mode(WIFI_STA);
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW initialized");

  // Register send and receive callback functions
  esp_now_register_send_cb(onDataSent);

  // Add the peer (receiver ESP32)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  Serial.println("Peer added successfully");

  // Perform initial calibration
  handleStartupCalibration();
}

void loop() {
  if (calibrationRequired) {
    handleStartupCalibration();  // Perform startup calibration
  } else {

    // Update sensor readings and send data every updateInterval
    if (millis() - lastUpdateTime >= updateInterval) {
      sendData();
      displaySensorData();
      lastUpdateTime = millis();
    }
  }
}
