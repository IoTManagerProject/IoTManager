//=============================================JSON===========================================================
// ------------- Чтение значения json
String jsonRead(String &json, String name) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  return root[name].as<String>();
}

// ------------- Чтение значения json
int jsonReadtoInt(String &json, String name) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  return root[name];
}
/*
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
  return json;

   DynamicJsonDocument  jsonBuffer(1024);
  deserializeJson(jsonBuffer,json);
  jsonBuffer[name] = volume;
  json = "";
  serializeJson(jsonBuffer,json);
  return json;
*/
// ------------- Запись значения json String
String jsonWriteStr(String &json, String name, String volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
  return json;
}

// ------------- Запись значения json int
String jsonWriteInt(String &json, String name, int volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
  return json;
}

// ------------- Запись значения json float
String jsonWriteFloat(String &json, String name, float volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
  return json;
}
/*
  String jsonWriteArray(String &json, String value1, String value2, String value3) {

  const int capacity = JSON_ARRAY_SIZE(1) + 3 * JSON_OBJECT_SIZE(3);

  StaticJsonBuffer<capacity> jb;
  JsonArray& arr = jb.createArray();
  JsonObject& obj1 = jb.createObject();

  obj1[value1] = 1;
  obj1[value2] = 2;
  obj1[value3] = 3;

  arr.add(obj1);

  arr.printTo(json);

  return json;
  }
*/
//=============================================CONFIG===========================================================
void saveConfig () {
  writeFile("config.json", configSetup);
}
//=============================================ТЕКСТ============================================================
// --------Выделяем строку от конца строки до маркера-----------------------------------------------------------
String selectToMarkerLast (String str, String found) {
  int p = str.lastIndexOf(found);
  return str.substring(p + found.length());
}

// --------Выделяем строку до маркера---------------------------------------------------------------------------
String selectToMarker (String str, String found) {
  int p = str.indexOf(found);
  return str.substring(0, p);
}
// --------Удаляем все после символа разделителя (вместе с символом разделителя)--------------------------------
String deleteAfterDelimiter (String str, String found) {
  int p = str.indexOf(found);
  return str.substring(0, p);
}

//---------Удаляем все до символа разделителя (вместе с символом разделителя)-----------------------------------
String deleteBeforeDelimiter(String str, String found) {
  int p = str.indexOf(found) + found.length();
  return str.substring(p);
}
//----------------------Удаляем все до символа разделителя (без символа разделителя)----------------------------
String deleteBeforeDelimiterTo(String str, String found) {
  int p = str.indexOf(found);
  return str.substring(p);
}

// -------------------Выделяем строку от конца строки до маркера ------------------------------------------------
String deleteToMarkerLast (String str, String found) {
  int p = str.lastIndexOf(found);
  return str.substring(0, p);
}
String selectToMarkerPlus (String str, String found, int plus) {
  int p = str.indexOf(found);
  return str.substring(0, p + plus);
}
//--------------------Выделяем строку от маркера до маркера
String selectFromMarkerToMarker(String str, String found, int number) {
  if (str.indexOf(found) == -1) return "not found"; // если строки поиск нет сразу выход
  str += found; // добавим для корректного поиска
  uint8_t i = 0; // Индекс перебора
  do {
    if (i == number) return selectToMarker(str, found); // если индекс совпал с позицией законцим вернем резултат
    str = deleteBeforeDelimiter(str, found);  // отбросим проверенный блок до разделителя
    i++; // увеличим индекс
  } while (str.length() != 0); // повторим пока строка не пустая
  return "not found"; // Достигли пустой строки и ничего не нашли
}
//--------------------Посчитать
int count(String str, String found) {
  if (str.indexOf(found) == -1) return 0; // если строки поиск нет сразу выход
  str += found; // добавим для корректного поиска
  uint8_t i = 0; // Индекс перебора
  while (str.length() != 0) {
    str = deleteBeforeDelimiter(str, found);  // отбросим проверенный блок до разделителя
    i++; // увеличим индекс
  }
  return i; // Достигли пустой строки и ничего не нашли
}

boolean isDigitStr (String str) {
  if (str.length() == 1) {
    return Digit (str);
  }
  if (str.length() > 1) {
    for (int i = 0; i < str.length(); i++) {
      if (!Digit (String(str.charAt(i)))) return false;
    }
    return true;
  }
}

boolean Digit (String str) {
  if (str == "0" || str == "1" || str == "2" || str == "3" || str == "4" || str == "5" || str == "6" || str == "7" || str == "8" || str == "9") {
    return true;
  } else {
    return false;
  }
}
//============================================URL===================================================================
// ----------------------Запрос на удаленный URL
String getURL(String urls) {
  String answer = "";
  HTTPClient http;
  http.begin(urls); //HTTP
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    answer = http.getString();
  }
  else {
    answer = "error";
  }
  http.end();
  return answer;
}

