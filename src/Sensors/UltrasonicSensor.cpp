#include "Sensors/UltrasonicSensor.h"

#include "Global.h"
#include "MqttClient.h"
#include "Events.h"

namespace Ultrasonic {
GMedian<10, int> medianFilter;

String levelPr_value_name = "";
String ultrasonicCm_value_name = "";

void ultrasonic_reading() {
    long duration_;
    int distance_cm;
    int level;
    static int counter;
    int trig = options.readInt("trig");
    int echo = options.readInt("echo");
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
                    options.readInt("e_lev"),
                    options.readInt("f_lev"), 0, 100);

        liveData.writeInt(levelPr_value_name, level);
        Scenario::fire(levelPr_value_name);

        MqttClient::publishStatus(VT_FLOAT, levelPr_value_name, String(level));

        Serial.println("[I] sensor '" + levelPr_value_name + "' data: " + String(level));

        liveData.writeInt(ultrasonicCm_value_name, distance_cm);
        Scenario::fire(ultrasonicCm_value_name);

        MqttClient::publishStatus(VT_FLOAT, ultrasonicCm_value_name, String(distance_cm));

        Serial.println("[I] sensor '" + ultrasonicCm_value_name + "' data: " + String(distance_cm));
    }
}
}  // namespace Ultrasonic