#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Define TFT pins
#define TFT_CS    5   // Chip select
#define TFT_RST   4   // Reset pin
#define TFT_DC    16  // Data/command pin
#define TFT_MOSI  23  // MOSI pin
#define TFT_CLK   18  // Clock pin

// Initialize ILI9341 display object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Structure to hold incoming sensor data
struct SensorData {
  float temperature;
  float humidity;
  float distance;
  float maxDistance;
  float co2;
};


// Create a variable to hold received data
SensorData receivedData;

// Timer for displaying no data message
unsigned long lastReceivedTime = 0;
const unsigned long displayTimeout = 5000;  // 5 seconds

// Previous data values to detect changes
float prevTemperature = -1;
float prevHumidity = -1;
float prevDistance = -1;
int prevLight = -1;
float prevCo2 = -1; 
//String prevStatus = "";

// Variable to track current display state
bool isWarningDisplayed = false;
bool isWaitingForData = false;  

// Function to display a connecting message
void displayConnectingMessage() {
  Serial.println("Displaying connecting message...");
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 120);
  tft.print("Connecting...");
}

// Function to display a connected message
void displayConnectedMessage() {
  Serial.println("Displaying connected message...");
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 120);
  tft.print("System Connected!");
  delay(2.000);  // Delay for 1 second to show the message
  redrawUI();   // After the connected message, restart the UI
}

// Function to determine water tank status based on distance percentage
String waterTankStatus(float distance) {
  float percentage = 100 * (1 - (distance / receivedData.maxDistance));

  if (distance < 3) {
    Serial.println("Warning: Tank Overflow detected");
    return "Warning: Tank Overflow";
  } else if (distance <= 5) {
    Serial.println("Warning: Tank Full detected");
    return "Warning: Tank Full";
  } else if (distance >= receivedData.maxDistance - 5) {
    Serial.println("Warning: Tank Empty detected");
    return "Warning: Tank Empty";
  }
  return "";  // No warning
}

// Function to draw a round-cornered rectangle for data display
void drawDataBox(int x, int y, String label, String value, bool updateOnly = false) {
  if (!updateOnly) {
    Serial.printf("Drawing data box at (%d, %d) for label: %s\n", x, y, label.c_str());
    tft.fillRoundRect(x, y, 130, 60, 10, ILI9341_WHITE);  // White rectangle
    tft.drawRoundRect(x, y, 130, 60, 10, ILI9341_WHITE);   // Border
  
    // Draw label (black)
    tft.setTextColor(ILI9341_BLACK);  
    tft.setTextSize(2);
    tft.setCursor(x + 10, y + 10);
    tft.print(label);
  }

  // Clear previous value area by drawing a white rectangle over it
  tft.fillRect(x + 10, y + 30, 120, 20, ILI9341_WHITE);

  // Draw value (blue)
  tft.setTextColor(ILI9341_BLUE);
  tft.setTextSize(2);
  tft.setCursor(x + 10, y + 30);
  tft.print(value);
}


// Function to display a warning message
void displayWarningMessage(String message) {
  Serial.printf("Displaying warning message: %s\n", message.c_str());
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(2);
  tft.setCursor(20, 120);
  tft.print(message);
  isWarningDisplayed = true;  // Set warning displayed state
}


// Function to redraw the entire UI after a warning or no-data state is cleared
void redrawUI() {
  Serial.println("Redrawing UI...");
  tft.fillScreen(ILI9341_BLACK);  // Clear screen

  // Header
  tft.setTextColor(ILI9341_BLUE);
  tft.setTextSize(3);
  tft.setCursor(60, 3);
  tft.println("SmartMonitor");

  // Draw static layout boxes
  drawDataBox(20, 30, "Temp:", String(receivedData.temperature)+ "C");   // First row, first column
  drawDataBox(170, 30, "Hum:", String(receivedData.humidity) + "%");      // First row, second column
  drawDataBox(20, 100, "Tank:", String(receivedData.distance) + "%");     // Second row, first column
  drawDataBox(170, 100, "CO2:", String(receivedData.co2) + "ppm");     // Second row, second column
  drawDataBox(20, 170, "Tank Size:", String(receivedData.maxDistance) + "cm");           // third row, first column
  drawDataBox(170, 170, "Status:", "OK");           // third row, second column


}

