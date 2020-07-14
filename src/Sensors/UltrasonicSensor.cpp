#include "Sensors/UltrasonicSensor.h"

#include "Global.h"
#include "MqttClient.h"
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

        liveData.write(levelPr_value_name, String(level, DEC), VT_INT);
        Scenario::fire(levelPr_value_name);

        MqttClient::publishStatus(levelPr_value_name, String(level, 2), VT_FLOAT);

        Serial.println("[I] sensor '" + levelPr_value_name + "' data: " + String(level));

        liveData.write(ultrasonicCm_value_name, String(distance_cm, DEC), VT_INT);

        Scenario::fire(ultrasonicCm_value_name);

        MqttClient::publishStatus(ultrasonicCm_value_name, String(distance_cm), VT_FLOAT);
    }
}
}  // namespace Ultrasonic