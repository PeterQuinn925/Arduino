//*****Make sure to use the WiFi LoRa 32(v3) board under the Heltec pull down. Otherwise will not work!!!!
#include "Arduino.h"
#include "HT_st7735.h"
#include "HT_TinyGPS++.h"
#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <LoRaWan_APP.h>
#include <time.h>
#include <LittleFS.h>

TinyGPSPlus GPS;
HT_st7735 st7735;

#define VGNSS_CTRL        3
#define DISPLAY_POWER_PIN 21
#define uS_TO_S_FACTOR    1000000ULL
#define TIME_TO_SLEEP     60  // seconds
#define SEND_INTERVAL     60000  // ms between live sends in USB mode
#define ADC_CTRL_PIN 37

const char* ssid     = "Quinn and Cole";
const char* password = "XXXXXXXXXX";

//IPAddress broker(10, 0, 0, 11); //local Raspi4
const char* broker = "archiethecat.duckdns.org"; //using DuckDNS to get to Oracle Cloud server
IPAddress local_IP(10, 0, 0, 150);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

int port = 1883;
const char topic[] = "location/test";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// --- WiFi + MQTT ---
bool connectAll() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    WiFi.config(local_IP, gateway, subnet, dns);
    WiFi.begin(ssid, password);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      attempts++;
    }
    if (WiFi.status() != WL_CONNECTED) return false;
  }
  if (!mqttClient.connected()) {
    if (!mqttClient.connect(broker, port)) return false;
  }
  return true;
}

// --- GPS epoch ---
time_t getGPSEpoch(TinyGPSPlus& gps) {
  if (!gps.date.isValid() || !gps.time.isValid()) return 0;
  struct tm t;
  t.tm_year  = gps.date.year() - 1900;
  t.tm_mon   = gps.date.month() - 1;
  t.tm_mday  = gps.date.day();
  t.tm_hour  = gps.time.hour();
  t.tm_min   = gps.time.minute();
  t.tm_sec   = gps.time.second();
  t.tm_isdst = 0;
  return mktime(&t);
}

// --- Flash storage ---
void saveToFlash(float lat, float lng, time_t epoch) {
  File file = LittleFS.open("/gps_log.txt", FILE_APPEND);
  if (!file) return;
  StaticJsonDocument<128> doc;
  doc["lat"]  = lat;
  doc["lng"]  = lng;
  doc["time"] = (long)epoch;
  serializeJson(doc, file);
  file.println();
  file.close();
}

void flushToMQTT() {
  if (!LittleFS.exists("/gps_log.txt")) return;
  File file = LittleFS.open("/gps_log.txt", FILE_READ);
  if (!file) return;
  bool allSent = true;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;
    if (!mqttClient.connected()) { allSent = false; break; }
    mqttClient.poll();
    mqttClient.beginMessage(topic);
    mqttClient.print(line);
    mqttClient.endMessage();
    delay(100);
  }
  file.close();
  if (allSent) LittleFS.remove("/gps_log.txt");
}

// --- Display ---
void updateDisplay(bool isConnected, bool isFlushing) {

  st7735.st7735_fill_screen(ST7735_BLACK);

  // Line 1 - WiFi
  st7735.st7735_write_str(0, 20, isConnected ? "WiFi: Online" : "WiFi: Offline");

  // Line 2 - GPS
  if (GPS.location.isValid()) {
    st7735.st7735_write_str(0, 40, "GPS: Fix OK");
  } else {
    st7735.st7735_write_str(0, 40, "GPS: Waiting...");
  }

  // Line 3 - Backfill / stored count
  if (isFlushing) {
    st7735.st7735_write_str(0, 60, "Backfilling...");
  } else if (LittleFS.exists("/gps_log.txt")) {
    File f = LittleFS.open("/gps_log.txt", FILE_READ);
    int lines = 0;
    while (f.available()) { if (f.read() == '\n') lines++; }
    f.close();
    st7735.st7735_write_str(0, 60, "Stored: " + String(lines));
  } else {
    st7735.st7735_write_str(0, 60, "Live OK");
  }
}

