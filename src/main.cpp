#include "Global.h"

void saveConfig() {
    writeFile("config.json", configSetupJson);
}

String getURL(const String& urls) {
    String res = "";
    HTTPClient http;
    http.begin(urls);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        res = http.getString();
    } else {
        res = "error";
    }
    http.end();
    return res;
}

void safeDataToFile(String data, String Folder) {
    //String fileName = GetDate();
    String fileName;
    fileName.toLowerCase();
    fileName = deleteBeforeDelimiter(fileName, " ");
    fileName.replace(" ", ".");
    fileName.replace("..", ".");
    fileName = Folder + "/" + fileName + ".txt";
    // addFile(fileName, GetTime() + "/" + data);
    Serial.println(fileName);
    jsonWriteStr(configLiveJson, "test", fileName);
}

void sendCONFIG(String topik, String widgetConfig, String key, String date) {
    yield();
    topik = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipId + "/" + topik + "/status";
    String outer = "{\"widgetConfig\":";
    String inner = "{\"";
    inner = inner + key;
    inner = inner + "\":\"";
    inner = inner + date;
    inner = inner + "\"";
    inner = inner + "}}";
    String t = outer + inner;
    //Serial.println(t);
    //client_mqtt.publish(MQTT::Publish(topik, t).set_qos(1));
    yield();
}

void setChipId() {
    chipId = getChipId();
    Serial.println(chipId);
}

#ifdef ESP8266
#ifdef LED_PIN
void led_blink(String satus) {
    pinMode(LED_PIN, OUTPUT);
    if (satus == "off") {
        noTone(LED_PIN);
        digitalWrite(LED_PIN, HIGH);
    }
    if (satus == "on") {
        noTone(LED_PIN);
        digitalWrite(LED_PIN, LOW);
    }
    if (satus == "slow") tone(LED_PIN, 1);
    if (satus == "fast") tone(LED_PIN, 20);
}
#endif
#endif
