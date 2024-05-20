
#include "DFRobot_RainfallSensor.h"
//#include <WiFiNINA.h>
#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>

DFRobot_RainfallSensor_I2C Sensor(&Wire);

//char* ssid = "QuinnandCole";
char* ssid     = "QuinnandCole3";
char* password = "CleavelandLu1u";
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
//const char broker[] = "10.0.0.11";
IPAddress broker {10,0,0,11};
int port = 1883;
const char topic[] = "weather/rain";
StaticJsonDocument<200> doc;
void setup(void)
{

  Serial.begin(9600);
  String x;
  delay(1000);
  x = Sensor.getFirmwareVersion();
Serial.println(x);

  while(!Sensor.begin()){
    Serial.println("Sensor init err!!!");
    delay(1000);
  }
  Serial.print("vid:\t");
  Serial.println(Sensor.vid,HEX);
  Serial.print("pid:\t");
  Serial.println(Sensor.pid,HEX);
  Serial.print("Version:\t");
  Serial.println(Sensor.getFirmwareVersion());
  //Set the cumulative rainfall value in units of mm.
  //Sensor.setRainAccumulatedValue(0.2794);

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
  }
}

void loop()
{
  //Get the sensor operating time in units of hours.
  Serial.print("Sensor WorkingTime:\t");
  float worktime = Sensor.getSensorWorkingTime();
  doc["worktime"]=worktime;
  Serial.print(worktime);
  Serial.println(" H");
  //Get the cumulative rainfall during the sensor operating time.
  Serial.print("Rainfall:\t");
  float rainfall = Sensor.getRainfall();
  doc["rainfall"]=rainfall;
  Serial.println(rainfall);
  //Here is an example function that calculates the cumulative rainfall in a specified hour of the system. The function takes an optional argument, which can be any value between 1 and 24.
  Serial.print("1 Hour Rainfall:\t");
  float hourrain = Sensor.getRainfall(1);
  doc["hourrain"]=hourrain;
  Serial.print(hourrain);
  Serial.println(" mm");
  //Get the raw data, which is the tipping bucket count of rainfall, in units of counts.
  Serial.print("rainfall raw:\t");
  int raw = Sensor.getRawData();
  doc["bucketcount"]=raw;
  Serial.println(raw);
// json data {"worktime": worktime, "rainfall": rainfall, "hourrain": hourrain, "bucketcount": raw}
  
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
  }
  delay(1000*60*1); //5 minutes
}
