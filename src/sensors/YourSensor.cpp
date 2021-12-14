#include "sensors/YourSensor.h"

float yourSensorReading(String type) {
    float value;
    if (type == "name1") {
        value++;
    } else if (type == "name2") {
        value--;
    } else if (type == "name3") {
        value = value + 10;
    }
    return value;
}
