#include "DeviceList.h"

static const char* firstLine PROGMEM = "Тип элемента;Id;Виджет;Имя вкладки;Имя виджета;Позиция виджета";

void addElement(String name) {
    String item = readFile("items/" + name + ".txt", 1024);
    
    item.replace("id", "id" + String(getNewElementNumber("id.txt")));
    item.replace("order", String(getNewElementNumber("order.txt")));

    item.replace("\r\n", "");
    item.replace("\r", "");
    item.replace("\n", "");
    addFile("conf.csv", "\n" + item);
}

void delAllElement() {
    removeFile("conf.csv");
    addFile("conf.csv", String(firstLine));
    removeFile("id.txt");
    removeFile("order.txt");
}


int getNewElementNumber(String file) {
    int number = readFile(file, 100).toInt();
    number++;
    removeFile(file);
    addFile(file, String(number));
    return number;
}



void do_delElement() {
    if (delElementFlag) {
        delElementFlag = false;
        delElement(itemsFile, itemsLine);
    }
}

void delElement(String _itemsFile, String _itemsLine) {
    File configFile = LittleFS.open("/" + _itemsFile, "r");
    if (!configFile) {
        return;
    }
    configFile.seek(0, SeekSet);
    String finalConf;
    int count = -1;
    while (configFile.position() != configFile.size()) {
        count++;
        String item = configFile.readStringUntil('\n');
        Serial.print(_itemsLine);
        Serial.print(" ");
        Serial.println(count);
        if (count != _itemsLine.toInt()) {
            if (count == 0) {
                finalConf += item;
            } else {
                finalConf += "\n" + item;
            }
        }
    }
    removeFile(_itemsFile);
    addFile(_itemsFile, finalConf);
    Serial.println(finalConf);
    itemsFile = "";
    itemsLine = "";
    configFile.close();
}