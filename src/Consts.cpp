#include "Consts.h"

#include <Arduino.h>

static const char* message_type_str[NUM_BROADCAST_MESSAGES] = {"iot_device", "mqtt_settings"};
const char* getMessageType(BroadcastMessage_t type) {
    return message_type_str[type];
}

BroadcastMessage_t getMessageType(const char* str) {
    for (size_t i = 0; i < NUM_BROADCAST_MESSAGES; i++) {
        if (strcmp(str, message_type_str[i]) == 0) {
            return BroadcastMessage_t(i);
        }
    }
    return NUM_BROADCAST_MESSAGES;
}