#include "ItemsList.h"
#include "Utils\StringUtils.h"

static const char* firstLine PROGMEM = "Тип элемента;Id;Виджет;Имя вкладки;Имя виджета;Позиция виджета";

void addItem(String name) {
    String item = readFile("items/" + name + ".txt", 1024);
    name = deleteToMarkerLast(name, "-");
    item.replace("id", name + String(getNewElementNumber("id.txt")));
    item.replace("order", String(getNewElementNumber("order.txt")));
    item.replace("pin", "pin[" + String(getFreePin()) + "]");
    item.replace("\r\n", "");
    item.replace("\r", "");
    item.replace("\n", "");
    addFile(DEVICE_CONFIG_FILE, "\n" + item);
}

void delAllItems() {
    removeFile(DEVICE_CONFIG_FILE);
    addFile(DEVICE_CONFIG_FILE, String(firstLine));
    removeFile("id.txt");
    removeFile("order.txt");
    removeFile("pins.txt");
}

uint8_t getNewElementNumber(String file) {
    uint8_t number = readFile(file, 100).toInt();
    number++;
    removeFile(file);
    addFile(file, String(number));
    return number;
}

uint8_t getFreePin() {
    #ifdef ESP8266
    uint8_t pins[] = {0, 12, 13, 14, 15, 16, 1, 2, 3, 4, 5};
    #endif
    #ifdef ESP32
    uint8_t pins[] = {0, 12, 13, 14, 15, 16, 1, 2, 3, 4, 5};
    #endif
    uint8_t array_sz = sizeof(pins) / sizeof(pins[0]);
    uint8_t i = getNewElementNumber("pins.txt");
    if (i < array_sz) {
        return pins[i];
    } else {
        return 0;
    }
}

//void do_getJsonListFromCsv() {
//    if (getJsonListFromCsvFlag) {
//        getJsonListFromCsvFlag = false;
//        removeFile("items/items.json");
//        addFile("items/items.json", getJsonListFromCsv(DEVICE_CONFIG_FILE, 1));
//    }
//}
//
//String getJsonListFromCsv(String csvFile, int colum) {
//    File configFile = LittleFS.open("/" + csvFile, "r");
//    if (!configFile) {
//        return "error";
//    }
//    configFile.seek(0, SeekSet);
//
//    String outJson = "{}";
//
//    int count = -1;
//
//    while (configFile.position() != configFile.size()) {
//        count++;
//        String item = configFile.readStringUntil('\n');
//        if (count > 0) {
//            String line = selectFromMarkerToMarker(item, ";", colum);
//            jsonWriteStr(outJson, line, line);
//        }
//    }
//    configFile.close();
//    csvFile = "";
//    return outJson;
//}
//
//void do_delElement() {
//    if (delElementFlag) {
//        delElementFlag = false;
//        delElement(itemsFile, itemsLine);
//    }
//}
//
//void delElement(String _itemsFile, String _itemsLine) {
//    File configFile = LittleFS.open("/" + _itemsFile, "r");
//    if (!configFile) {
//        return;
//    }
//    configFile.seek(0, SeekSet);
//    String finalConf;
//    int count = -1;
//    while (configFile.position() != configFile.size()) {
//        count++;
//        String item = configFile.readStringUntil('\n');
//        Serial.print(_itemsLine);
//        Serial.print(" ");
//        Serial.println(count);
//        if (count != _itemsLine.toInt()) {
//            if (count == 0) {
//                finalConf += item;
//            } else {
//                finalConf += "\n" + item;
//            }
//        }
//    }
//    removeFile(_itemsFile);
//    addFile(_itemsFile, finalConf);
//    Serial.println(finalConf);
//    itemsFile = "";
//    itemsLine = "";
//    configFile.close();
//}