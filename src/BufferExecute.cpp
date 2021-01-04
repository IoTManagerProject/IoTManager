#include "BufferExecute.h"
#include "Global.h"
#include "SoftUART.h"
//
#include "items/sysUptime.h"
#include "items/vSensorDallas.h"
#include "items/vSensorUltrasonic.h"
#include "items/vButtonOut.h"
#include "items/vPwmOut.h"
#include "items/vInOutput.h"
#include "items/vLogging.h"
#include "items/vImpulsOut.h"
#include "items/vCountDown.h"
#include "items/vSensorAnalog.h"
#include "items/vSensorDht.h"
#include "items/vSensorBme280.h"
#include "items/vSensorBmp280.h"

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
            String order = selectToMarker(buf, " "); //отсечка самой команды

            if (order == F("button-out")) {
                sCmd.addCommand(order.c_str(), buttonOut);
            }
#ifdef PwmOutEnable
            else if (order == F("pwm-out")) {
                sCmd.addCommand(order.c_str(), pwmOut);
            }
#endif
            else if (order == F("button-in")) {
                sCmd.addCommand(order.c_str(), buttonIn);
            }
            else if (order == F("inoutput")) {
                sCmd.addCommand(order.c_str(), inOutput);
            }
            else if (order == F("analog-adc")) {
                sCmd.addCommand(order.c_str(), analogAdc);
            }
            else if (order == F("ultrasonic-cm")) {
                sCmd.addCommand(order.c_str(), ultrasonic);
            }
            else if (order == F("dallas-temp")) {
                sCmd.addCommand(order.c_str(), dallas);
            }
#ifdef SensorDhtEnabled
            else if (order == F("dht")) {
                sCmd.addCommand(order.c_str(), dhtSensor);
            }
#endif
#ifdef SensorBme280Enabled
            else if (order == F("bme280")) {
                sCmd.addCommand(order.c_str(), bme280Sensor);
            }
#endif
#ifdef SensorBmp280Enabled
            else if (order == F("bmp280")) {
                sCmd.addCommand(order.c_str(), bmp280Sensor);
            }
#endif
            else if (order == F("uptime")) {
                sCmd.addCommand(order.c_str(), sysUptime);
            }
            else if (order == F("logging")) {
                sCmd.addCommand(order.c_str(), logging);
            }
            else if (order == F("impuls-out")) {
                sCmd.addCommand(order.c_str(), impuls);
            }
            else if (order == F("count-down")) {
                sCmd.addCommand(order.c_str(), countDown);
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
    }
    else if (store != nullptr) {
        return store;
    }
    else if (store == nullptr && live == nullptr) {
        return "no value";
    }
    else {
        return "data error";
    }
}
