#include "DeviceList.h"

static const char* firstLine PROGMEM = "Удалить;Тип элемента;Id;Виджет;Имя вкладки;Имя виджета;Позиция виджета";

void addElement(String name) {
    String item = readFile("items/" + name + ".txt", 1024);
    item.replace("\r\n", "");
    item.replace("\r", "");
    item.replace("\n", "");
    addFile("conf.csv", "\n" + item);
}

void delAllElement() {
    removeFile("conf.csv");
    addFile("conf.csv", String(firstLine));
}

void do_delElement() {
    if (delElementFlag) {
        delElementFlag = false;
        delElement();
    }
}

void delElement() {
    File configFile = LittleFS.open("/conf.csv", "r");
    if (!configFile) {
        return;
    }
    configFile.seek(0, SeekSet);  //поставим курсор в начало файла
    String finalConf;
    while (configFile.position() != configFile.size()) {
        String item = configFile.readStringUntil('\n');
        if (selectToMarker(item, ";") == "0") {
            finalConf += "\n" + item;
        }
    }
    removeFile("conf.csv");
    addFile("conf.csv", String(firstLine) + "\n" + finalConf);
    configFile.close();
}
