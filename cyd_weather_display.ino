#include <TFT_eSPI.h>  // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>

int DST = 1;  //1 for Daylight Savings Time, 0 for no Daylight Savings Time

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

unsigned long targetTime = 0;
byte red = 31;
byte green = 0;
byte blue = 0;
byte state = 0;
unsigned int colour = red << 11;

const char* ssid = "Quinn and Col3";
const char* password = "CleavelandLulu";

double inTemp = -99.9;
double outTemp = -99.9;
double lastinTemp = 0;
double lastoutTemp = 0;
long epoch = 0;
long lastepoch = 0;
String forecastName = "";
String forecastDetail = "";
String lastDetail = "foo";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "10.0.0.11";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
//char msg[50];
int value = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setup(void) {

  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  timeClient.begin();
  timeClient.setTimeOffset(-(8 - DST) * 3600);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  targetTime = millis() + 100;
}
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* message, unsigned int length) {

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  StaticJsonDocument<1000> doc;
  DeserializationError error = deserializeJson(doc, message);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  inTemp = doc["extraTemp2"];
  outTemp = doc["outTemp"];
  //  for (int i = 0; i < length; i++) {
  //    Serial.print((char)message[i]);
  //    messageTemp += (char)message[i];
  //  }
  //Serial.println(inTemp);
  //Serial.println(outTemp);
  //get forecast
  HTTPClient http;
  http.begin("https://api.weather.gov/gridpoints/MTR/99,108/forecast");
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      //Serial.println(payload);
      DeserializationError error = deserializeJson(doc, payload);

      // Test if parsing succeeds.
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }
      const char* F1 = doc["properties"]["periods"][0]["name"];
      const char* F2 = doc["properties"]["periods"][0]["detailedForecast"];
      forecastName = F1;
      forecastDetail = F2;
      //Serial.print(forecastName);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("cyd")) {
      //Serial.println("connected");
      // Subscribe
      client.subscribe("weather/data");
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  //tft.fillScreen(TFT_GREEN);
  //delay(1000);
  char blanks[40] = "                                       ";
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  timeClient.update();
  int h = timeClient.getHours();
  int m = timeClient.getMinutes();
  epoch = timeClient.getEpochTime();

  char timedisp[10] = "         ";
  char ampm[] = "AM";
  if (h > 11) {
    ampm[0] = 'P';
    if (h > 12) {
      h = h - 12;
    }
  }
  sprintf_P(timedisp, PSTR("%d:%02d %s"), h, m, ampm);

  Serial.println(timedisp);
  if (epoch - lastepoch > 300) {
    tft.fillScreen(TFT_BLACK);  //overkill to do this every loop, and it's distracting to flicker. Only doing it once every 5 minutes now.
    lastDetail = "nonsense";
  }
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(12, 5);
  char nchar[30];
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  sprintf(nchar, "%3.0f F   ", outTemp);
  tft.drawString("Outside:      ", 10, 30, 2);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  if (outTemp > 100) {
    tft.setTextColor(TFT_MAROON, TFT_BLACK);
  } else if (outTemp > 90) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
  } else if (outTemp > 80) {
    tft.setTextColor(TFT_PINK, TFT_BLACK);
  } else if (outTemp < 50) {
    tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
  } else if (outTemp < 40) {
    tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
  } else if (outTemp < 30) {
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
  }
  tft.drawString(nchar, 80, 25, 4);
  //erase previous arrows. Looks like this is really needed if the direction flips from going up to going down during one period.
  tft.drawWideLine(160, 68, 160, 25, 20, TFT_BLACK, TFT_BLACK);

  if (outTemp > lastoutTemp) {
    tft.drawWideLine(160, 42, 160, 25, 3, TFT_RED, TFT_BLACK);
    tft.drawWideLine(160, 25, 155, 30, 3, TFT_RED, TFT_BLACK);
    tft.drawWideLine(160, 25, 165, 30, 3, TFT_RED, TFT_BLACK);
  } else if (outTemp < lastoutTemp) {
    tft.drawWideLine(160, 42, 160, 25, 3, TFT_BLUE, TFT_BLACK);
    tft.drawWideLine(160, 42, 155, 38, 3, TFT_BLUE, TFT_BLACK);
    tft.drawWideLine(160, 42, 165, 38, 3, TFT_BLUE, TFT_BLACK);
  }

  sprintf(nchar, "%3.0f F    ", inTemp);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Inside:       ", 10, 55, 2);
  if (inTemp > 90) {
    tft.setTextColor(TFT_MAROON, TFT_BLACK);
  } else if (inTemp > 80) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
  } else if (inTemp > 75) {
    tft.setTextColor(TFT_PINK, TFT_BLACK);
  } else if (inTemp < 70) {
    tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
  } else if (inTemp < 68) {
    tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
  } else if (inTemp < 65) {
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
  }
  tft.drawString(nchar, 80, 50, 4);

  if (inTemp > lastinTemp) {
    tft.drawWideLine(160, 68, 160, 50, 3, TFT_RED, TFT_BLACK);
    tft.drawWideLine(160, 50, 155, 54, 3, TFT_RED, TFT_BLACK);
    tft.drawWideLine(160, 50, 165, 54, 3, TFT_RED, TFT_BLACK);
  } else if (inTemp < lastinTemp) {
    tft.drawWideLine(160, 68, 160, 50, 3, TFT_BLUE, TFT_BLACK);
    tft.drawWideLine(160, 68, 155, 62, 3, TFT_BLUE, TFT_BLACK);
    tft.drawWideLine(160, 68, 165, 62, 3, TFT_BLUE, TFT_BLACK);
  }
  if (epoch - lastepoch > 300) {
    lastoutTemp = outTemp;
    lastinTemp = inTemp;
    lastepoch = epoch;
  }
  //display time
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawRightString(timedisp, 260, 10, 2);

  //display forecast
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(&FreeSans9pt7b);  //values from https://github.com/Bodmer/TFT_eSPI/blob/master/examples/320%20x%20240/Free_Font_Demo/Free_Fonts.h
  tft.drawString(blanks, 10, 98);
  tft.drawString(forecastName, 10, 98);
  //Serial.println(forecastName);
  if (forecastDetail != lastDetail) {
    int f_len = forecastDetail.length();
    if (f_len > 0) {      //forecast not set yet
      int lineleng = 40;  //but we're going to look for a space up to 6 characters early
      int f_lines = f_len / lineleng;
      int last_end = -1;
      // how many lines of n characters
      for (int i = 0; i < f_lines + 1; i++) {
        int start = last_end + 1;
        int end = start + lineleng;  //first guess
        if (end > f_len) {
          end = f_len;
        }
        while (forecastDetail.substring(end, end + 1) != " ") {  //look for a space to break the line on
          end--;
        }
        last_end = end;
        tft.drawString(blanks, 10, 120 + (i * 20), 2);
        tft.drawString(forecastDetail.substring(start, end), 10, 120 + (i * 20), 2);
      }
    }
    lastDetail = forecastDetail;
  }
  delay(10000);
}
/*
    // The new larger fonts do not use the .setCursor call, coords are embedded
    tft.setTextColor(TFT_BLACK, TFT_BLACK); // Do not plot the background colour

    // Overlay the black text on top of the rainbow plot (the advantage of not drawing the backgorund colour!)
    tft.drawCentreString("Font size 2", 80, 14, 2); // Draw text centre at position 80, 12 using font 2

    //tft.drawCentreString("Font size 2",81,12,2); // Draw text centre at position 80, 12 using font 2

    tft.drawCentreString("Font size 4", 80, 30, 4); // Draw text centre at position 80, 24 using font 4

    tft.drawCentreString("12.34", 80, 54, 6); // Draw text centre at position 80, 24 using font 6

    tft.drawCentreString("12.34 is in font size 6", 80, 92, 2); // Draw text centre at position 80, 90 using font 2

    // Note the x position is the top left of the font!

    // draw a floating point number
    float pi = 3.14159; // Value to print
    int precision = 3;  // Number of digits after decimal point
    int xpos = 50;      // x position
    int ypos = 110;     // y position
    int font = 2;       // font number only 2,4,6,7 valid. Font 6 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 0 : a p m
    xpos += tft.drawFloat(pi, precision, xpos, ypos, font); // Draw rounded number and return new xpos delta for next print position
    tft.drawString(" is pi", xpos, ypos, font); // Continue printing from new x position
    delay(6000);
*/
