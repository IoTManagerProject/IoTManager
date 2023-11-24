#include "EspFileSystem.h"
#include "Global.h"

bool fileSystemInit()
{
    if (!FileFS.begin()) {
        SerialPrint(F("E"), F("FS"), F("Init ERROR, may be FS was not flashed"));
        return false;
    }
    SerialPrint(F("i"), F("FS"), F("Init completed"));
    return true;
}

void globalVarsSync()
{
    settingsFlashJson = readFile(F("settings.json"), 4096);
    settingsFlashJson.replace("\r\n", "");

    valuesFlashJson = readFile(F("values.json"), 4096);
    valuesFlashJson.replace("\r\n", "");

    mqttPrefix = jsonReadStr(settingsFlashJson, F("mqttPrefix"));
    jsonWriteStr_(settingsFlashJson, "id", chipId);

    mqttRootDevice = mqttPrefix + "/" + chipId;

    // это не используется - удалить в последствии
    jsonWriteStr_(settingsFlashJson, "root", mqttRootDevice);
}

void syncSettingsFlashJson()
{
    writeFile(F("settings.json"), settingsFlashJson);
}

void syncValuesFlashJson()
{
    writeFile(F("values.json"), valuesFlashJson);
}

const String getChipId()
{
    return String(ESP_getChipId()) + "-" + String(getFlashChipIdNew()); // + "v" + String(FIRMWARE_VERSION);
}

void setChipId()
{
    chipId = getChipId();
    SerialPrint("i", "System", "id: " + chipId);
}

const String getUniqueId(const char* name)
{
    return String(name) + getMacAddress();
}

const String getWebVersion()
{
    String text = readFile("/index.html", 2000);
    text = selectFromMarkerToMarker(text, "title", 1);
    text = selectFromMarkerToMarker(text, " ", 2);
    text.replace("</", "");
    text.replace(".", "");
    return text;
}

uint32_t ESP_getChipId(void)
{
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

// устарела используем новую функцию ниже
#if !defined(esp32s2_4mb) && !defined(esp32c3m_4mb) && !defined(esp32s3_16mb)
//#ifndef esp32s2_4mb
uint32_t ESP_getFlashChipId(void)
{
#ifdef ESP32
    // Нет аналогичной (без доп.кода) функций в 32
    // надо использовать другой id - варианты есть
    return ESP_getChipId();
#else
    return ESP.getFlashChipId();
#endif
}
#endif

// https://github.com/espressif/arduino-esp32/issues/6945#issuecomment-1199900892
// получение flash ch id из проекта esp easy

uint32_t getFlashChipIdNew()
{
    // Cache since size does not change
    static uint32_t flashChipId = 0;

    if (flashChipId == 0) {
#ifdef ESP32
        uint32_t tmp = g_rom_flashchip.device_id;

        for (int i = 0; i < 3; ++i) {
            flashChipId = flashChipId << 8;
            flashChipId |= (tmp & 0xFF);
            tmp = tmp >> 8;
        }

        //    esp_flash_read_id(nullptr, &flashChipId);
#elif defined(ESP8266)
        flashChipId = ESP.getFlashChipId();
#endif // ifdef ESP32
    }
    return flashChipId;
}

const String getMacAddress()
{
    uint8_t mac[6];
    char buf[13] = { 0 };
#if defined(ESP8266)
    WiFi.macAddress(mac);
    sprintf(buf, MACSTR, MAC2STR(mac));
#else
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    sprintf(buf, MACSTR, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
    return String(buf);
}