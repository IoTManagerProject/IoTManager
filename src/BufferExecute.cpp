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
#ifdef uartEnable
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
                sCmd.addCommand(order.c_str(), buttonOut);
            } else if (order == F("pwm-out")) {
                sCmd.addCommand(order.c_str(), pwmOut);
            } else if (order == F("button-in")) {
                sCmd.addCommand(order.c_str(), buttonIn);
            } else if (order == F("input")) {
                sCmd.addCommand(order.c_str(), input);
            } else if (order == F("output")) {
                sCmd.addCommand(order.c_str(), output);
            } else if (order == F("analog-adc")) {
                sCmd.addCommand(order.c_str(), analogAdc);
            } else if (order == F("ultrasonic-cm")) {
                sCmd.addCommand(order.c_str(), ultrasonic);
            } else if (order == F("dallas-temp")) {
                sCmd.addCommand(order.c_str(), dallas);
            } else if (order == F("dht")) {
                sCmd.addCommand(order.c_str(), dhtSensor);
            } else if (order == F("bme280")) {
                sCmd.addCommand(order.c_str(), bme280Sensor);
            } else if (order == F("bmp280")) {
                sCmd.addCommand(order.c_str(), bmp280Sensor);
            } else if (order == F("ccs811")) {
                sCmd.addCommand(order.c_str(), ccs811Sensor);
            } else if (order == F("pzem")) {
                sCmd.addCommand(order.c_str(), pzemSensor);
            } else if (order == F("uptime")) {
                sCmd.addCommand(order.c_str(), uptimeSensor);
            } else if (order == F("logging")) {
                sCmd.addCommand(order.c_str(), logging);
            } else if (order == F("impuls-out")) {
                sCmd.addCommand(order.c_str(), impuls);
            } else if (order == F("count-down")) {
                sCmd.addCommand(order.c_str(), countDown);
            } else if (order == F("impuls-in")) {
                //sCmd.addCommand(order.c_str(), impulsInSensor);
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
    if (mysensorBuf.length()) {
        String tmp = selectToMarker(mysensorBuf, ";");

        String type = selectFromMarkerToMarker(tmp, ",", 0);
        String nodeId = selectFromMarkerToMarker(tmp, ",", 1);
        String childSensorId = selectFromMarkerToMarker(tmp, ",", 2);
        String value = selectFromMarkerToMarker(tmp, ",", 3);

        String key = nodeId + "-" + childSensorId;
        static String infoJson = "{}";

        if (childSensorId == "255") {  //это презентация

            String prevValue = jsonReadStr(infoJson, nodeId);
            prevValue += value + " ";
            jsonWriteStr(infoJson, nodeId, prevValue);

            //100;255;0;0;17;2.3.2 версия библиотеки
            //100;255;3;0;6;255
            //100;255;3;0;11;Passive node имя
            //100;255;3;0;12;1.0

            SerialPrint("I", "MySensor", "New device connected " + infoJson);
        } else {  //это данные
            if (value != "") {
                eventGen2(key, value);
                jsonWriteStr(configLiveJson, key, value);
                publishStatus(key, value);
                SerialPrint("I", "MySensor", "Payload Type:" + type + ", NodeId:" + nodeId + ", ChildId:" + childSensorId + ", Payload:" + value);
            }
        }
        mysensorBuf = deleteBeforeDelimiter(mysensorBuf, ";");
    }
}