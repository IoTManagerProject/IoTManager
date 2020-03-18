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

// ------------- Запись значения json String
String jsonWrite(String &json, String name, String volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
  return json;
}

// ------------- Запись значения json int
String jsonWrite(String &json, String name, int volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
  return json;
}

// ------------- Запись значения json float
String jsonWrite(String &json, String name, float volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
  return json;
}
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
  jsonWrite(configJson, "test", fileName);
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
String readFileString(String fileName, String found)
{
  File configFile = SPIFFS.open("/" + fileName, "r");
  if (!configFile) {
    return "Failed";
  }
  if (configFile.find(found.c_str())) {
    return configFile.readStringUntil('\r\n');  //'\r'
  }
  //return "|-|-|";
  configFile.close();
}

//=======================================УПРАВЛЕНИЕ ВИДЖЕТАМИ MQTT======================================================================


void sendCONFIG(String topik, String widgetConfig, String key, String date) {
  yield();
  topik = prefix + "/" + chipID + "/" + topik + "/status";
  String outer = "{\"widgetConfig\":";
  String inner = "{\"";
  inner = inner + key;
  inner = inner + "\":\"";
  inner = inner + date;
  inner = inner + "\"";
  inner = inner + "}}";
  String t = outer + inner;
  //Serial.println(t);
  //client.publish(MQTT::Publish(topik, t).set_qos(1));
  yield();
}
//=============================================================================================================
#ifdef led_status
void led_blink(int pin, int fq, String blink_satus) {

  pinMode(pin, OUTPUT);

  if (blink_satus == "on") tone(pin, fq);
  if (blink_satus == "off") {

    noTone(pin);
    digitalWrite(pin, HIGH);

  }
}
#endif

void getMemoryLoad(String text) {
#ifdef ESP8266
  int all_memory = 53312;
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
  Serial.print("memory remain:");
  Serial.println(String(memory_remain) + " k bytes");

}
//esp32 full memory = 362868 k bytes
//esp8266 full memory = 53312 k bytes


//===================================================================
/*
void web_print (String text) {
  if (WiFi.status() == WL_CONNECTED) {
    jsonWrite(json, "test1",  jsonRead(json, "test2"));
    jsonWrite(json, "test2",  jsonRead(json, "test3"));
    jsonWrite(json, "test3",  jsonRead(json, "test4"));
    jsonWrite(json, "test4",  jsonRead(json, "test5"));
    jsonWrite(json, "test5",  jsonRead(json, "test6"));

    jsonWrite(json, "test6", GetTime() + " " + text);

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
