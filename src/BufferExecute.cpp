#include "BufferExecute.h"

#include "Global.h"
#include "SoftUART.h"
#include "items/test.h"
#include "items/vButtonOut.h"
#include "items/vCountDown.h"
#include "items/vImpulsOut.h"
#include "items/vInput.h"
#include "items/vLogging.h"
#include "items/vOutput.h"
#include "items/vPwmOut.h"
#include "items/vSensorAnalog.h"
#include "items/vSensorBme280.h"
#include "items/vSensorBmp280.h"
#include "items/vSensorCcs811.h"
#include "items/vSensorDallas.h"
#include "items/vSensorDht.h"
#include "items/vSensorPzem.h"
#include "items/vSensorUltrasonic.h"
#include "items/vSensorUptime.h"

void loopCmdAdd(const String& cmdStr) {
    if (cmdStr.endsWith(",")) {
        orderBuf += cmdStr;
#ifdef EnableUart
        if (jsonReadBool(configSetupJson, "uart")) {
            if (jsonReadBool(configSetupJson, "uartEvents")) {
                if (myUART) {
                    myUART->print(cmdStr);
                    SerialPrint("I", "<=UART", cmdStr);
                }
            }
        }
#endif
    }
}

void fileCmdExecute(const String& filename) {
    String cmdStr = readFile(filename, 4096);
    csvCmdExecute(cmdStr);
}

void csvCmdExecute(String& cmdStr) {
    cmdStr.replace(";", " ");
    cmdStr += "\r\n";
    cmdStr.replace("\r\n", "\n");
    cmdStr.replace("\r", "\n");
    int count = 0;
    while (cmdStr.length()) {
        String buf = selectToMarker(cmdStr, "\n");

        buf = deleteBeforeDelimiter(buf, " ");  //отсечка чекбокса

        count++;

        if (count > 1) {
            SerialPrint("I", "Items", buf);
            String order = selectToMarker(buf, " ");  //отсечка самой команды
            if (order == F("button-out")) {
#ifdef EnableButtonOut
                sCmd.addCommand(order.c_str(), buttonOut);
#endif
            } else if (order == F("pwm-out")) {
#ifdef EnablePwmOut
                sCmd.addCommand(order.c_str(), pwmOut);
#endif
            } else if (order == F("button-in")) {
#ifdef EnableButtonIn
                sCmd.addCommand(order.c_str(), buttonIn);
#endif
            } else if (order == F("input")) {
#ifdef EnableInput
                sCmd.addCommand(order.c_str(), input);
#endif
            } else if (order == F("output")) {
#ifdef EnableOutput
                sCmd.addCommand(order.c_str(), output);
#endif
            } else if (order == F("analog-adc")) {
#ifdef EnableSensorAnalog
                sCmd.addCommand(order.c_str(), analogAdc);
#endif
            } else if (order == F("ultrasonic-cm")) {
#ifdef EnableSensorUltrasonic
                sCmd.addCommand(order.c_str(), ultrasonic);
#endif
            } else if (order == F("dallas-temp")) {
#ifdef EnableSensorDallas
                sCmd.addCommand(order.c_str(), dallas);
#endif
            } else if (order == F("dht")) {
#ifdef EnableSensorDht
                sCmd.addCommand(order.c_str(), dhtSensor);
#endif
            } else if (order == F("bme280")) {
#ifdef EnableSensorBme280
                sCmd.addCommand(order.c_str(), bme280Sensor);
#endif
            } else if (order == F("bmp280")) {
#ifdef EnableSensorBmp280
                sCmd.addCommand(order.c_str(), bmp280Sensor);
#endif
            } else if (order == F("ccs811")) {
#ifdef EnableSensorCcs811
                sCmd.addCommand(order.c_str(), ccs811Sensor);
#endif
            } else if (order == F("pzem")) {
#ifdef EnableSensorPzem
                sCmd.addCommand(order.c_str(), pzemSensor);
#endif
            } else if (order == F("uptime")) {
#ifdef EnableSensorUptime
                sCmd.addCommand(order.c_str(), uptimeSensor);
#endif
            } else if (order == F("logging")) {
#ifdef EnableLogging
                sCmd.addCommand(order.c_str(), logging);
#endif
            } else if (order == F("impuls-out")) {
#ifdef EnableImpulsOut
                sCmd.addCommand(order.c_str(), impuls);
#endif
            } else if (order == F("count-down")) {
#ifdef EnableCountDown
                sCmd.addCommand(order.c_str(), countDown);
#endif
            } else if (order == F("impuls-in")) {
#ifdef EnableImpulsIn
                //sCmd.addCommand(order.c_str(), impulsInSensor);
#endif
            }

            sCmd.readStr(buf);
        }
        cmdStr = deleteBeforeDelimiter(cmdStr, "\n");
    }
}

