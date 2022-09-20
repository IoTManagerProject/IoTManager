#include "EspFileSystem.h"
#include "Global.h"

bool fileSystemInit() {
    if (!FileFS.begin()) {
        SerialPrint(F("E"), F("FS"), F("Init ERROR, may be FS was not flashed"));
        return false;
    }
    SerialPrint(F("i"), F("FS"), F("Init completed"));
    return true;
}

void globalVarsSync() {
    settingsFlashJson = readFile(F("settings.json"), 4096);
    settingsFlashJson.replace("\r\n", "");

    mqttPrefix = jsonReadStr(settingsFlashJson, F("mqttPrefix"));
    mqttRootDevice = mqttPrefix + "/" + chipId;
    jsonWriteStr_(settingsFlashJson, "root", mqttRootDevice);
    jsonWriteStr_(settingsFlashJson, "id", chipId);

    jsonWriteStr_(errorsHeapJson, "errors", "");  //метка для парсинга
    jsonWriteStr_(ssidListHeapJson, "ssid", "");  //метка для парсинга
    // jsonWriteStr(paramsHeapJson, "params", "");   //метка для парсинга
}

String getParamsJson() {
    String json;
    serializeJson(*getLocalItemsAsJSON(), json);
    jsonWriteStr_(json, "params", "");
    return json;
}

void syncSettingsFlashJson() {
    writeFile(F("settings.json"), settingsFlashJson);
}

const String getChipId() {
    return String(ESP_getChipId()) + "-" + String(ESP_getFlashChipId());
}

void setChipId() {
    chipId = getChipId();
    SerialPrint("i", "System", "id: " + chipId);
}

const String getUniqueId(const char* name) {
    return String(name) + getMacAddress();
}

const String getWebVersion() {
    String text = readFile("/index.html", 2000);
    text = selectFromMarkerToMarker(text, "title", 1);
    text = selectFromMarkerToMarker(text, " ", 2);
    text.replace("</", "");
    text.replace(".", "");
    return text;
}

uint32_t ESP_getChipId(void) {
#ifdef ESP32
    uint32_t id = 0;
    for (uint32_t i = 0; i < 17; i = i + 8) {
        id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    return id;
#else
    return ESP.getChipId();
#endif
}

uint32_t ESP_getFlashChipId(void) {
#ifdef ESP32
    // Нет аналогичной (без доп.кода) функций в 32
    // надо использовать другой id - варианты есть
    return ESP_getChipId();
#else
    return ESP.getFlashChipId();
#endif
}

const String getMacAddress() {
    uint8_t mac[6];
    char buf[13] = {0};
#if defined(ESP8266)
    WiFi.macAddress(mac);
    sprintf(buf, MACSTR, MAC2STR(mac));
#else
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    sprintf(buf, MACSTR, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
    return String(buf);
}