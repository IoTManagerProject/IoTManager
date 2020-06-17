
#ifndef TheThingsMQTT_H
#define TheThingsMQTT_H
//#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

#define MQTT_PORT 1883
#define SERVER "mqtt.thethings.io"
#define MAX_VALUES 5
#define FIRST_PART_TOPIC "v2/things/"

#define META_DEBUG Serial

typedef struct Value {
    char* _variableLabel;
    float _value;
    char* _context;
    char* _timestamp;
} Value;

class TheThings {
   private:
    void (*callback)(char*, uint8_t*, unsigned int);
    char* getMac();
    void initialize(char* token, char* clientName);
    WiFiClient espClient;
    PubSubClient _client = PubSubClient(espClient);
    WiFiManager wifiManager;
    char* _clientName = NULL;
    bool _debug = false;
    uint8_t currentValue;
    char* _password;
    char* _server;
    char* _ssid;
    char* _token;
    Value* val;

   public:
    TheThings(char* token);
    TheThings(char* token, char* clientName);
    bool add(char* variableLabel, float value);
    bool add(char* variableLabel, float value, char* context);
    bool add(char* variableLabel, float value, char* context, char* timestamp);
    void begin(void (*callback)(char*, uint8_t*, unsigned int));
    bool connected();
    bool loop();
    bool subscribe();
    bool publish();
    void setBroker(char* broker);
    void reconnect();
    void setDebug(bool debug);
    bool wifiConnection(char* ssid, char* pass);
    bool wifiConnection2();
    int state();
};

#endif
