#include "BufferExecute.h"
#include "Global.h"
#include "Module/Terminal.h"
#include "Errors.h"

static const char *MODULE = "Cmd";

void loopCmdAdd(const String &cmdStr) {
    orderBuf += cmdStr;
    if (!cmdStr.endsWith(",")) {
        orderBuf += ",";
    }
}

void fileCmdExecute(const String &filename) {
    String cmdStr = readFile(filename, 2048);
    csvCmdExecute(cmdStr);
}

void csvCmdExecute(String &cmdStr) {
    cmdStr.replace(";", " ");
    cmdStr += "\r\n";
    cmdStr.replace("\r\n", "\n");
    cmdStr.replace("\r", "\n");
    int count = 0;
    while (cmdStr.length()) {
        String buf = selectToMarker(cmdStr, "\n");
        count++;
        if (count > 1) sCmd.readStr(buf);
        cmdStr = deleteBeforeDelimiter(cmdStr, "\n");
    }
}

void spaceCmdExecute(String &cmdStr) {
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
        pm.info("do: " + tmp);
        sCmd.readStr(tmp);                                    //выполняем
        orderBuf = deleteBeforeDelimiter(orderBuf, ",");  //осекаем
    }
}

void sensorsInit() {
    ts.add(
        SENSORS, 10000, [&](void *) {
            String buf = sensorReadingMap;
            while (buf.length()) {
                String tmp = selectToMarker(buf, ",");
                sCmd.readStr(tmp);
                buf = deleteBeforeDelimiter(buf, ",");
            }
        },
        nullptr, true);
}

//void loopSerial() {
//    if (term) {
//        term->loop();
//    }
//}
