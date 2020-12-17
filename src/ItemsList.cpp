#include "ItemsList.h"

#include "Class/NotAsync.h"
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

void addItem2(String param) {
    int num = selectToMarker(param, "-").toInt();
    File configFile = LittleFS.open("/items/items.txt", "r");
    if (!configFile) {
        return;
    }
    configFile.seek(0, SeekSet);
    String seachingLine;

    while (configFile.position() != configFile.size()) {
        String item = configFile.readStringUntil('\n');
        int tmpNum = selectToMarker(item, ";").toInt();
        if (tmpNum == num) {
            seachingLine = item;
            break;
        }
    }
    configFile.close();

    String name = deleteBeforeDelimiter(param, "-");
    randomSeed(micros());
    unsigned int rnd = random(0, 1000);
    seachingLine.replace("id", name + String(rnd));
    seachingLine.replace("order", String(getNewElementNumber("order.txt")));

    if (seachingLine.indexOf("pin") != -1) {
        seachingLine.replace("pin", "pin[" + String(getFreePinAll()) + "]");
    }

    seachingLine = deleteBeforeDelimiter(seachingLine, ";");
    seachingLine.replace("\r\n", "");
    seachingLine.replace("\r", "");
    seachingLine.replace("\n", "");
    addFile(DEVICE_CONFIG_FILE, "\n" + seachingLine);
    Serial.println(seachingLine);
}


void addPreset2(int num) {
    File configFile = LittleFS.open("/presets/presets.c.txt", "r");
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
            }
            else {
                config = item;
            }
            break;
        }
    }
    configFile.close();
    addFile(DEVICE_CONFIG_FILE, config);

    File scenFile = LittleFS.open("/presets/presets.s.txt", "r");
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
            scen = item;
            break;
        }
    }
    scenFile.close();
    if (readFile(String(DEVICE_SCENARIO_FILE), 2048) == "//") {
        removeFile(DEVICE_SCENARIO_FILE);
    }
    addFile(DEVICE_SCENARIO_FILE, scen);
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
    uint8_t pins[] = { 0, 12, 13, 14, 15, 16, 1, 2, 3, 4, 5 };
#endif
#ifdef ESP32
    uint8_t pins[] = { 0, 12, 13, 14, 15, 16, 1, 2, 3, 4, 5 };
#endif
    uint8_t array_sz = sizeof(pins) / sizeof(pins[0]);
    uint8_t i = getNewElementNumber("pins.txt");
    if (i < array_sz) {
        return pins[i];
    }
    else {
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
        }
        else {
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