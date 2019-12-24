#ifdef ESP8266

#include <time.h>
void Time_Init() {
  server.on("/Time", HTTP_GET, [](AsyncWebServerRequest * request) {
    //handle_Time();
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });
  server.on("/timeZone", HTTP_GET, [](AsyncWebServerRequest * request) {
    //handle_time_zone();
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });

  timeSynch(jsonReadtoInt(configSetup, "timezone"));
}


void timeSynch(int zone) {
  if (WiFi.status() == WL_CONNECTED) {
    // Настройка соединения с NTP сервером
    configTime(zone * 3600, 0, "pool.ntp.org", "ru.pool.ntp.org");
    // int i = 0;
    // Serial.println("\nWaiting for time");
    // while (!time(nullptr) && i < 10) {
    //   Serial.print(".");
    //   i++;
    //   delay(1000);
    // }
    Serial.println("");
    Serial.println("ITime Ready!");
    delay(1000);
    Serial.println(GetTime());
    Serial.println(GetDate());
  }
}
/*// Установка параметров времянной зоны по запросу вида http://192.168.0.101/timeZone?timeZone=3
void handle_time_zone() {
  if (request->hasArg("timeZone")) {
    jsonWrite(configSetup, "timeZone", request->getParam("timeZone")->value());
  }
  saveConfig();
  //request->send(200, "text/text", "OK");
}

void handle_Time() {
  timeSynch(jsonReadtoInt(configSetup, "timezone"));
  //request->send(200, "text/text", "OK");
}
*/
#endif

// Получение текущего времени
String GetTime() {
  time_t now = time(nullptr); // получаем время с помощью библиотеки time.h
  String Time = ""; // Строка для результатов времени
  Time += ctime(&now); // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
  int i = Time.indexOf(":"); //Ишем позицию первого символа :
  Time = Time.substring(i - 2, i + 6); // Выделяем из строки 2 символа перед символом : и 6 символов после
  return Time; // Возврашаем полученное время
}

String GetTimeWOsec() {
  time_t now = time(nullptr); // получаем время с помощью библиотеки time.h
  String Time = ""; // Строка для результатов времени
  Time += ctime(&now); // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
  int i = Time.indexOf(":"); //Ишем позицию первого символа :
  Time = Time.substring(i - 2, i + 3); // Выделяем из строки 2 символа перед символом : и 6 символов после
  return Time; // Возврашаем полученное время
}

// Получение даты
String GetDate() {
  time_t now = time(nullptr); // получаем время с помощью библиотеки time.h
  String Data = ""; // Строка для результатов времени
  Data += ctime(&now); // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
  Data.replace("\n", "");
  uint8_t i = Data.lastIndexOf(" "); //Ишем позицию последнего символа пробел
  String Time = Data.substring(i - 8, i + 1); // Выделяем время и пробел
  Data.replace(Time, ""); // Удаляем из строки 8 символов времени и пробел
  return Data; // Возврашаем полученную дату
}

String GetDataDigital() {
  String date = GetDate();

  date = deleteBeforeDelimiter(date, " ");

  date.replace("Jan", "01");
  date.replace("Feb", "02");
  date.replace("Mar", "03");
  date.replace("Apr", "04");
  date.replace("May", "05");
  date.replace("Jun", "06");
  date.replace("Jul", "07");
  date.replace("Aug", "08");
  date.replace("Sep", "09");
  date.replace("Oct", "10");
  date.replace("Nov", "11");
  date.replace("Dec", "12");

  String month = date.substring(0, 2);
  String day = date.substring(3, 5);
  String year = date.substring(8, 10);

  String out = day;
  out += ".";
  out += month;
  out += ".";
  out += year;

  return out;
}
