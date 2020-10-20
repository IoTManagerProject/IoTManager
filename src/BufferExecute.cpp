#include "BufferExecute.h"

#include "Global.h"
#include "Module/Terminal.h"

void loopCmdAdd(const String &cmdStr) {
    orderBuf += cmdStr;
    if (!cmdStr.endsWith(",")) {
        orderBuf += ",";
    }
}

void fileCmdExecute(const String &filename) {
    String cmdStr = readFile(filename, 4096);
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
        buf = deleteBeforeDelimiter(buf, " ");  //отсечка чекбокса
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
        SerialPrint("I", "CMD", "do: " + tmp);
        sCmd.readStr(tmp);                                //выполняем
        orderBuf = deleteBeforeDelimiter(orderBuf, ",");  //осекаем
    }
}

void sensorsInit() {
    ts.add(
        SENSORS10SEC, 10000, [&](void *) {
            String buf = sensorReadingMap10sec;
            while (buf.length()) {
                String tmp = selectToMarker(buf, ",");
                sCmd.readStr(tmp);
                buf = deleteBeforeDelimiter(buf, ",");
            }
        },
        nullptr, true);

    ts.add(
        SENSORS60SEC, 60000, [&](void *) {
            String buf = sensorReadingMap60sec;
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
