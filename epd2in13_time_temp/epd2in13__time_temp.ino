#include <SPI.h>
#include "epd2in13_V3.h"
#include "epdpaint.h"
#include "imagedata.h"
#include "DHT.h"
#include <WiFi.h>
#include "time.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>

#define COLORED 0
#define UNCOLORED 1
#define DHTPIN 2       // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11  // DHT 11

const int DST = 1;  // Daylight savings is **ON**

/**
  * Due to RAM not enough in Arduino UNO, a frame buffer is not allowed.
  * In this case, a smaller image buffer is allocated and you have to 
  * update a partial display several times.
  * 1 byte = 8 pixels, therefore you have to set 8*N pixels at a time.
  */
unsigned char image[4200];
Paint paint(image, 0, 0);
Epd epd;
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "myhomewifi";
//const char* ssid     = "myhomewifi 3";
const char* password = "Password123";

const char* ntpServer = "pool.ntp.org";
int daylightOffset_sec = 3600;
const long gmtOffset_sec = -28800 + DST * daylightOffset_sec;
const char* streamId = "tempdata.txt";
char replyPacket[] = "success!";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec);
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "10.0.0.11";
int port = 1883;
const char topic[] = "weather/inTemp";
StaticJsonDocument<200> doc;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println(WiFi.status());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(WiFi.status());
  }
  Serial.println("");
  Serial.print("WiFi connected: ");
  Serial.println(ssid);

  timeClient.begin();

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1)
      ;
  }

  Serial.print("epd FULL\n");
  epd.Init(FULL);
  //epd.Display(IMAGE_DATA);
  Serial.print("done epd Full\n");

  Paint paint(image, epd.bufwidth * 8, epd.bufheight);  //width should be the multiple of 8
  paint.SetRotate(0);

  paint.Clear(UNCOLORED);
  //paint.DrawStringAt(8, 2, "e-Paper 2.13", &Font8, COLORED);
  epd.Display(image);  //1
  dht.begin();
}
void disp_time_temp(float tempF) {
  char timedisp[10] = "         ";
  char tempdisp[20];
  String t = "";  //temporary variable due to arduino string weirdness

  timeClient.update();
  //t = timeClient.getFormattedTime();
  int h = timeClient.getHours();
  int m = timeClient.getMinutes();
  char ampm[] = "AM";
  if (h > 11) {
    ampm[0] = 'P';
    if (h > 12) {
      h = h - 12;
    }
  }

  sprintf_P(timedisp, PSTR("%d:%02d %s"), h, m, ampm);

  Serial.println(timedisp);
  /*
  String t1(tempF,1); //sprint is not an option on arduino
  t1.toCharArray(tempdisp, 20);
  */
  sprintf_P(tempdisp, PSTR("%.1f F"), tempF);
  Serial.print(tempdisp);
  Serial.print("\n");
  epd.Init(FULL);
  //  epd.Clear();
  Paint paint(image, epd.bufwidth * 8, epd.bufheight);  //width should be the multiple of 8
  paint.Clear(UNCOLORED);

  //use line 74 and 128
  paint.DrawStringAt(8, 74, timedisp, &Font20, COLORED);
  paint.DrawStringAt(8, 128, tempdisp, &Font24, COLORED);
  epd.Display(image);  //1

  //epd.Init(FULL);
  Serial.println("finished sending time/temp");
  //  epd.Clear();
  epd.Sleep();
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alive which
  // avoids being disconnected by the broker
  mqttClient.poll();
  float tempF = dht.readTemperature(true);
  doc["inTemp"] = tempF;
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
  disp_time_temp(tempF);
  delay(1000 * 60);  //120 = 2 minutes
}
