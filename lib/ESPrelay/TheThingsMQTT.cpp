/*
Copyright (c) 2016 TheThings.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Original Maker: Mateo Velez - Metavix for TheThings Inc
Modified and Maintened by: Jose Garcia - TheThings Inc

*/

#include "TheThingsMQTT.h"


TheThings::TheThings(char* token){
    initialize(token, NULL);
}


TheThings::TheThings(char* token, char* clientName) {
    initialize(token, clientName);
}


void TheThings::begin(void (*callback)(char*,uint8_t*,unsigned int)) {
    this->callback = callback;
    _client.setServer(_server, MQTT_PORT);
    _client.setCallback(callback);
}


bool TheThings::add(char* variableLabel, float value) {
    return add(variableLabel, value, "NULL", "NULL");
}


bool TheThings::add(char* variableLabel, float value, char *context) {
    return add(variableLabel, value, context, "NULL");
}


bool TheThings::add(char* variableLabel, float value, char *context, char *timestamp) {
    (val+currentValue)->_variableLabel = variableLabel;
    (val+currentValue)->_value = value;
    (val+currentValue)->_context = context;
    (val+currentValue)->_timestamp = timestamp;
    currentValue++;
    if (currentValue > MAX_VALUES) {
        Serial.println(F("You are sending more than the maximum of consecutive variables"));
        currentValue = MAX_VALUES;
    }
    return true;
}


bool TheThings::connected(){
    return _client.connected();
}


char* TheThings::getMac(){
    // Obtains the MAC of the device
    Serial.println("entra");
    byte mac[6];
    WiFi.macAddress(mac);
    char macAddr[18];
    sprintf(macAddr, "%2X%2X%2X%2X%2X%2X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return macAddr;
}


void TheThings::initialize(char* token, char* clientName){
    _server = SERVER;
    _token = token;
    currentValue = 0;
    val = (Value *)malloc(MAX_VALUES*sizeof(Value));
    if(clientName!=NULL){
        _clientName = clientName;
    }
}


bool TheThings::loop() {
    if (!_client.connected()) {
        reconnect();
    }
    return _client.loop();
}


void TheThings::reconnect() {
    while (!_client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (_client.connect(_clientName, _token, NULL)) {
            Serial.println("connected");
            break;
    } else {
      Serial.print("failed, rc=");
      Serial.print(_client.state());
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
  }
}


void TheThings::setDebug(bool debug){
    _debug = debug;
}


void TheThings::  setBroker(char* broker){
    if (_debug){
        Serial.println("Broker set for Business Account");
    }
    _server = broker;
}


bool TheThings::subscribe() {
    char topic[150];
    sprintf(topic, "%s%s", FIRST_PART_TOPIC, _token);
    if (!_client.connected()) {
        reconnect();
    }
    if (_debug){
        Serial.println("Subscribed to: ");
        Serial.println(topic);
    }
    return _client.subscribe(topic);
}


bool TheThings::publish() {
    
    char topic[150];
    char payload[500];
    String str;
    sprintf(topic, "%s%s", FIRST_PART_TOPIC, _token);
    sprintf(payload, "[");
    for (int i = 0; i <= currentValue; ) {
        str = String((val+i)->_value, 1);
        sprintf(payload,"%s{\"key\":\"%s\",\"value\":%s",payload,(val+i)->_variableLabel, str.c_str());
        if ((val+i)->_timestamp != "NULL") {
            sprintf(payload, "%s, \"timestamp\": %s", payload, (val+i)->_timestamp);
        }
        if ((val+i)->_context != "NULL") {
            sprintf(payload, "%s, \"context\": {%s}", payload, (val+i)->_context);
        }
        i++;
        if (i >= currentValue) {
            sprintf(payload, "%s}]", payload);
            break;
        } else {
            sprintf(payload, "%s},", payload);
        }
    }
    if (_debug){
        Serial.println("publishing to TOPIC: ");
        Serial.println(topic);
        Serial.print("JSON dict: ");
        Serial.println(payload);
    }
    currentValue = 0;
    return _client.publish(topic, payload, 512);
}


bool TheThings::wifiConnection(char* ssid, char* pass) {
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());
    if(_clientName==NULL){
        _clientName = getMac();
    }
}

bool TheThings::wifiConnection2() {
    WiFiManager wifiManager;
    wifiManager.setTimeout(180);
  
    //if(!wifiManager.autoConnect("AutoConnectAP")) {
    if(!wifiManager.autoConnect()) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      ESP.reset();
      delay(5000);
    } 
}

int TheThings::state() {
  return _client.state();
}

