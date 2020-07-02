#include "Sensor/AnalogSensor.h"

#include "Global.h"
#include "MqttClient.h"
#include "Events.h"

static const char* MODULE = "AnalogSensor";

String analog_value_names_list = "";
int enter_to_analog_counter = 0;

void analog_reading1() {
    String name = selectFromMarkerToMarker(analog_value_names_list, ",", 0);
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
    String name = selectFromMarkerToMarker(analog_value_names_list, ",", 1);
#ifdef ESP32
    int raw = analogRead(35);
#endif
#ifdef ESP8266
    int raw = analogRead(A0);
#endif
    int mapped = map(raw,
                     options.readInt(name + "_st"),
                     options.readInt(name + "_end"),
                     options.readInt(name + "_st_out"),
                     options.readInt(name + "_end_out"));
    liveData.writeInt(name, mapped);
    Events::fire(name);
    MqttClient::publishStatus(name, String(mapped));
}