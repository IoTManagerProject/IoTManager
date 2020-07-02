#include "Sensors.h"

#include "Options.h"
#include "Events.h"
#include "LiveData.h"
#include "MqttClient.h"
#include "Utils/PrintMessage.h"

const static char *MODULE = "Sensor";

namespace Sensors {

boolean sensor_enabled[NUM_SENSORS] = {false};

void enable(size_t num, boolean value) {
    sensor_enabled[num] = value;
}

void init() {
    for (size_t i = 0; i < NUM_SENSORS; i++) {
        enable(i, false);
    }
    ts.add(
        SENSORS, 1000, [&](void *) {
            if (sensor_enabled[0])
                ultrasonic_reading();
            if (sensor_enabled[1])
                analog_reading1();
            if (sensor_enabled[2])
                analog_reading2();
            if (sensor_enabled[3])
                dallas_reading();
            if (sensor_enabled[4])
                dhtT_reading();
            if (sensor_enabled[5])
                dhtH_reading();
            if (sensor_enabled[6])
                dhtP_reading();
            if (sensor_enabled[7])
                dhtC_reading();
            if (sensor_enabled[8])
                dhtD_reading();
            if (sensor_enabled[9])
                bmp280T_reading();
            if (sensor_enabled[10])
                bmp280P_reading();
            if (sensor_enabled[11])
                bme280T_reading();
            if (sensor_enabled[12])
                bme280P_reading();
            if (sensor_enabled[13])
                bme280H_reading();
            if (sensor_enabled[14])
                bme280A_reading();
        },
        nullptr, true);
}  // namespace Sensors

void ultrasonic_reading() {
    long duration_;
    int distance_cm;
    int level;
    static int counter;
    int trig = Options::readInt("trig");
    int echo = Options::readInt("echo");
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
                    Options::readInt("e_lev"),
                    Options::readInt("f_lev"), 0, 100);

        LiveData::writeInt(levelPr_value_name, level);
        Events::fire(levelPr_value_name);

        MqttClient::publishStatus(levelPr_value_name, String(level));

        Serial.println("[I] sensor '" + levelPr_value_name + "' data: " + String(level));

        LiveData::writeInt(ultrasonicCm_value_name, distance_cm);
        Events::fire(ultrasonicCm_value_name);

        MqttClient::publishStatus(ultrasonicCm_value_name, String(distance_cm));

        Serial.println("[I] sensor '" + ultrasonicCm_value_name + "' data: " + String(distance_cm));
    }
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
        LiveData::write(buf, String(temp));
        Events::fire(buf);
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
            Events::fire(dhtT_value_name);
            LiveData::write(dhtT_value_name, String(value));
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
            Events::fire(dhtH_value_name);
            LiveData::write(dhtH_value_name, String(value));
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
        value = dht.computePerception(LiveData::read(dhtT_value_name).toFloat(), LiveData::read(dhtH_value_name).toFloat(), false);
        String final_line = perceptionStr(value);
        LiveData::write("dhtPerception", final_line);
        Events::fire("dhtPerception");

        if (MqttClient::isConnected()) {
            MqttClient::publishStatus("dhtPerception", final_line);
            Serial.println("[I] sensor 'dhtPerception' data: " + final_line);
        }
    }
}

void dhtC_reading() {
    ComfortState cf;
    if (dht.getStatus() != 0) {
        MqttClient::publishStatus("dhtComfort", String(dht.getStatusString()));
    } else {
        dht.getComfortRatio(cf,
                            LiveData::read(dhtT_value_name).toFloat(),
                            LiveData::read(dhtH_value_name).toFloat(), false);
        String final_line = comfortStr(cf);
        LiveData::write("dhtComfort", final_line);
        Events::fire("dhtComfort");
        MqttClient::publishStatus("dhtComfort", final_line);
        Serial.println("[I] sensor 'dhtComfort' send date " + final_line);
    }
}

void dhtD_reading() {
    float value;
    if (dht.getStatus() != 0) {
        MqttClient::publishStatus("dhtDewpoint", String(dht.getStatusString()));
    } else {
        value = dht.computeDewPoint(
            LiveData::read(dhtT_value_name).toFloat(),
            LiveData::read(dhtH_value_name).toFloat(), false);
        LiveData::writeFloat("dhtDewpoint", value);
        Events::fire("dhtDewpoint");
        MqttClient::publishStatus("dhtDewpoint", String(value));
        Serial.println("[I] sensor 'dhtDewpoint' data: " + String(value));
    }
}

void bmp280T_reading() {
    float value = 0;
    sensors_event_t temp_event;
    bmp_temp->getEvent(&temp_event);
    value = temp_event.temperature;
    LiveData::write(bmp280T_value_name, String(value));
    Events::fire(bmp280T_value_name);
    MqttClient::publishStatus(bmp280T_value_name, String(value));
    Serial.println("[I] sensor '" + bmp280T_value_name + "' data: " + String(value));
}

void bmp280P_reading() {
    float value = 0;
    sensors_event_t pressure_event;
    bmp_pressure->getEvent(&pressure_event);
    value = pressure_event.pressure;
    value = value / 1.333224;
    LiveData::write(bmp280P_value_name, String(value));
    Events::fire(bmp280P_value_name);
    MqttClient::publishStatus(bmp280P_value_name, String(value));
    Serial.println("[I] sensor '" + bmp280P_value_name + "' data: " + String(value));
}

void bme280T_reading() {
    float value = 0;
    value = bme.readTemperature();
    LiveData::write(bme280T_value_name, String(value));
    Events::fire(bme280T_value_name);
    MqttClient::publishStatus(bme280T_value_name, String(value));
    Serial.println("[I] sensor '" + bme280T_value_name + "' data: " + String(value));
}

void bme280P_reading() {
    float value = 0;
    value = bme.readPressure();
    value = value / 1.333224;
    LiveData::write(bme280P_value_name, String(value));
    Events::fire(bme280P_value_name);
    MqttClient::publishStatus(bme280P_value_name, String(value));
    Serial.println("[I] sensor '" + bme280P_value_name + "' data: " + String(value));
}

void bme280H_reading() {
    float value = 0;
    value = bme.readHumidity();
    LiveData::write(bme280H_value_name, String(value));
    Events::fire(bme280H_value_name);
    MqttClient::publishStatus(bme280H_value_name, String(value));
    Serial.println("[I] sensor '" + bme280H_value_name + "' data: " + String(value));
}

void bme280A_reading() {
    float value = bme.readAltitude(1013.25);
    LiveData::write(bme280A_value_name, String(value, 2));

    Events::fire(bme280A_value_name);

    MqttClient::publishStatus(bme280A_value_name, String(value));

    Serial.println("[I] sensor '" + bme280A_value_name + "' data: " + String(value));
}

}  // namespace Sensors