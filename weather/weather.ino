#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// --- WiFi Configuration ---
// Replace with your actual WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// --- Weather API Configuration ---
// Replace YOUR_CITY with your city name. Use URL encoding for spaces (e.g., "New%20York").
const char* weatherApiUrl = "https://wttr.in/YOUR_CITY?format=j1"; 
const long updateInterval = 15 * 60 * 1000; // 15 minutes

// --- NTP (Time) Configuration ---
// Set your UTC offset in seconds. 
// Examples: UTC+1 is 3600, UTC+2 is 7200.
// CET (Central European Time) is UTC+1, CEST (Central European Summer Time) is UTC+2.
const long utcOffsetInSeconds = 7200; // Defaulting to UTC+2 (CEST)

// --- Object Initialization ---
TFT_eSPI tft = TFT_eSPI(320, 240);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Variable to track the last weather update time
unsigned long previousWeatherMillis = 0;

// --- Color Definitions (BGR format) ---
uint16_t colorBGR(uint8_t r, uint8_t g, uint8_t b) {
  return ((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r >> 3);
}
#define COLOR_TEAL colorBGR(0, 128, 128)
#define COLOR_YELLOW colorBGR(255, 255, 0)

// --- Helper function to draw wrapped text, centered ---
int drawWrappedTextCentered(String text, int startY, int maxWidth, int textSize, int lineSpacing) {
  tft.setTextSize(textSize);
  tft.setTextDatum(MC_DATUM);

  String line = "", word = "";
  int y = startY;
  int linesDrawn = 0;

  for (int i = 0; i <= text.length(); i++) {
    char c = text[i];
    if (c == ' ' || i == text.length()) {
      if (i == text.length()) word += c;
      int lineWidth = tft.textWidth(line + word, textSize);
      if (lineWidth > maxWidth && line.length() > 0) {
        tft.drawString(line, tft.width() / 2, y);
        y += textSize * 8 + lineSpacing;
        linesDrawn++;
        line = word + " ";
      } else {
        line += word + " ";
      }
      word = "";
    } else {
      word += c;
    }
  }
  if (line.length() > 0) {
    tft.drawString(line, tft.width() / 2, y);
    linesDrawn++;
  }
  return linesDrawn * (textSize * 8 + lineSpacing);
}

// --- Function to display date and time ---
void displayTime() {
    // Frame settings
    int box_h = 28;
    int box_margin_x = 10;
    int box_margin_y = 4;
    int box_x = box_margin_x;
    int box_y = tft.height() - box_h - box_margin_y;
    int box_w = tft.width() - 2 * box_margin_x;
    int corner_radius = 5;

    // Font and alignment settings
    tft.setTextDatum(MC_DATUM); // Middle-Center alignment
    tft.setTextSize(2);
    tft.setTextColor(COLOR_YELLOW, COLOR_TEAL);

    // Get formatted time (HH:MM:SS)
    String formattedTime = timeClient.getFormattedTime();

    // Get date
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime((time_t *)&epochTime);
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon + 1;
    int currentYear = ptm->tm_year + 1900;
    String currentDate = String(monthDay) + "." + String(currentMonth) + "." + String(currentYear);

    String dateTimeStr = currentDate + "  " + formattedTime;

    // Clear the area for the frame
    tft.fillRect(box_x, box_y, box_w, box_h, COLOR_TEAL);
    
    // Draw the rounded frame
    tft.drawRoundRect(box_x, box_y, box_w, box_h, corner_radius, COLOR_YELLOW);
    
    // Draw the date and time text inside the frame
    tft.drawString(dateTimeStr, tft.width() / 2, box_y + box_h / 2);
}

// --- Function to fetch and display weather ---
void getAndDisplayWeather() {
  HTTPClient http;
  http.begin(weatherApiUrl);
  int httpCode = http.GET();

  // Clear the entire screen only on weather update
  tft.fillScreen(COLOR_TEAL);
  tft.setTextColor(COLOR_YELLOW, COLOR_TEAL);

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      const char* temp_C = doc["current_condition"][0]["temp_C"];
      const char* feelsLike_C = doc["current_condition"][0]["FeelsLikeC"];
      const char* humidity = doc["current_condition"][0]["humidity"];
      const char* weatherDesc = doc["current_condition"][0]["weatherDesc"][0]["value"];
      const char* areaName = doc["nearest_area"][0]["areaName"][0]["value"];
      
      int y = 10;
      tft.setTextDatum(MC_DATUM);

      // City
      tft.setTextSize(3);
      tft.drawString(String(areaName), tft.width() / 2, 20);
      y += 3 * 8 + 8;

      // Weather description (with wrapping)
      int usedHeight = drawWrappedTextCentered(String(weatherDesc), y, tft.width() - 20, 2, 4);
      y += usedHeight + 8;

      // Temperature and wind
      const char* windSpeed = doc["current_condition"][0]["windspeedKmph"];
      const char* windDir = doc["current_condition"][0]["winddir16Point"];
      tft.setTextSize(3);
      tft.drawString(String(temp_C) + "C " + String(windSpeed) + "km/h " + String(windDir), tft.width() / 2, y);
      y += 3 * 8 + 8;

      // Forecast for tomorrow
      tft.setTextSize(2);
      const char* tomorrow_maxtempC = doc["weather"][1]["maxtempC"];
      const char* tomorrow_mintempC = doc["weather"][1]["mintempC"];
      const char* tomorrow_windSpeed = doc["weather"][1]["hourly"][4]["windspeedKmph"];
      const char* tomorrow_windDir = doc["weather"][1]["hourly"][4]["winddir16Point"];
      tft.drawString("Tomorrow: " + String(tomorrow_maxtempC) + "C/" + String(tomorrow_mintempC) + "C", tft.width() / 2, y);
      y += 2 * 8 + 2;
      tft.drawString("Wind: " + String(tomorrow_windSpeed) + " km/h " + String(tomorrow_windDir), tft.width() / 2, y);
      y += 2 * 8 + 8;

      // Forecast for the day after tomorrow
      const char* dayAfter_maxtempC = doc["weather"][2]["maxtempC"];
      const char* dayAfter_mintempC = doc["weather"][2]["mintempC"];
      const char* dayAfter_windSpeed = doc["weather"][2]["hourly"][4]["windspeedKmph"];
      const char* dayAfter_windDir = doc["weather"][2]["hourly"][4]["winddir16Point"];
      tft.drawString("Day after: " + String(dayAfter_maxtempC) + "C/" + String(dayAfter_mintempC) + "C", tft.width() / 2, y);
      y += 2 * 8 + 2;
      tft.drawString("Wind: " + String(dayAfter_windSpeed) + " km/h " + String(dayAfter_windDir), tft.width() / 2, y);

    } else {
      tft.setTextDatum(TL_DATUM);
      tft.drawString("JSON parsing error", 10, 10);
    }
  } else {
    tft.setTextDatum(TL_DATUM);
    tft.drawString("HTTP Error: " + String(httpCode), 10, 10);
  }
  http.end();
}

// --- Main Program ---
void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(COLOR_TEAL);
  tft.setTextColor(COLOR_YELLOW, COLOR_TEAL);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");

  // Start NTP client
  timeClient.begin();

  // Fetch and display weather for the first time
  getAndDisplayWeather();
  previousWeatherMillis = millis(); // Save the time of the last update
}

void loop() {
  // Check if it's time to update the weather
  unsigned long currentMillis = millis();
  if (currentMillis - previousWeatherMillis >= updateInterval) {
    previousWeatherMillis = currentMillis;
    getAndDisplayWeather();
  }

  // Update and display the time every second
  timeClient.update();
  displayTime();

  delay(1000); // Wait 1 second
}
