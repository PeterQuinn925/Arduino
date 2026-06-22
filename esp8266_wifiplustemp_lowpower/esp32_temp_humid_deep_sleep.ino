//TODO: deep sleep and battery
//TODO: case and mount
//TODO: change weewx to use MQTT for everything (do it in a VM)

#include "DHT.h"
#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
#include <Wire.h>

#define DHTPIN 23                  // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22              // DHT 11
#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 120           // Time ESP32 will go to sleep (in seconds)

char serverAddress[] = "10.0.0.11";  // server address

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Quinn and Cole";
//const char* ssid = "Quinn and Cole 3";
const char* password = "ClevelandLulu";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

//const char broker[] = "10.0.0.11";
IPAddress broker{ 10, 0, 0, 11 };
int port = 1883;
const char topic[] = "weather/outTemp";
StaticJsonDocument<200> doc;
int counter;
const unsigned long wdt = 600;  //watchdog timer timeout in seconds

void setup() {
  delay(100);
  Serial.begin(9600);
  delay(100);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(WiFi.status());
  }
  Serial.println("");
  Serial.print("WiFi connected: ");
  Serial.println(ssid);
  int count = 0;
  if (!mqttClient.connect(broker)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    Serial.println(broker);
    Serial.println(port);
    //might as well reboot if we're stuck
    esp_deep_sleep_start();
    while (1)
      ;
  }
  dht.begin();
  mqttClient.poll();
  delay(1000);
  float tempF = 0;
  float humid = 0;
  tempF = dht.readTemperature(true);
  humid = dht.readHumidity();
  Serial.print("tempF: ");
  Serial.println(tempF);
  Serial.print("humid: ");
  Serial.println(humid);

  doc["outTemp"] = tempF;
  doc["humid"] = humid;

  char mqtt_msg[256];
  serializeJson(doc, mqtt_msg);
  if (mqttClient.connected()) {
    mqttClient.beginMessage(topic);
    mqttClient.print(mqtt_msg);
    mqttClient.endMessage();
  } else {  //reconnect and retry
    Serial.print("connection error");
    mqttClient.connect(broker, port);
    mqttClient.beginMessage(topic);
    mqttClient.print(mqtt_msg);
    mqttClient.endMessage();
  }

  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush();
  
  esp_deep_sleep_start();
}

void loop() {}
/*
  //Watchdog.reset();
  mqttClient.poll();
  delay(100);
  float tempF = 0;
  float humid =0;
  tempF = dht.readTemperature(true);
  humid = dht.readHumidity();
  Serial.print("tempF: ");
  Serial.println(tempF);
  Serial.print("humid: ");
  Serial.println(humid);

  doc["outTemp"] = tempF;
  doc["humid"] = humid;

  char mqtt_msg[256];
  serializeJson(doc, mqtt_msg);
  if (mqttClient.connected()) {
    mqttClient.beginMessage(topic);
    mqttClient.print(mqtt_msg);
    mqttClient.endMessage();
  } else {  //reconnect and retry
    Serial.print("connection error");
    mqttClient.connect(broker, port);
    mqttClient.beginMessage(topic);
    mqttClient.print(mqtt_msg);
    mqttClient.endMessage();
  }
  
    delay(1000 * 60);  //120 = 2 minutes
}

*/
