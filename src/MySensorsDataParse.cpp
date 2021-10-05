#include "Consts.h"
#ifdef MYSENSORS
#include "Class/NotAsync.h"
#include "ItemsList.h"
#include "MySensorsDataParse.h"
#include "items/vSensorNode.h"

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

        static bool presentBeenStarted = false;

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
                presentBeenStarted = true;
                int num;
                String widget;
                String descr;
                sensorType(type.toInt(), num, widget, descr);
                if (jsonReadBool(configSetupJson, "gateAuto")) {
                    if (!isItemAdded(key)) {
                        addItemAuto(num, key, widget, descr);
                        descr.replace("#", " ");
                        SerialPrint("I", "MySensor", "Add new item: " + key + ": " + descr);
                    } else {
                        descr.replace("#", " ");
                        SerialPrint("I", "MySensor", "Item already exist: " + key + ": " + descr);
                    }
                } else {
                    descr.replace("#", " ");
                    SerialPrint("I", "MySensor", "Presentation: " + key + ": " + descr);
                }
            }
            if (command == "1") {  //это данные
                if (value != "") {
                    if (presentBeenStarted) {
                        presentBeenStarted = false;
                        SerialPrint("I", "MySensor", "!!!Presentation of node: " + nodeId + " completed successfully!!!");
                        myNotAsyncActions->make(do_deviceInit);
                    }
                    if (mySensorNode != nullptr) {
                        for (unsigned int i = 0; i < mySensorNode->size(); i++) {
                            mySensorNode->at(i).onChange(value, key);  //вызываем поочередно все экземпляры, там где подойдет там и выполнится
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

void sensorType(int index, int &num, String &widget, String &descr) {
    switch (index) {
        case 0:
            descr = F("Door#and#window#sensors");
            widget = F("alarm");
            num = 1;
            break;
        case 1:
            descr = F("Motion#sensors");
            widget = F("alarm");
            num = 1;
            break;
        case 2:
            descr = F("Smoke#sensor");
            widget = F("fillgauge");
            num = 1;
            break;
        case 3:
            descr = F("Binary#device#(on/off)");
            widget = F("toggleBtn");
            num = 2;
            break;
        case 4:
            descr = F("Dimmable#device");
            //to do
            //widget = F("range");
            //num = 2;
            break;
        case 5:
            descr = F("Window#covers#or#shades");
            //to do
            //widget = F("range");
            //num = 2;
            break;
        case 6:
            descr = F("Temperature#sensor");
            widget = F("anydataTemp");
            num = 1;
            break;
        case 7:
            descr = F("Humidity#sensor");
            widget = F("anydataHum");
            num = 1;
            break;
        case 8:
            descr = F("Pressure#sensor");
            widget = F("anydataPress");
            num = 1;
            break;
        case 9:
            descr = F("Wind#sensor");
            widget = F("anydataTime");
            num = 1;
            break;
        case 10:
            descr = F("Rain#sensor");
            widget = F("anydataTime");
            num = 1;
            break;
        case 11:
            descr = F("UV#sensor");
            widget = F("anydataTime");
            num = 1;
            break;
        case 12:
            descr = F("Weight#sensor");
            widget = F("anydataTime");
            num = 1;
            break;
        case 13:
            descr = F("Power#measuring#device");
            widget = F("anydataWtt");
            num = 1;
            break;
        case 14:
            descr = F("Heater#device");
            widget = F("anydataTemp");
            num = 1;
            break;
        case 15:
            descr = F("Distance#sensor");
            widget = F("anydata");
            num = 1;
            break;
        case 16:
            descr = F("Light#sensor");
            widget = F("anydataTime");
            num = 1;
            break;
        case 17:
            descr = F("Arduino#node#device");
            widget = F("anydata");
            num = 1;
            break;
        case 18:
            descr = F("Arduino#repeating#node#device");
            widget = F("anydata");
            num = 1;
            break;
        case 19:
            descr = F("Lock#device");
            widget = F("toggleBtn");
            num = 2;
            break;
        case 20:
            descr = F("Ir#sender/receiver#device");
            widget = F("toggleBtn");
            num = 2;
            break;
        case 21:
            descr = F("Water#meter");
            widget = F("anydata");
            num = 1;
            break;
        case 22:
            descr = F("Air#quality#sensor");
            widget = F("anydata");
            num = 1;
            break;
        case 23:
            descr = F("Custom#sensor");
            widget = F("anydata");
            num = 1;
            break;
        case 24:
            descr = F("Dust#level#sensor");
            widget = F("anydata");
            num = 1;
            break;
        case 25:
            descr = F("Scene#controller#device");
            widget = F("anydata");
            num = 1;
            break;
        case 26:
            descr = F("RGB#light");
            widget = F("anydata");
            num = 1;
            break;
        case 27:
            descr = F("RGBW#light#(with#separate#white#component)");
            widget = F("anydata");
            num = 1;
            break;
        case 28:
            descr = F("Color#sensor");
            widget = F("anydata");
            num = 1;
            break;
        case 29:
            descr = F("Thermostat/HVAC#device");
            widget = F("anydata");
            num = 1;
            break;
        case 30:
            descr = F("Multimeter#device");
            widget = F("anydataVlt");
            num = 1;
            break;
        case 31:
            descr = F("Sprinkler#device");
            widget = F("anydata");
            num = 1;
            break;
        case 32:
            descr = F("Water#leak#sensor");
            widget = F("alarm");
            num = 1;
            break;
        case 33:
            descr = F("Sound#sensor");
            widget = F("anydata");
            num = 1;
            break;
        case 34:
            descr = F("Vibration#sensor");
            widget = F("anydata");
            num = 1;
            break;
        case 35:
            descr = F("Moisture#sensor");
            widget = F("anydata");
            num = 1;
            break;
        case 36:
            descr = F("LCD#text#device");
            widget = F("anydata");
            num = 1;
            break;
        case 37:
            descr = F("Gas#meter");
            widget = F("anydata");
            num = 1;
            break;
        case 38:
            descr = F("GPS#Sensor");
            widget = F("anydata");
            num = 1;
            break;
        case 39:
            descr = F("Water#quality#sensor");
            widget = F("anydata");
            num = 1;
            break;
        default:
            descr = F("Unknown");
            widget = F("anydata");
            num = 1;
            break;
    }
}
#endif

//отличный пример разбора строки
//void test(char *inputString) {
//    char *str, *p;
//    uint8_t index = 0;
//    for (str = strtok_r(inputString, ";", &p);
//         str && index < 5;
//         str = strtok_r(NULL, ";", &p), index++) {
//        switch (index) {
//            case 0:
//                Serial.println(str);
//                break;
//            case 1:
//                Serial.println(str);
//                break;
//            case 2:
//                Serial.println(str);
//                break;
//            case 3:
//                Serial.println(str);
//                break;
//            case 4:
//                Serial.println(str);
//                break;
//        }
//    }
//}
