#include "Sensors.h"

#include "Utils/PrintMessage.h"

const static char *MODULE = "Sensor";

namespace Sensors {

boolean sensors_reading_map[NUM_SENSORS] = {false};

void enable(size_t num, boolean enable) {
    sensors_reading_map[num] = enable;
}

void init() {
    for (size_t i = 0; i < NUM_SENSORS; i++) {
        Sensors::enable(i, false);
    }

    ts.add(
        SENSORS, 1000, [&](void *) {
            static int counter;
            counter++;

            if (sensors_reading_map[0] == 1)
                ultrasonic_reading();

            if (counter > 10) {
                counter = 0;

                if (sensors_reading_map[1] == 1)
                    analog_reading1();
                if (sensors_reading_map[2] == 1)
                    analog_reading2();
                if (sensors_reading_map[3] == 1)
                    dallas_reading();
                if (sensors_reading_map[4] == 1)
                    dhtT_reading();
                if (sensors_reading_map[5] == 1)
                    dhtH_reading();
                if (sensors_reading_map[6] == 1)
                    dhtP_reading();
                if (sensors_reading_map[7] == 1)
                    dhtC_reading();
                if (sensors_reading_map[8] == 1)
                    dhtD_reading();
                if (sensors_reading_map[9] == 1)
                    bmp280T_reading();
                if (sensors_reading_map[10] == 1)
                    bmp280P_reading();
                if (sensors_reading_map[11] == 1)
                    bme280T_reading();
                if (sensors_reading_map[12] == 1)
                    bme280P_reading();
                if (sensors_reading_map[13] == 1)
                    bme280H_reading();
                if (sensors_reading_map[14] == 1)
                    bme280A_reading();
            }
        },
        nullptr, true);
}

void ultrasonic_reading() {
    long duration_;
    int distance_cm;
    int level;
    static int counter;
    int trig = jsonReadInt(configOptionJson, "trig");
    int echo = jsonReadInt(configOptionJson, "echo");
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    duration_ = pulseIn(echo, HIGH, 30000);  // 3000 µs = 50cm // 30000 µs = 5 m
    distance_cm = duration_ / 29 / 2;
    distance_cm = medianFilter.filtered(distance_cm);  //отсечение промахов медианным фильтром
    counter++;
    if (counter > TANK_LEVEL_SAMPLES) {
        counter = 0;
        level = map(distance_cm,
                    jsonReadInt(configOptionJson, "e_lev"),
                    jsonReadInt(configOptionJson, "f_lev"), 0, 100);

        jsonWriteInt(configLiveJson, levelPr_value_name, level);
        fireEvent(levelPr_value_name, "");

        MqttClient::publishStatus(levelPr_value_name, String(level));

        Serial.println("[I] sensor '" + levelPr_value_name + "' data: " + String(level));

        jsonWriteInt(configLiveJson, ultrasonicCm_value_name, distance_cm);
        fireEvent(ultrasonicCm_value_name, "");

        MqttClient::publishStatus(ultrasonicCm_value_name, String(distance_cm));

        Serial.println("[I] sensor '" + ultrasonicCm_value_name + "' data: " + String(distance_cm));
    }
}

void analog_reading1() {
    String name = selectFromMarkerToMarker(analog_value_names_list, ",", 0);
#ifdef ESP32
    int analog_in = analogRead(34);
#endif
#ifdef ESP8266
    int raw = analogRead(A0);
#endif

    int mapped = map(raw,
                     jsonReadInt(configOptionJson, name + "_st"),
                     jsonReadInt(configOptionJson, name + "_end"),
                     jsonReadInt(configOptionJson, name + "_st_out"),
                     jsonReadInt(configOptionJson, name + "_end_out"));

    pm.info("sensor: '" + name + "' value: " + String(mapped, DEC));

    jsonWriteInt(configLiveJson, name, mapped);

    fireEvent(name);

    MqttClient::publishStatus(name, String(mapped));
}

void analog_reading2() {
    String value_name = selectFromMarkerToMarker(analog_value_names_list, ",", 1);
#ifdef ESP32
    int analog_in = analogRead(35);
#endif
#ifdef ESP8266
    int analog_in = analogRead(A0);
#endif
    int analog = map(analog_in,
                     jsonReadInt(configOptionJson, value_name + "_st"),
                     jsonReadInt(configOptionJson, value_name + "_end"),
                     jsonReadInt(configOptionJson, value_name + "_st_out"),
                     jsonReadInt(configOptionJson, value_name + "_end_out"));
    jsonWriteInt(configLiveJson, value_name, analog);
    fireEvent(value_name, "");
    MqttClient::publishStatus(value_name, String(analog));
    Serial.println("[I] sensor '" + value_name + "' data: " + String(analog));
}

void dallas_reading() {
    if (!dallasTemperature) {
        return;
    }

    float temp = 0;
    byte num = dallasTemperature->getDS18Count();
    String dallas_value_name_tmp_buf = dallas_value_name;
    dallasTemperature->requestTemperatures();
    for (byte i = 0; i < num; i++) {
        temp = dallasTemperature->getTempCByIndex(i);
        String buf = selectToMarker(dallas_value_name_tmp_buf, ";");
        dallas_value_name_tmp_buf = deleteBeforeDelimiter(dallas_value_name_tmp_buf, ";");
        jsonWriteStr(configLiveJson, buf, String(temp));
        fireEvent(buf, "");
        MqttClient::publishStatus(buf, String(temp));
        Serial.println("[I] sensor '" + buf + "' send date " + String(temp));
    }
}

void dhtT_reading() {
    float value = 0;
    static int counter;
    if (dht.getStatus() != 0 && counter < 5) {
        MqttClient::publishStatus(dhtT_value_name, String(dht.getStatusString()));
        counter++;
    } else {
        counter = 0;
        value = dht.getTemperature();
        if (String(value) != "nan") {
            fireEvent(dhtT_value_name, "");
            jsonWriteStr(configLiveJson, dhtT_value_name, String(value));
            MqttClient::publishStatus(dhtT_value_name, String(value));
            Serial.println("[I] sensor '" + dhtT_value_name + "' data: " + String(value));
        }
    }
}

void dhtH_reading() {
    float value = 0;
    static int counter;
    if (dht.getStatus() != 0 && counter < 5) {
        MqttClient::publishStatus(dhtH_value_name, String(dht.getStatusString()));
        counter++;
    } else {
        counter = 0;
        value = dht.getHumidity();
        if (String(value) != "nan") {
            fireEvent(dhtH_value_name, "");
            jsonWriteStr(configLiveJson, dhtH_value_name, String(value));
            MqttClient::publishStatus(dhtH_value_name, String(value));
            Serial.println("[I] sensor '" + dhtH_value_name + "' data: " + String(value));
        }
    }
}

void dhtP_reading() {
    byte value;
    if (dht.getStatus() != 0) {
        MqttClient::publishStatus("dhtPerception", String(dht.getStatusString()));
    } else {
        value = dht.computePerception(jsonReadStr(configLiveJson, dhtT_value_name).toFloat(), jsonReadStr(configLiveJson, dhtH_value_name).toFloat(), false);
        String final_line = perceptionStr(value);
        jsonWriteStr(configLiveJson, "dhtPerception", final_line);
        fireEvent("dhtPerception", "");
        MqttClient::publishStatus("dhtPerception", final_line);
        if (mqtt.connected()) {
            Serial.println("[I] sensor 'dhtPerception' data: " + final_line);
        }
    }
}

void dhtC_reading() {
    ComfortState cf;
    if (dht.getStatus() != 0) {
        MqttClient::publishStatus("dhtComfort", String(dht.getStatusString()));
    } else {
        dht.getComfortRatio(cf, jsonReadStr(configLiveJson, dhtT_value_name).toFloat(), jsonReadStr(configLiveJson, dhtH_value_name).toFloat(), false);
        String final_line = comfortStr(cf);
        jsonWriteStr(configLiveJson, "dhtComfort", final_line);
        fireEvent("dhtComfort", "");
        MqttClient::publishStatus("dhtComfort", final_line);
        Serial.println("[I] sensor 'dhtComfort' send date " + final_line);
    }
}

void dhtD_reading() {
    float value;
    if (dht.getStatus() != 0) {
        MqttClient::publishStatus("dhtDewpoint", String(dht.getStatusString()));
    } else {
        value = dht.computeDewPoint(jsonReadStr(configLiveJson, dhtT_value_name).toFloat(), jsonReadStr(configLiveJson, dhtH_value_name).toFloat(), false);
        jsonWriteInt(configLiveJson, "dhtDewpoint", value);
        fireEvent("dhtDewpoint", "");
        MqttClient::publishStatus("dhtDewpoint", String(value));
        Serial.println("[I] sensor 'dhtDewpoint' data: " + String(value));
    }
}

void bmp280T_reading() {
    float value = 0;
    sensors_event_t temp_event;
    bmp_temp->getEvent(&temp_event);
    value = temp_event.temperature;
    jsonWriteStr(configLiveJson, bmp280T_value_name, String(value));
    fireEvent(bmp280T_value_name, "");
    MqttClient::publishStatus(bmp280T_value_name, String(value));
    Serial.println("[I] sensor '" + bmp280T_value_name + "' data: " + String(value));
}

void bmp280P_reading() {
    float value = 0;
    sensors_event_t pressure_event;
    bmp_pressure->getEvent(&pressure_event);
    value = pressure_event.pressure;
    value = value / 1.333224;
    jsonWriteStr(configLiveJson, bmp280P_value_name, String(value));
    fireEvent(bmp280P_value_name, "");
    MqttClient::publishStatus(bmp280P_value_name, String(value));
    Serial.println("[I] sensor '" + bmp280P_value_name + "' data: " + String(value));
}

void bme280T_reading() {
    float value = 0;
    value = bme.readTemperature();
    jsonWriteStr(configLiveJson, bme280T_value_name, String(value));
    fireEvent(bme280T_value_name, "");
    MqttClient::publishStatus(bme280T_value_name, String(value));
    Serial.println("[I] sensor '" + bme280T_value_name + "' data: " + String(value));
}

void bme280P_reading() {
    float value = 0;
    value = bme.readPressure();
    value = value / 1.333224;
    jsonWriteStr(configLiveJson, bme280P_value_name, String(value));
    fireEvent(bme280P_value_name, "");
    MqttClient::publishStatus(bme280P_value_name, String(value));
    Serial.println("[I] sensor '" + bme280P_value_name + "' data: " + String(value));
}

void bme280H_reading() {
    float value = 0;
    value = bme.readHumidity();
    jsonWriteStr(configLiveJson, bme280H_value_name, String(value));
    fireEvent(bme280H_value_name, "");
    MqttClient::publishStatus(bme280H_value_name, String(value));
    Serial.println("[I] sensor '" + bme280H_value_name + "' data: " + String(value));
}

void bme280A_reading() {
    float value = bme.readAltitude(1013.25);
    jsonWriteStr(configLiveJson, bme280A_value_name, String(value, 2));

    fireEvent(bme280A_value_name, "");

    MqttClient::publishStatus(bme280A_value_name, String(value));

    Serial.println("[I] sensor '" + bme280A_value_name + "' data: " + String(value));
}

}  // namespace Sensors