//===========================================FILES===================================================================

String safeDataToFile(String data, String Folder)
{
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
// ------------- Чтение файла в строку
String readFile(String fileName, size_t len ) {
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

String sizeFile(String fileName) {
  File configFile = SPIFFS.open("/" + fileName, "r");
  if (!configFile) {
    return "Failed";
  }
  size_t size = configFile.size();
  configFile.close();
  return String(size);
}

// ------------- Запись строки в файл
String writeFile(String fileName, String strings ) {
  File configFile = SPIFFS.open("/" + fileName, "w");
  if (!configFile) {
    return "Failed to open config file";
  }
  configFile.print(strings);
  //strings.printTo(configFile);
  configFile.close();
  return "Write sucsses";
}

// ------------- Добовление строки в файл
String addFile(String fileName, String strings ) {
  File configFile = SPIFFS.open("/" + fileName, "a");
  if (!configFile) {
    return "Failed to open config file";
  }
  configFile.println(strings);
  configFile.close();
  return "Write sucsses";
}

// ------------- Чтение строки из файла
//возвращает стоку из файла в которой есть искомое слово found
String readFileString(String fileName, String found) {
  File configFile = SPIFFS.open("/" + fileName, "r");
  if (!configFile) {
    return "Failed";
  }
  if (configFile.find(found.c_str())) {
    return configFile.readStringUntil('\n');  //'\r'
  }
  configFile.close();
}

// Запись данных в файл с частотой 1 секунда и более. Максимальное количество данных в суточном файле 1440 значений
void safeDataToFile(int inter, String par, uint16_t data) {
  yield();
  // Формируем зоголовок (префикс) Интервал, Параметр, размер_параметра
  uint16_t dataSize = sizeof(data);
  String prifexFile;
  prifexFile += inter;
  prifexFile += "," + par;
  prifexFile += ",";
  prifexFile += dataSize;
  prifexFile += ":";
  uint16_t prifexLen = prifexFile.length(); //Размер префикса

  // Сделаем имя файла
  String fileName = GetDate();
  fileName = deleteBeforeDelimiter(fileName, " "); // удалим день недели
  fileName.replace(" ", ".");
  fileName.replace("..", "."); // Заменяем пробелы точками
  fileName = par + "/" + fileName + ".txt"; // Имя файла параметр в виде директории и дата
  fileName.toLowerCase(); //fileName = "san aug 31 2018"; Имя файла строчными буквами
  Serial.println(fileName);
  File configFile = SPIFFS.open("/" + fileName, "a"); // Открываем файл на добавление
  size_t size = configFile.size();
  yield();
  if (size == 0) {
    configFile.print(prifexFile);
  }
  size = configFile.size();
  // Получим время и определим позицию в файле
  String time = GetTime();
  //time = "00:15:00";
  int timeM = timeToMin(time); // Здесь количество минут с начала суток
  timeM = timeM / inter;
  int poz = timeM * dataSize + prifexLen + 1; // позиция в которую нужно записать.
  int endF = (size - prifexLen) * dataSize + prifexLen + 1; // позиция конца файла
  if (poz >= endF) { // если файл имел пропуски в записи данных
    int i = (poz - endF) / dataSize;
    for (int j = 0; j < i; j++) { // Заполним недостающие данные
      for (int d = 0; d < dataSize; d++) {
        yield();
        configFile.write(0);    // нулями
      }
    }
  }
  yield();
  configFile.write(data >> 8); // добавим текущие
  configFile.write(data);      // данные
  configFile.close();
}

//=======================================УПРАВЛЕНИЕ ВИДЖЕТАМИ MQTT======================================================================


void sendCONFIG(String topik, String widgetConfig, String key, String date) {
  yield();
  topik = jsonRead(configSetup, "mqttPrefix") + "/" + chipID + "/" + topik + "/status";
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

uint8_t hexStringToUint8(String hex) {
  uint8_t   tmp = strtol(hex.c_str(), NULL, 0);
  if (tmp >= 0x00 && tmp <= 0xFF) {
    return tmp;
  }
}

uint16_t hexStringToUint16(String hex) {
  uint16_t   tmp = strtol(hex.c_str(), NULL, 0);
  if (tmp >= 0x0000 && tmp <= 0xFFFF) {
    return tmp;
  }
}
//=============================================================================================================

void led_blink(String satus) {
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
  if (memory_load > 65) Serial.print("Attention!!! too match memory used!!!");
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
    jsonWriteStr(json, "test1",  jsonRead(json, "test2"));
    jsonWriteStr(json, "test2",  jsonRead(json, "test3"));
    jsonWriteStr(json, "test3",  jsonRead(json, "test4"));
    jsonWriteStr(json, "test4",  jsonRead(json, "test5"));
    jsonWriteStr(json, "test5",  jsonRead(json, "test6"));

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
