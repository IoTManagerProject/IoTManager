#include "Global.h"

void saveConfig() {
    writeFile("config.json", configSetup);
}

//--------------------Посчитать -----------------------------------------------------------------------------------
int count(String str, String found) {
    if (str.indexOf(found) == -1) return 0;  // если строки поиск нет сразу выход
    str += found;                            // добавим для корректного поиска
    uint8_t i = 0;                           // Индекс перебора
    while (str.length() != 0) {
        str = deleteBeforeDelimiter(str, found);  // отбросим проверенный блок до разделителя
        i++;                                      // увеличим индекс
    }
    return i;  // Достигли пустой строки и ничего не нашли
}

boolean isDigitStr(String str) {
    for (int i = 0; i < str.length(); i++) {
        if (!isDigit(str.charAt(i))) {
            return false;
        }
    }
    return str.length();
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
    jsonWriteStr(configJson, "test", fileName);
}

// ------------- Чтение файла в строку -------------------------------------------------------------------------------
String readFile(String fileName, size_t len) {
    File configFile = SPIFFS.open("/" + fileName, "r");
    if (!configFile) {
        return "Failed";
    }
    size_t size = configFile.size();
    if (size > len) {
        configFile.close();
        return "Large";
    }
    String temp = configFile.readString();
    configFile.close();
    return temp;
}
// ------------- Размер файла ----------------------------------------------------------------------------------------
String sizeFile(String fileName) {
    File configFile = SPIFFS.open("/" + fileName, "r");
    if (!configFile) {
        return "Failed";
    }
    size_t size = configFile.size();
    configFile.close();
    return String(size);
}
// ------------- Запись строки в файл ---------------------------------------------------------------------------------
String writeFile(String fileName, String strings) {
    File configFile = SPIFFS.open("/" + fileName, "w");
    if (!configFile) {
        return "Failed to open config file";
    }
    configFile.print(strings);
    //strings.printTo(configFile);
    configFile.close();
    return "Write sucsses";
}
// ------------- Добовление строки в файл ------------------------------------------------------------------------------
String addFile(String fileName, String strings) {
    File configFile = SPIFFS.open("/" + fileName, "a");
    if (!configFile) {
        return "Failed to open config file";
    }
    configFile.println(strings);
    configFile.close();
    return "Write sucsses";
}
// ------------- Чтение строки из файла ---------------------------------------------------------------------------------
//возвращает стоку из файла в которой есть искомое слово found

String readFileString(const String& filename, const String& str_to_found) {
    String res = "failed";
    auto file = SPIFFS.open("/" + filename, "r");
    if (file && file.find(str_to_found.c_str())) {
        res = file.readStringUntil('\n');
    }
    file.close();
    return res;
}

