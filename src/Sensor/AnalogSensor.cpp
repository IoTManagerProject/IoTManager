#include "Sensor/AnalogSensor.h"

#include "Sensors.h"
#include "MqttClient.h"
#include "Events.h"

static const char* MODULE = "AnalogSensor";

namespace AnalogSensor {

String _list = "";
size_t _count = 0;

void add(String name) {
    _list += name + ",";
    _count++;
    Sensors::enable(_count);
}

void analog_reading1() {
    String name = selectFromMarkerToMarker(_list, ",", 0);
#ifdef ESP32
    int analog_in = analogRead(34);
#endif
#ifdef ESP8266
    int raw = analogRead(A0);
#endif
    int mapped = map(raw,
                     options.readInt(name + "_st"),
                     options.readInt(name + "_end"),
                     options.readInt(name + "_st_out"),
                     options.readInt(name + "_end_out"));
    pm.info(name + " raw:" + String(raw, DEC) + "map: " + String(mapped, DEC));
    liveData.writeInt(name, mapped);
    Events::fire(name);
    MqttClient::publishStatus(name, String(mapped));
}

void analog_reading2() {
    String name = selectFromMarkerToMarker(_list, ",", 1);
#ifdef ESP32
    int raw = analogRead(35);
#endif
#ifdef ESP8266
    int raw = analogRead(A0);
#endif
    int value = map(raw,
                    options.readInt(name + "_st"),
                    options.readInt(name + "_end"),
                    options.readInt(name + "_st_out"),
                    options.readInt(name + "_end_out"));
    pm.info("name: " + name + ", raw:" + String(raw, DEC) + ", value: " + String(value, DEC));
    liveData.writeInt(name, value);
    Events::fire(name);
    MqttClient::publishStatus(name, String(value));
}
}  // namespace AnalogSensor