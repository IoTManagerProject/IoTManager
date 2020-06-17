/****************************************
 * Include Libraries
 ****************************************/
#include "TheThingsMQTT.h"
#include "TickerScheduler.h"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

/****************************************
 * Define Constants
 ****************************************/
#define TOKEN "" // Your TheThings TOKEN
#define WIFINAME "" //Your SSID
#define WIFIPASS "" // Your Wifi Pass

TheThings client(TOKEN);

#define DHTPIN          14         // Pin which is cosnnected to the DHT sensor.
// Uncomment the type of sensor in use:
//#define DHTTYPE           DHT11     // DHT 11
#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;
#define TRIGGER_PIN 0
#define FRECUENCY 2000
char *AP="TEST_BQ";
String prefix="rel";
const char WiFiAPPSK[] = "prueba";

WiFiManager wifiManager;

#define RELAY1 12
#define RELAY2 13

#define MEASUREMENT_TIME 30000 // Time in milisecons

float Temperature,Humidity;
TickerScheduler ts(1);

/****************************************
 * Auxiliar Functions
 ****************************************/

//void configWiFi(){
//    wifiManager.resetSettings();
//    if(!wifiManager.autoConnect(AP,WiFiAPPSK)) {
//        Serial.println("Reconfiguration called");
//        delay(3000);
//        //reset and try again, or maybe put it to deep sleep
//        ESP.reset();
//        delay(15000);
//    }
//}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived");
    // handle message arrived
    String text = "";
    for (int i = 0; i < length; i++)
        text.concat((char)payload[i]);
    Serial.println(text);
    text.replace(" ", "");
    text.toUpperCase();
    if (text.indexOf("\"KEY\":\"REL1\"") >= 0) {
        if (text.indexOf("\"VALUE\":\"ON\"") >= 0) digitalWrite(RELAY1,HIGH);
        else if (text.indexOf("\"VALUE\":\"OFF\"") >= 0) digitalWrite(RELAY1,LOW);
    }
    else if (text.indexOf("\"KEY\":\"REL2\"") >= 0) {
        if (text.indexOf("\"VALUE\":\"ON\"") >= 0)  digitalWrite(RELAY2,HIGH);
        else if (text.indexOf("\"VALUE\":\"OFF\"") >= 0) digitalWrite(RELAY2,LOW);
    }
}

float temperatureMeasurement(sensors_event_t event, DHT_Unified dht){
  // Get temperature event and print its value.
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
      Serial.println("Error reading temperature!");
  }
  else {
      Serial.print("Temperature: ");
      Serial.print(event.temperature);
      Serial.println(" *C");
      return event.temperature;
  }
}

float humidityMeasurement(sensors_event_t event, DHT_Unified dht){
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
      Serial.println("Error reading humidity!");
  }
  else {
      Serial.print("Humidity: ");
      Serial.print(event.relative_humidity);
      Serial.println("%");
      return event.relative_humidity;
  }
}

void measureAndSend(){
      sensors_event_t event;
      Temperature=temperatureMeasurement(event,dht);
      Humidity=humidityMeasurement(event,dht);
      client.add("Temperature", Temperature);
      client.add("Humidity", Humidity);
      client.publish();
}

/****************************************
 * Main Functions
 ****************************************/


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
  pinMode(RELAY1,OUTPUT);
  pinMode(RELAY2,OUTPUT);

  dht.begin();
  sensor_t sensor;

  

//  pinMode(TRIGGER_PIN, INPUT);
//    attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), configWiFi, RISING);
//
//      //sets timeout until configuration portal gets turned off
//    //useful to make it all retry or go to sleep
//    //in seconds
//    wifiManager.setTimeout(280);
//
//    //fetches ssid and pass and tries to connect
//    //if it does not connect it starts an access point with the specified name
//    //here  "AutoConnectAP"
//    //and goes into a blocking loop awaiting configuration
//    if(!wifiManager.autoConnect(AP,WiFiAPPSK)) {
//        Serial.println("failed to connect and hit timeout");
//        delay(3000);
//        //reset and try again, or maybe put it to deep sleep
//        //ESP.reset();
//        delay(FRECUENCY);
//    }
//    //if you get here you have connected to the WiFi
//    Serial.println("connected...yeey :)");

  client.wifiConnection(WIFINAME, WIFIPASS);

  //client.wifiConnection2();
  client.begin(callback);
  client.subscribe(); //Insert the dataSource and Variable's Labels

  ts.add(0, MEASUREMENT_TIME, [&](void*) { measureAndSend(); }, nullptr, false);
  ts.enableAll();

  }
  
void loop() {
  // put your main code here, to run repeatedly:
  ts.update();
  
  if(!client.connected()){
      client.reconnect();
      client.subscribe(); //Insert the dataSource and Variable's Labels
  }
  client.loop();
  
}
