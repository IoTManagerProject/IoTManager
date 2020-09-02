#include "items/itemsGlobal.h"
#include "items/SensorUltrasonicClass.h"
//#ifdef LEVEL_ENABLED
//=========================================Модуль ультрозвукового дальномера==================================================================
//ultrasonic-cm;id;anydata;Сенсоры;Расстояние;order;pin[12,13];map[1,100,1,100];c[1]
//=========================================================================================================================================
SensorUltrasonic* mySensorUltrasonic;
void ultrasonicCm() {
    mySensorUltrasonic = new SensorUltrasonic();
    mySensorUltrasonic->update();
    String key = mySensorUltrasonic->gkey();
    String pin = mySensorUltrasonic->gpin();
    String trig = selectFromMarkerToMarker(pin, ",", 0);
    String echo = selectFromMarkerToMarker(pin, ",", 1);
    sCmd.addCommand(key.c_str(), ultrasonicReading);
    sensorReadingMap += key + ",";
    jsonWriteStr(configOptionJson, key + "_trig", trig);
    jsonWriteStr(configOptionJson, key + "_echo", echo);
    jsonWriteStr(configOptionJson, key + "_map", mySensorUltrasonic->gmap());
    jsonWriteStr(configOptionJson, key + "_с", mySensorUltrasonic->gc());
    mySensorUltrasonic->clear();
}

void ultrasonicReading() {
    String key = sCmd.order();
    String trig = jsonReadStr(configOptionJson, key + "_trig");
    String echo = jsonReadStr(configOptionJson, key + "_echo");
    String pin = trig + "," + echo;
    mySensorUltrasonic->SensorUltrasonicRead(key, pin);
}
//#endif

//void levelPr() {
//    String value_name = sCmd.next();
//    String trig = sCmd.next();
//    String echo = sCmd.next();
//    String widget_name = sCmd.next();
//    String page_name = sCmd.next();
//    String type = sCmd.next();
//    String empty_level = sCmd.next();
//    String full_level = sCmd.next();
//    String page_number = sCmd.next();
//    levelPr_value_name = value_name;
//    jsonWriteStr(configOptionJson, "e_lev", empty_level);
//    jsonWriteStr(configOptionJson, "f_lev", full_level);
//    jsonWriteStr(configOptionJson, "trig", trig);
//    jsonWriteStr(configOptionJson, "echo", echo);
//    pinMode(trig.toInt(), OUTPUT);
//    pinMode(echo.toInt(), INPUT);
//    createWidgetByType(widget_name, page_name, page_number, type, value_name);
//    sensors_reading_map[0] = 1;
//}
////ultrasonicCm cm 14 12 Дистанция,#см Датчики fillgauge 1
//void ultrasonicCm() {
//    String value_name = sCmd.next();
//    String trig = sCmd.next();
//    String echo = sCmd.next();
//    String widget_name = sCmd.next();
//    String page_name = sCmd.next();
//    String type = sCmd.next();
//    String empty_level = sCmd.next();
//    String full_level = sCmd.next();
//    String page_number = sCmd.next();
//    ultrasonicCm_value_name = value_name;
//    jsonWriteStr(configOptionJson, "trig", trig);
//    jsonWriteStr(configOptionJson, "echo", echo);
//    pinMode(trig.toInt(), OUTPUT);
//    pinMode(echo.toInt(), INPUT);
//    createWidgetByType(widget_name, page_name, page_number, type, value_name);
//    sensors_reading_map[0] = 1;
//}
//
//void ultrasonic_reading() {
//    long duration_;
//    int distance_cm;
//    int level;
//    static int counter;
//    int trig = jsonReadInt(configOptionJson, "trig");
//    int echo = jsonReadInt(configOptionJson, "echo");
//    distance_cm = medianFilter.filtered(distance_cm);  //отсечение промахов медианным фильтром
//    counter++;
//    if (counter > TANK_LEVEL_SAMPLES) {
//        counter = 0;
//        level = map(distance_cm,
//                    jsonReadInt(configOptionJson, "e_lev"),
//                    jsonReadInt(configOptionJson, "f_lev"), 0, 100);
//
//        jsonWriteInt(configLiveJson, levelPr_value_name, level);
//        eventGen(levelPr_value_name, "");
//
//        MqttClient::publishStatus(levelPr_value_name, String(level));
//
//        Serial.println("[I] sensor '" + levelPr_value_name + "' data: " + String(level));
//
//        jsonWriteInt(configLiveJson, ultrasonicCm_value_name, distance_cm);
//        eventGen(ultrasonicCm_value_name, "");
//
//        MqttClient::publishStatus(ultrasonicCm_value_name, String(distance_cm));
//
//        Serial.println("[I] sensor '" + ultrasonicCm_value_name + "' data: " + String(distance_cm));
//    }
//}
//