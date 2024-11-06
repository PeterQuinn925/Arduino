#include <SPI.h>
#include "epd2in13_V3.h"
#include "epdpaint.h"
#include "imagedata.h"
#include "DHT.h"
//#include <WiFiNINA.h>
#include <WiFi.h>
//#include "timeLib.h"
#include <NTPClient.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>
//#include <HttpClient.h> //this is different code from HTTPClient used on ESP chips. the DST code is different because of this
#include <ArduinoHttpClient.h>  //gave up using the standard lib. it didn't handle ports correctly

#define COLORED 0
#define UNCOLORED 1
#define DHTPIN 2       // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11  // DHT 11

int DST = 1;                          // Daylight savings 0 = Off, 1 = On. TODO:Automate this
char serverAddress[] = "10.0.0.11";  // server address
int apiPort = 8000;

unsigned char image[4200];
Paint paint(image, 0, 0);
Epd epd;
DHT dht(DHTPIN, DHTTYPE);

//const char* ssid = "Quinn and Cole";
const char* ssid = "Quinn and Cole 3";
const char* password = "ClevelandLulu";

const char* ntpServer = "pool.ntp.org";
int daylightOffset_sec = 3600;

const char* streamId = "tempdata.txt";
char replyPacket[] = "success!";

WiFiUDP ntpUDP;
WiFiClient wifiClient;
NTPClient timeClient(ntpUDP, ntpServer);
MqttClient mqttClient(wifiClient);
HttpClient HtppClient = HttpClient(wifiClient, serverAddress, apiPort);

//const char broker[] = "10.0.0.11";
IPAddress broker{ 10, 0, 0, 11 };
int port = 1883;
const char topic[] = "weather/inTemp";
StaticJsonDocument<200> doc;
int counter;
const unsigned long wdt = 600;  //watchdog timer timeout in seconds


void setup() {
  Serial.begin(9600);
  //while (!Serial)
  //delay(10);
  delay(100);
  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(WiFi.status());
  }
  Serial.println("");
  Serial.print("WiFi connected: ");
  Serial.println(ssid);
  getDST(DST);
  long gmtOffset_sec = -28800 + DST * daylightOffset_sec;
  timeClient.setTimeOffset(gmtOffset_sec);
  timeClient.begin();
  timeClient.update();
  //DST calc *****************************
  long epoch = timeClient.getEpochTime();  //work in progress
  //int day = time.day(epoch);
  //int month = time.month(epoch);
  //Serial.println (day,"/",month);
  Serial.println(timeClient.getFormattedTime());  //double check that we're connected to Wifi and the internet

  if (!mqttClient.connect(broker)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    Serial.println(broker);
    Serial.println(port);
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
  //int countdownMS = Watchdog.enable(1000000);  //set up a watchdog to reboot if there is a problem
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
  delay(10);  //shot in the dark - maybe there is a race condition that is causing intermittant crashes
  //  epd.Clear();
  Paint paint(image, epd.bufwidth * 8, epd.bufheight);  //width should be the multiple of 8
  paint.Clear(UNCOLORED);
  delay(10);  //shot in the dark - maybe there is a race condition that is causing intermittant crashes
  //use line 74 and 128
  paint.DrawStringAt(8, 74, timedisp, &Font20, COLORED);
  paint.DrawStringAt(8, 128, tempdisp, &Font24, COLORED);
  epd.Display(image);  //1
  delay(10);           //shot in the dark - maybe there is a race condition that is causing intermittant crashes
  //epd.Init(FULL);
  Serial.println("finished sending time/temp");
  //  epd.Clear();
  epd.Sleep();
}


void loop() {
  //Watchdog.reset();
  counter++;
  /* Turning off reset loop as the watchdog should take care of it now
   if (counter % 120 == 0) { //reset everything every 2 hr for now
     //WiFi.begin(ssid, password); //
     doc["reset"] = counter;
     char mqtt_msg[256];
     serializeJson(doc, mqtt_msg);
     mqttClient.beginMessage("weather/reset");
     mqttClient.print(mqtt_msg);
     mqttClient.endMessage();
     delay(1000);
     NVIC_SystemReset(); //this Wifi reset didn't help. trying to force a complete reboot instead
  }
  */
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
void getDST(int dst) {
  Serial.println("making GET request");
  HtppClient.get("/api/dst");

  // read the status code and body of the response
  int statusCode = HtppClient.responseStatusCode();
  String response = HtppClient.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

  DeserializationError error = deserializeJson(doc, response);

  dst = doc["dst"];
  DST = dst;
  Serial.println(DST);
}
