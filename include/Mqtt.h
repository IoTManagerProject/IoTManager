#pragma once

#include <Arduino.h>

extern boolean MQTT_Connecting();
extern boolean sendMQTT(String end_of_topik, String data);
extern boolean sendCHART(String topik, String data);
extern void sendSTATUS(String topik, String state);
extern void sendCONTROL(String id, String topik, String state);

extern void do_mqtt_connection();