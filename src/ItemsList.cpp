#include "ItemsList.h"

#include "Class/NotAsync.h"
#include "FileSystem.h"
#include "Init.h"
#include "Utils/StringUtils.h"

static const char* firstLine PROGMEM = "Удалить;Тип элемента;Id;Виджет;Имя вкладки;Имя виджета;Позиция виджета";

void itemsListInit() {
    myNotAsyncActions->add(
        do_deviceInit, [&](void*) {
            deviceInit();
        },
        nullptr);

    myNotAsyncActions->add(
        do_delChoosingItems, [&](void*) {
            delChoosingItems();
        },
        nullptr);

    SerialPrint("I", F("Items"), F("Items Init"));
}

void addItem2(int num) {
    File configFile = FileFS.open("/items/items.txt", "r");
    if (!configFile) {
        return;
    }
    configFile.seek(0, SeekSet);
    String seachingLine;
    int i = 0;
    while (configFile.position() != configFile.size()) {
        i++;
        String item = configFile.readStringUntil('*');
        if (i == num) {
            if (i == 1) {
                seachingLine = "\n" + item;
            } else {
                seachingLine = item;
            }
            break;
        }
    }
    configFile.close();

    randomSeed(micros());
    unsigned int rnd = random(0, 1000);
    seachingLine.replace("id", String(rnd));
    seachingLine.replace("order", String(getNewElementNumber("order.txt")));
    if (seachingLine.indexOf("gpio") != -1) {
        seachingLine.replace("gpio", "pin[" + String(getFreePinAll()) + "]");
    }

    addFile(DEVICE_CONFIG_FILE, seachingLine);
    Serial.println(seachingLine);
}

void addPreset2(int num) {
    File configFile = FileFS.open("/presets/presets.c.txt", "r");
    if (!configFile) {
        return;
    }
    configFile.seek(0, SeekSet);
    String config;
    int i1 = 0;
    while (configFile.position() != configFile.size()) {
        i1++;
        String item = configFile.readStringUntil('*');
        if (i1 == num) {
            if (i1 == 1) {
                config = "\n" + item;
            } else {
                config = item;
            }
            break;
        }
    }
    configFile.close();
    addFile(DEVICE_CONFIG_FILE, config);
    //===========================================================================
    File scenFile = FileFS.open("/presets/presets.s.txt", "r");
    if (!scenFile) {
        return;
    }
    scenFile.seek(0, SeekSet);
    String scen;
    int i2 = 0;
    while (scenFile.position() != scenFile.size()) {
        i2++;
        String item = scenFile.readStringUntil('*');
        if (i2 == num) {
            if (i1 == 1) {
                scen = "\n" + item;
            } else {
                scen = item;
            }
            break;
        }
    }
    scenFile.close();
    if (readFile(String(DEVICE_SCENARIO_FILE), 2048) == "//") {
        removeFile(DEVICE_SCENARIO_FILE);
        scen = deleteBeforeDelimiter(scen, "\n");
        addFile(DEVICE_SCENARIO_FILE, scen);
    } else {
        addFile(DEVICE_SCENARIO_FILE, scen);
    }
}

void delAllItems() {
    removeFile(DEVICE_CONFIG_FILE);
    addFile(DEVICE_CONFIG_FILE, String(firstLine));
    removeFile(DEVICE_SCENARIO_FILE);
    addFile(DEVICE_SCENARIO_FILE, "//");
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
    uint8_t pins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39};
#endif
    uint8_t array_sz = sizeof(pins) / sizeof(pins[0]);
    uint8_t i = getNewElementNumber("pins.txt");
    if (i < array_sz) {
        return pins[i];
    } else {
        return 0;
    }
}

bool isPinExist(unsigned int num) {
    bool ret = false;
#ifdef ESP8266
    unsigned int pins[] = {0, 1, 2, 3, 4, 5, 9, 10, 12, 13, 14, 15, 16};
#endif
#ifdef ESP32
    unsigned int pins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39};
#endif
    uint8_t array_sz = sizeof(pins) / sizeof(pins[0]);
    for (uint8_t i = 0; i < array_sz; i++) {
        if (pins[i] == num) ret = true;
    }
    return ret;
}

uint8_t getFreePinAnalog() {
#ifdef ESP8266
    return 0;
#endif
}

void delChoosingItems() {
    File configFile = FileFS.open("/" + String(DEVICE_CONFIG_FILE), "r");
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