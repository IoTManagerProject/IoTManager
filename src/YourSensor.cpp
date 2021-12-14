#include "YourSensor.h"

float yourSensorReading(String type) {
    float value;
    if (type == "type1") {
        value++;
    } else if (type == "type2") {
        value--;
    } else if (type == "type3") {
        value = value + 10;
    }
    return value;
}