void sendCONFIG(String topik, String widgetConfig, String key, String date) {
    yield();
    topik = jsonReadStr(configSetup, "mqttPrefix") + "/" + chipID + "/" + topik + "/status";
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

void led_blink(String satus) {
#ifdef ESP8266
#ifdef blink_pin
    pinMode(blink_pin, OUTPUT);
    if (satus == "off") {
        noTone(blink_pin);
        digitalWrite(blink_pin, HIGH);
    }
    if (satus == "on") {
        noTone(blink_pin);
        digitalWrite(blink_pin, LOW);
    }
    if (satus == "slow") tone(blink_pin, 1);
    if (satus == "fast") tone(blink_pin, 20);
#endif
#endif
}

void getMemoryLoad(String text) {
#ifdef ESP8266
    int all_memory = 52864;
#endif
#ifdef ESP32
    int all_memory = 362868;
#endif
    int memory_remain = ESP.getFreeHeap();
    int memory_used = all_memory - memory_remain;
    int memory_load = (memory_used * 100) / all_memory;
    if (memory_load > 65) {
        Serial.println("Memory low!");
    }
    Serial.print(text + " memory used:");
    Serial.print(String(memory_load) + "%; ");
    Serial.print("memory remain: ");
    Serial.println(String(memory_remain) + " k bytes");
}
//esp32 full memory = 362868 k bytes
//esp8266 full memory = 52864 k bytes

//===================================================================
/*
  void web_print (String text) {
  if (WiFi.status() == WL_CONNECTED) {
    jsonWriteStr(json, "test1",  jsonReadStr(json, "test2"));
    jsonWriteStr(json, "test2",  jsonReadStr(json, "test3"));
    jsonWriteStr(json, "test3",  jsonReadStr(json, "test4"));
    jsonWriteStr(json, "test4",  jsonReadStr(json, "test5"));
    jsonWriteStr(json, "test5",  jsonReadStr(json, "test6"));

    jsonWriteStr(json, "test6", GetTime() + " " + text);

    ws.textAll(json);
  }
  }
*/
//===================================================================
/*
  "socket": [
      "ws://{{ip}}/ws"
        ],
*/
//===================================================================
/*
  {
      "type": "h4",
      "title": "('{{build2}}'=='{{firmware_version}}'?'NEW':'OLD')"
    },
*/
//===================================================================
/*
    {
      "type": "button",
      "title": "Конфигурация устройства",
      "socket": "test2",
      "class": "btn btn-block btn-primary"
    },
  {
      "type": "hr"
    },
               {
      "type": "h6",
      "title": "{{test1}}"
    },
                {
      "type": "h6",
      "title": "{{test2}}"
    },
               {
      "type": "h6",
      "title": "{{test3}}"
    },
                {
      "type": "h6",
      "title": "{{test4}}"
    },
                {
      "type": "h6",
      "title": "{{test5}}"
    },
                {
      "type": "h6",
      "title": "{{test6}}"
    },
                {
      "type": "hr"
    },
*/
//===================================================================

/*
  String getResetReason(uint8_t core) {
  int reason = rtc_get_reset_reason(core);
  switch (reason) {
    case 1  : return "Power on"; break;                                  //Vbat power on reset
    case 3  : return "Software reset digital core"; break;               //Software reset digital core
    case 4  : return "Legacy watch dog reset digital core"; break;       //Legacy watch dog reset digital core
    case 5  : return "Deep Sleep reset digital core"; break;             //Deep Sleep reset digital core
    case 6  : return "Reset by SLC module, reset digital core"; break;   //Reset by SLC module, reset digital core
    case 7  : return "Timer Group0 Watch dog reset digital core"; break; //Timer Group0 Watch dog reset digital core
    case 8  : return "Timer Group1 Watch dog reset digital core"; break; //Timer Group1 Watch dog reset digital core
    case 9  : return "RTC Watch dog Reset digital core"; break;          //
    case 10 : return "Instrusion tested to reset CPU"; break;
    case 11 : return "Time Group reset CPU"; break;
    case 12 : return "Software reset CPU"; break;
    case 13 : return "RTC Watch dog Reset CPU"; break;
    case 14 : return "for APP CPU, reseted by PRO CPU"; break;
    case 15 : return "Reset when the vdd voltage is not stable"; break;
    case 16 : return "RTC Watch dog reset digital core and rtc module"; break;
    default : return "NO_MEAN";
  }
  }


  String EspClass::getResetReason(void) {
  char buff[32];
  if (resetInfo.reason == REASON_DEFAULT_RST) { // normal startup by power on
    strcpy_P(buff, PSTR("Power on"));
  } else if (resetInfo.reason == REASON_WDT_RST) { // hardware watch dog reset
    strcpy_P(buff, PSTR("Hardware Watchdog"));
  } else if (resetInfo.reason == REASON_EXCEPTION_RST) { // exception reset, GPIO status won’t change
    strcpy_P(buff, PSTR("Exception"));
  } else if (resetInfo.reason == REASON_SOFT_WDT_RST) { // software watch dog reset, GPIO status won’t change
    strcpy_P(buff, PSTR("Software Watchdog"));
  } else if (resetInfo.reason == REASON_SOFT_RESTART) { // software restart ,system_restart , GPIO status won’t change
    strcpy_P(buff, PSTR("Software/System restart"));
  } else if (resetInfo.reason == REASON_DEEP_SLEEP_AWAKE) { // wake up from deep-sleep
    strcpy_P(buff, PSTR("Deep-Sleep Wake"));
  } else if (resetInfo.reason == REASON_EXT_SYS_RST) { // external system reset
    strcpy_P(buff, PSTR("External System"));
  } else {
    strcpy_P(buff, PSTR("Unknown"));
  }
  return String(buff);
  }
*/
