#define JSON

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <xCore.h>
#include <xSW01.h>
#include <xOD01.h>

// Define http and mqtt endpoints
#define http "api.allthingstalk.io"  // API endpoint
#define mqtt "api.allthingstalk.io"  // broker
#include <ATT_IOT.h>
#include <SPI.h>  // required to have support for signed/unsigned long type.

String TOKEN="Your DEVICE_TOKEN";
String DEVICE_ID="Your DEVICE_ID";

byte mac[6];
long rssi;

WiFiClient espClient;

void callback(char* topic, byte* payload, unsigned int length);
PubSubClient pubSub(mqtt, 1883, callback, espClient);
ATTDevice device(DEVICE_ID,TOKEN);

xSW01 SW01;
xOD01 OD01;


void setup()
{
  Serial.begin(115200);  // Init serial link for debugging
    // Set the I2C Pins for CW01
  #ifdef ESP8266
    Wire.pins(2, 14);
    Wire.setClockStretchLimit(15000);
  #endif
  
  // Start the I2C Comunication
  Wire.begin();
  
  // Start the  SW01 Sensor
  Serial.print("SW01: ");
  Serial.println(SW01.begin());
  OD01.begin();

  OD01.println("Weather Station");
  OD01.println("Loading please wait...");
  
  // Enter your WiFi credentials here!
  setupWiFi("WiFi_SSID", "WiFi_PSK");

  rssi = WiFi.RSSI();
  WiFi.macAddress(mac);

  Serial.println();
  
  while(!device.connect(&espClient, http))  // Connect to AllThingsTalk
    Serial.println("retrying");
  // Create device assets
  device.addAsset("2", "2", "", "sensor", "{\"type\": \"number\"}");
  device.addAsset("3", "3", "", "sensor", "{\"type\": \"number\"}");
  device.addAsset("4", "4", "", "sensor", "{\"type\": \"number\"}");
  device.addAsset("5", "5", "", "sensor", "{\"type\": \"number\"}");
  device.addAsset("6", "6", "", "sensor", "{\"type\": \"string\"}");
  device.addAsset("7", "7", "", "actuator", "{\"type\": \"string\"}");
  device.addAsset("8", "8", "", "sensor", "{\"type\": \"number\"}");
  device.addAsset("9", "9", "", "sensor", "{\"type\": \"number\"}");
  device.addAsset("10", "10", "", "sensor", "{\"type\": \"number\"}");

   while(!device.subscribe(pubSub))  // Subscribe to mqtt
    Serial.println("retrying"); 
}
void setupWiFi(const char* ssid, const char* password)
{
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
}

unsigned long prevTime;
unsigned int prevVal = 0;

void loop()
{
   float tempC,pres,hum;
   float alt,dew,cloudBase;
   
   SW01.poll();

   rssi = WiFi.RSSI();
   
   tempC = SW01.getTempC();
   pres = SW01.getPressure();
   hum = SW01.getHumidity();
   alt = SW01.getQNE();
   dew = SW01.getDewPoint();
   cloudBase = ((tempC - dew)/4.4)*1000 + alt;
   
  unsigned long curTime = millis();
  if (curTime > (prevTime + 5000))  // Update and send counter value every 5 seconds
  {
    device.send(String(alt), "9");
    device.send(String(tempC), "2");
    device.send(String(pres), "3");
    device.send(String(hum), "4");
    device.send(String(rssi), "5");
    device.send(String((char*)mac), "6");
    device.send(String(dew), "8");
    device.send(String(cloudBase), "10");

    OD01.home();
    OD01.print("Temp.: ");
    OD01.print(tempC);
    OD01.println(" C");
    OD01.print("Press.: ");
    OD01.print(pres);
    OD01.println(" Pa");
    OD01.print("Hum.: ");
    OD01.print(hum);
    OD01.println(" %");
    OD01.print("Dew point: ");
    OD01.print(dew);
    OD01.println(" C");
    OD01.println("Cloud base: ");
    OD01.print(cloudBase);
    OD01.println(" m");
    prevTime = curTime;
  }
  device.process();
}

void callback(char* topic, byte* payload, unsigned int length) 
{ 
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // Convert payload to json
  StaticJsonBuffer<500> jsonBuffer;
  char json[500];
  for (int i = 0; i < length; i++) {
    json[i] = (char)payload[i];
  }
  json[length] = '\0';
  
  JsonObject& root = jsonBuffer.parseObject(json);

  // Do something
  if(root.success())
  {
    const char* value = root["value"];
    OD01.clear();
    OD01.println(value);
    device.send(value, "toggle");  // Send command back as ACK using JSON
    delay(2000);
  }
  else
    Serial.println("Parsing JSON failed");
}
