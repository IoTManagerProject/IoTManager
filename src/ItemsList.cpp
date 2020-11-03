#include "ItemsList.h"

#include "Class/NotAsync.h"
#include "Init.h"
#include "Utils/StringUtils.h"

static const char* firstLine PROGMEM = "Удалить;Тип элемента;Id;Виджет;Имя вкладки;Имя виджета;Позиция виджета";

void itemsListInit() {
    myNotAsyncActions->add(
        do_deviceInit, [&](void*) {
            Device_init();
        },
        nullptr);

    myNotAsyncActions->add(
        do_delChoosingItems, [&](void*) {
            delChoosingItems();
        },
        nullptr);
}

void addItem(String name) {
    String item = readFile("items/" + name + ".txt", 1024);

    name = deleteToMarkerLast(name, ".");

    item.replace("id", name + "-" + String(getNewElementNumber("id.txt")));
    item.replace("order", String(getNewElementNumber("order.txt")));

    if (item.indexOf("pin") != -1) {  //all cases (random pins from available)
        item.replace("pin", "pin[" + String(getFreePinAll()) + "]");
    } else

        if (item.indexOf("gol") != -1) {  //analog
        item.replace("gol", "pin[" + String(getFreePinAnalog()) + "]");
    } else

        if (item.indexOf("cin") != -1) {  //ultrasonic
        item.replace("cin", "pin[" + String(getFreePinAll()) + "," + String(getFreePinAll()) + "]");
    } else

        if (item.indexOf("sal") != -1) {  //dallas
        item.replace("sal", "pin[2]");
    } else

        if (item.indexOf("thd") != -1) {  //dht11/22
        item.replace("thd", "pin[2]");
    }

    item.replace("\r\n", "");
    item.replace("\r", "");
    item.replace("\n", "");
    addFile(DEVICE_CONFIG_FILE, "\n" + item);
}

void addPreset(String name) {
    String preset = readFile("presets/" + name + ".txt", 4048);
    addFile(DEVICE_CONFIG_FILE, "\n" + preset);

    name.replace(".c",".s");

    String scenario = readFile("presets/" + name + ".txt", 4048);
    removeFile(DEVICE_SCENARIO_FILE);
    addFile(DEVICE_SCENARIO_FILE, scenario);
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

uint8_t getFreePinAll() {
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

uint8_t getFreePinAnalog() {
#ifdef ESP8266
    return 0;
#endif
}

void delChoosingItems() {
    File configFile = LittleFS.open("/" + String(DEVICE_CONFIG_FILE), "r");
    if (!configFile) {
        return;
    }
    configFile.seek(0, SeekSet);
    String finalConf;
    bool firstLine = true;
    while (configFile.position() != configFile.size()) {
        String item = configFile.readStringUntil('\n');
        if (firstLine) {
            finalConf += item;
        } else {
            int checkbox = selectToMarker(item, ";").toInt();
            if (checkbox == 0) {
                finalConf += "\n" + item;
            }
        }
        firstLine = false;
    }
    removeFile(String(DEVICE_CONFIG_FILE));
    addFile(String(DEVICE_CONFIG_FILE), finalConf);
    Serial.println(finalConf);
    configFile.close();
}