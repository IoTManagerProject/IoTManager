#include "Objects/AnalogSensorItem.h"

int AnalogSensorItem::onRead() {
#ifdef ESP32
    int analog_in = analogRead(34);
#endif
    return analogRead(getPin());
};
