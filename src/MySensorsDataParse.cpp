#include "Consts.h"
#ifdef MYSENSORS
#include "MySensorsDataParse.h"
#include "items/vSensorGate.h"

//для того что бы выключить оригинальный лог нужно перейти в файл библиотеки MyGatewayTransportSerial.cpp
//и заккоментировать строку 36 MY_SERIALDEVICE.print(protocolMyMessage2Serial(message));

void loopMySensorsExecute() {
    if (mysensorBuf.length()) {
        String tmp = selectToMarker(mysensorBuf, ";");

        String nodeId = selectFromMarkerToMarker(tmp, ",", 0);         //node-id
        String childSensorId = selectFromMarkerToMarker(tmp, ",", 1);  //child-sensor-id
        String type = selectFromMarkerToMarker(tmp, ",", 2);           //type of var
        String command = selectFromMarkerToMarker(tmp, ",", 3);        //command
        String value = selectFromMarkerToMarker(tmp, ",", 4);          //value

        String key = nodeId + "-" + childSensorId;
        static String infoJson = "{}";

        if (childSensorId == "255") {
            if (command == "3") {    //это особое внутреннее сообщение
                if (type == "11") {  //название ноды
                    SerialPrint("I", "MySensor", "Node name: " + value);
                }
                if (type == "12") {  //версия ноды
                    SerialPrint("I", "MySensor", "Node version: " + value);
                }
            }
        } else {
            if (command == "0") {  //это презентация
                SerialPrint("I", "MySensor", "Presentation " + key + ": " + sensorType(type.toInt()));
            }
            if (command == "1") {  //это данные
                if (value != "") {
                    if (mySensorGate != nullptr) {
                        for (unsigned int i = 0; i < mySensorGate->size(); i++) {
                            mySensorGate->at(i).onChange(value, key); //вызываем поочередно все экземпляры, там где подойдет там и выполнится
                        }
                    }
                    SerialPrint("I", "MySensor", "node: " + nodeId + ", sensor: " + childSensorId + ", command: " + command + ", type: " + type + ", val: " + value);
                }
            }
            if (command == "2") {  //это запрос значения переменной
                SerialPrint("I", "MySensor", "Request a variable value");
            }
        }

        mysensorBuf = deleteBeforeDelimiter(mysensorBuf, ";");
    }
}

String sensorType(int index) {
    switch (index) {
        case 0:
            return F("Door and window sensors");
            break;
        case 1:
            return F("Motion sensors");
            break;
        case 2:
            return F("Smoke sensor");
            break;
        case 3:
            return F("Binary device (on/off)");
            break;
        case 4:
            return F("Dimmable device of some kind");
            break;
        case 5:
            return F("Window covers or shades");
            break;
        case 6:
            return F("Temperature sensor");
            break;
        case 7:
            return F("Humidity sensor");
            break;
        case 8:
            return F("Barometer sensor (Pressure)");
            break;
        case 9:
            return F("Wind sensor");
            break;
        case 10:
            return F("Rain sensor");
            break;
        case 11:
            return F("UV sensor");
            break;
        case 12:
            return F("Weight sensor for scales etc.");
            break;
        case 13:
            return F("Power measuring device, like power meters");
            break;
        case 14:
            return F("Heater device");
            break;
        case 15:
            return F("Distance sensor");
            break;
        case 16:
            return F("Light sensor");
            break;
        case 17:
            return F("Arduino node device");
            break;
        case 18:
            return F("Arduino repeating node device");
            break;
        case 19:
            return F("Lock device");
            break;
        case 20:
            return F("Ir sender/receiver device");
            break;
        case 21:
            return F("Water meter");
            break;
        case 22:
            return F("Air quality sensor e.g. MQ-2");
            break;
        case 23:
            return F("Use this for custom sensors where no other fits.");
            break;
        case 24:
            return F("Dust level sensor");
            break;
        case 25:
            return F("Scene controller device");
            break;
        case 26:
            return F("RGB light");
            break;
        case 27:
            return F("RGBW light (with separate white component)");
            break;
        case 28:
            return F("Color sensor");
            break;
        case 29:
            return F("Thermostat/HVAC device");
            break;
        case 30:
            return F("Multimeter device");
            break;
        case 31:
            return F("Sprinkler device");
            break;
        case 32:
            return F("Water leak sensor");
            break;
        case 33:
            return F("Sound sensor");
            break;
        case 34:
            return F("Vibration sensor");
            break;
        case 35:
            return F("Moisture sensor");
            break;
        case 36:
            return F("LCD text device");
            break;
        case 37:
            return F("Gas meter");
            break;
        case 38:
            return F("GPS Sensor");
            break;
        case 39:
            return F("Water quality sensor");
            break;
        default:
            return F("Unknown");
            break;
    }
}
#endif

//отличный пример разбора строки
void test(char* inputString) {
    char *str, *p;
    uint8_t index = 0;
    for (str = strtok_r(inputString, ";", &p);
         str && index < 5;
         str = strtok_r(NULL, ";", &p), index++) {
        switch (index) {
            case 0:
                Serial.println(str);
                break;
            case 1:
                Serial.println(str);
                break;
            case 2:
                Serial.println(str);
                break;
            case 3:
                Serial.println(str);
                break;
            case 4:
                Serial.println(str);
                break;
        }
    }
}
