/*Use NodeMCU-32S in adafruit's ESP32
if not available, add the following to File/Preferences/Additional Boards Manaager
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
*/
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <WiFi.h>
#include "time.h"
#include <WiFiUdp.h>

#define DHTPIN 23     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
// set the LCD number of columns and rows
int lcdColumns = 20;
int lcdRows = 4;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
DHT dht(DHTPIN, DHTTYPE);
//const char* ssid     = "Quinn and Cole";
const char* ssid     = "Quinn and Cole 3";
const char* password = "ClevelandRocks";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -28800;
int   daylightOffset_sec = 3600;//3600;
const char* streamId   = "tempdata.txt";
char  replyPacket[] = "success!";

WiFiUDP Udp;
unsigned int localPort = 1026;
unsigned int localUDPPort = 1026;
int counter = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Thermostat"));
  dht.begin();
  // initialize LCD
  lcd.init();
  // turn on LCD backlight
  lcd.backlight();
  Serial.print("Connecting to ");
  lcd.setCursor(0, 0);
  lcd.print(ssid);
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  Serial.println(WiFi.status());

 while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(WiFi.status());
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  lcd.print(" Connected");
  delay(5000);
  lcd.setCursor(0,0);
  lcd.print("                          ");

  Udp.begin(localPort);
  IPAddress ip(10, 0, 0, 20); //hardcoded to Raspi
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // printLocalTime();
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }

}

void loop() {
  counter++;
  //if (counter % 600 == 0)
  Serial.println(WiFi.status());
  if (WiFi.status() != 3 || counter % 100 == 0 )
  { //reset the wifi just in case
    Serial.println("Wifi reconnecting.");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Udp.begin(localPort);
    IPAddress ip(10, 0, 0, 20); //hardcoded to Raspi
  }
  delay(10000); //10 sec delay
  // set cursor to first column, first row
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  int h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }
  Serial.println(&timeinfo, "%l:%M %p");
  char time_disp[9];
  strftime(time_disp, 9, "%l:%M %p", &timeinfo);
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.println(f);
    // print message
  lcd.setCursor(0, 0);
  lcd.print(time_disp);
  //  lcd.setCursor(0,1);
  //  lcd.print("                 ");
  lcd.setCursor(0, 1);
  lcd.print(f);
  lcd.print(" F");
  //  lcd.setCursor(0,2);
  //  lcd.print("               ");
  lcd.setCursor(0, 2);
  lcd.print("RH:");
  lcd.print(h);
  lcd.print("%");
  //send data to UDP
  char f_char[7] = "999.99";
  char rh_char[] = "99";
  dtostrf(f, 6, 2, f_char);
  dtostrf(h, 2, 1, rh_char);
  //  replyPacket = f_char + "," + rh_char
  IPAddress ip(10, 0, 0, 20); //hardcoded to Raspi
  Udp.beginPacket(ip, localUDPPort);
  Udp.print(f);
  Udp.print(",");
  Udp.print(rh_char);
  Udp.endPacket();
}