void spaceCmdExecute(String& cmdStr) {
    cmdStr += "\r\n";
    cmdStr.replace("\r\n", "\n");
    cmdStr.replace("\r", "\n");
    while (cmdStr.length()) {
        String buf = selectToMarker(cmdStr, "\n");
        sCmd.readStr(buf);
        cmdStr = deleteBeforeDelimiter(cmdStr, "\n");
    }
}

void loopCmdExecute() {
    if (orderBuf.length()) {
        String tmp = selectToMarker(orderBuf, ",");  //выделяем первую команду rel 5 1,
        SerialPrint("I", "CMD", "do: " + tmp);
        sCmd.readStr(tmp);                                //выполняем
        orderBuf = deleteBeforeDelimiter(orderBuf, ",");  //осекаем
    }
}

void addKey(String& key, String& keyNumberTable, int number) {
    keyNumberTable += key + " " + String(number) + ",";
}

int getKeyNum(String& key, String& keyNumberTable) {
    String keyNumberTableBuf = keyNumberTable;

    int number = -1;
    while (keyNumberTableBuf.length()) {
        String tmp = selectToMarker(keyNumberTableBuf, ",");
        String keyIncomming = selectToMarker(tmp, " ");
        if (keyIncomming == key) {
            number = selectToMarkerLast(tmp, " ").toInt();
        }
        keyNumberTableBuf = deleteBeforeDelimiter(keyNumberTableBuf, ",");
    }
    return number;
}

String getValue(String& key) {
    String live = jsonReadStr(configLiveJson, key);
    String store = jsonReadStr(configStoreJson, key);
    if (live != nullptr) {
        return live;
    } else if (store != nullptr) {
        return store;
    } else if (store == nullptr && live == nullptr) {
        return "no value";
    } else {
        return "data error";
    }
}

void loopMySensorsExecute() {
#ifdef MYSENSORS
    if (mysensorBuf.length()) {
        String tmp = selectToMarker(mysensorBuf, ";");

        String nodeId = selectFromMarkerToMarker(tmp, ",", 0);         //node-id
        String childSensorId = selectFromMarkerToMarker(tmp, ",", 1);  //child-sensor-id
        String type = selectFromMarkerToMarker(tmp, ",", 2);           //type
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
                SerialPrint("I", "MySensor", "New device presentation");
            }
            if (command == "1") {  //это данные
                if (value != "") {
                    eventGen2(key, value);
                    jsonWriteStr(configLiveJson, key, value);
                    publishStatus(key, value);
                    jsonWriteStr(configTimesJson, key, "0");
                    publishLastUpdateTime(key, "0 min");
                    SerialPrint("I", "MySensor", "node: " + nodeId + ", sensor: " + childSensorId + ", command: " + command + ", type: " + type + ", val: " + value);
                }
            }
            if (command == "2") {  //это запрос значения переменной
                SerialPrint("I", "MySensor", "Request a variable value");
            }
        }

        mysensorBuf = deleteBeforeDelimiter(mysensorBuf, ";");
    }
#endif
}

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