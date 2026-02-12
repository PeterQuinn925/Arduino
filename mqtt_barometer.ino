#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>

Adafruit_BMP085 bmp;

char serverAddress[] = "10.0.0.11";  // server address
//const char* ssid = "Quinn and Cole";
const char* ssid = "Quinn and Cole";
const char* password = "Cave1andLu1u";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

//const char broker[] = "10.0.0.11";
IPAddress broker{ 10, 0, 0, 11 };
int port = 1883;
const char topic[] = "weather/barometer";
StaticJsonDocument<200> doc;

void setup() {
  Serial.begin(9600);
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085/BMP180 sensor, check wiring!");
    while (1) {}
  }
delay(100);
WiFi.mode(WIFI_STA);
WiFi.disconnect();
delay(100);
WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(WiFi.status());
  }
  Serial.println("");
  Serial.print("WiFi connected: ");
  Serial.println(ssid);

  if (!mqttClient.connect(broker)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    Serial.println(broker);
    Serial.println(port);
    while (1)
      ;
  }
}

void loop() {
  Serial.print("Temperature = ");
  double t = bmp.readTemperature();
  double t_degF = (t * 9 / 5) + 32;
  Serial.print(t_degF);
  Serial.println(" *F");

  Serial.print("Pressure = ");
  double p = bmp.readPressure();
  double p_inhg = p / 3386.3886666667;
  Serial.print(p_inhg);
  Serial.println(" in Hg");
  // we need to send the baro pressure in mbar, I think

  doc["inTemp"] = t_degF;
  doc["pressure"] = p /100;
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
    Serial.println();
  }
  delay(60000);  //60 sec
}