// --- USB mode: stay awake, show display, send on interval ---
void runUSBMode() {
  bool wasConnected = false;
  bool isFlushing   = false;
  unsigned long lastSent    = 0;
  unsigned long lastDisplay = 0;

  while (true) {
    // Feed GPS
    while (Serial1.available() > 0) {
      GPS.encode(Serial1.read());
    }

    bool isConnected = connectAll();
    mqttClient.poll();

    // Flush stored data on first connection or reconnection
    if (isConnected && !wasConnected && LittleFS.exists("/gps_log.txt")) {
      isFlushing = true;
      updateDisplay(isConnected, true);
      flushToMQTT();
      isFlushing = false;
    }
    wasConnected = isConnected;

    // Send live GPS on interval, not every update
    if (GPS.location.isValid() && GPS.time.isValid()) {
      if (millis() - lastSent >= SEND_INTERVAL) {
        if (isConnected) {
          StaticJsonDocument<128> doc;
          doc["lat"]  = GPS.location.lat();
          doc["lng"]  = GPS.location.lng();
          doc["time"] = (long)getGPSEpoch(GPS);
          String payload;
          serializeJson(doc, payload);
          mqttClient.beginMessage(topic);
          mqttClient.print(payload);
          mqttClient.endMessage();
        } else {
          saveToFlash(GPS.location.lat(), GPS.location.lng(), getGPSEpoch(GPS));
        }
        lastSent = millis();
      }
    }

    // Update display every 2 seconds
    if (millis() - lastDisplay > 2000) {
      updateDisplay(isConnected, isFlushing);
      lastDisplay = millis();
    }
  }
}

// --- Battery mode: get fix, send, sleep ---
void runBatteryMode() {

unsigned long gpsStart = millis();
int validFixCount = 0;
const int REQUIRED_FIXES = 3;  // Tune this: 2-3 is a good balance

double lastLat = 0, lastLng = 0;
time_t lastEpoch = 0;

while (millis() - gpsStart < 90000) {
  while (Serial1.available() > 0) {
    GPS.encode(Serial1.read());
  }

  if (GPS.location.isValid() && GPS.time.isValid() && GPS.hdop.hdop() < 2.0) {
    // Only count if location has actually updated since last fix
    if (GPS.location.isUpdated()) {
      validFixCount++;
      lastLat   = GPS.location.lat();
      lastLng   = GPS.location.lng();
      lastEpoch = getGPSEpoch(GPS);
    }
    if (validFixCount >= REQUIRED_FIXES) break;
  }
}

if (validFixCount > 0) {
  float lat   = lastLat;
  float lng   = lastLng;
  time_t epoch = lastEpoch;

    bool isConnected = connectAll();
    if (isConnected) {
      flushToMQTT();
      StaticJsonDocument<128> doc;
      doc["lat"]  = lat;
      doc["lng"]  = lng;
      doc["time"] = (long)epoch;
      String payload;
      serializeJson(doc, payload);
      mqttClient.beginMessage(topic);
      mqttClient.print(payload);
      mqttClient.endMessage();
      delay(500);
    } else {
      saveToFlash(lat, lng, epoch);
    }
  }
}

// --- Setup ---
void setup() {

  Serial1.begin(115200, SERIAL_8N1, 33, 34);
  LittleFS.begin(true);
  setenv("TZ", "UTC0", 1);
  tzset();

  // Init display AFTER power pin is HIGH
  //in theory, this is the backlight pin and this should turn it off
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);  // Backlight OFF (cathode pin — HIGH = off)
  st7735.st7735_init();
  digitalWrite(ST7735_CS_Pin, LOW);
  digitalWrite(ST7735_DC_Pin, LOW);  // Command mode
  SPI.transfer(0x28);                // DISPOFF
  digitalWrite(ST7735_CS_Pin, HIGH);
  //st7735.st7735_fill_screen(ST7735_BLACK);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
//  bool usbPower = isUSBPowered(); Removed, doesn't work. hardcoding
bool usbPower = false; //normal is false. true for debugging

  // Show raw voltage for calibration - remove once threshold is tuned
  //st7735.st7735_write_str(0, 20, "V: " + String(getBatteryVoltage(), 3));

  if (usbPower) {
    runUSBMode();
  } else {
    digitalWrite(DISPLAY_POWER_PIN, LOW);
    runBatteryMode();
    esp_deep_sleep_start();
  }
}

void loop() {
  // Never reached
}
