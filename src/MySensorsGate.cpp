#include "MySensorsGate.h"
#ifdef MYSENSORS

MyMessage msg(CHILD_ID, V_TEXT);

void receive(const MyMessage &message) {
    if ((message.getSensor() == 0) && (message.getSender() == 100)) {
        Serial.println(message.getFloat());
    }
}
#endif