// Function to display the received data on the ILI9341 TFT
void displayReceivedData() {
  Serial.println("Displaying received data...");
  String status = waterTankStatus(receivedData.distance);
  
  // Check for warnings
  if (status.startsWith("Warning")) {
    displayWarningMessage(status);
    return;  // Skip normal data display
  }


  // If warning or "Waiting for Data..." was previously displayed, clear it and refresh UI
  if (isWarningDisplayed || isWaitingForData) {
    redrawUI();  // Re-draw entire UI after clearing the message
    isWarningDisplayed = false;   // Reset the warning state
    isWaitingForData = false;     // Reset the waiting state
  }
  
  // Calculate the percentage for distance
  float distancePercentage = 100 * (1 - (receivedData.distance / receivedData.maxDistance));

  // Update sensor values only if they have changed
  // Show distance as a percentage
  if (distancePercentage != prevDistance) {
  drawDataBox(20, 100, "Tank:", String(distancePercentage, 1) + "%", true);  // Display with 1 decimal place
  prevDistance = distancePercentage;  // Update with percentage value instead of raw distance
}

  if (receivedData.humidity != prevHumidity) {
    drawDataBox(170, 30, "Hum:", String(receivedData.humidity) + "%", true);
    prevHumidity = receivedData.humidity;
  }

     if (receivedData.co2 != prevCo2) {
    drawDataBox(170, 100, "CO2:", String(receivedData.co2) + "ppm", true);
    prevCo2 = receivedData.co2;
  }

  if (receivedData.temperature != prevTemperature) {
    drawDataBox(20, 30, "Temp:", String(receivedData.temperature) + "C", true);
    prevTemperature = receivedData.temperature;
  }

    if (receivedData.maxDistance != prevDistance) {
    drawDataBox(20, 170, "Tank Size:", String(receivedData.maxDistance) + "cm", true);
  }


   drawDataBox(170, 170, "Status:", "OK", true);

}

// Function to display no data message
void displayNoDataMessage() {
  if (!isWaitingForData) {
    Serial.println("Displaying no data message...");
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setCursor(20, 120);
    tft.print("Calibration Needed!");
    isWaitingForData = true;  // Set the flag to indicate we're displaying the no data message
  }
}

// Callback function to receive ESP-NOW data

void onDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  uint8_t expectedMac[] = {0x80, 0x7D, 0x3A, 0xF8, 0x1A, 0x24};  // Sender ESP32 MAC address

  // Compare received sender's MAC with the expected sender

  if (memcmp(info->src_addr, expectedMac, 6) != 0) {
    Serial.println("Received packet from unknown device, ignoring...");
    return;  // Ignore data from unknown devices
  }

    memcpy(&receivedData, incomingData, sizeof(receivedData));
    // Validate received data

/*

if (
    // Check if any value is NaN or out of range
    isnan(receivedData.temperature) || receivedData.temperature < -40 || receivedData.temperature > 125 ||
    isnan(receivedData.humidity) || receivedData.humidity < 0 || receivedData.humidity > 100 ||
    isnan(receivedData.distance) || receivedData.distance < 0 || receivedData.distance > 600 ||
    isnan(receivedData.maxDistance) || receivedData.maxDistance < 0 || receivedData.maxDistance > 600 ||
    isnan(receivedData.light) || receivedData.light < 0 || receivedData.light > 5000
) {

  Serial.println("Received invalid data, ignoring...");
  Serial.print("Received data from: ");

for (int i = 0; i < 6; i++) {
  Serial.printf("%02X", info->src_addr[i]);
  if (i < 5) Serial.print(":");
}
Serial.println();

    return;  // Ignore invalid data
}
*/

lastReceivedTime = millis();
Serial.println();

    Serial.printf("Temperature: %.2f, Humidity: %.2f, Distance: %.2f, Light: %.2f, Max Distance: %.2f\n, CO2: %.2f\n",
                  receivedData.temperature, receivedData.humidity, receivedData.distance,
                  receivedData.maxDistance, receivedData.co2);
    displayReceivedData();
}


void setup() {
  Serial.begin(115200);

  // Initialize the ILI9341 TFT display
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  
  // Display connecting message
  displayConnectingMessage();
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW initialized");

  // Display connected message
  displayConnectedMessage();

  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  if (millis() - lastReceivedTime > displayTimeout) {
    displayNoDataMessage();  // Show no data message if no data received within timeout
  } else {
    // If data arrives, clear "no data" message and refresh the UI
    if (isWaitingForData) {
      isWaitingForData = false;  // Reset the state
      redrawUI();  // Re-draw the normal UI
    }
  }
}

