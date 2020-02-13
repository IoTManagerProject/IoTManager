//===============================================================================================================================
//=========================================Модуль аналогового сенсора============================================================
void analog() {
  String pin = sCmd.next();
  String viget_name = sCmd.next();
  String page_name = sCmd.next();
  String type = sCmd.next();
  String analog_start = sCmd.next();
  String analog_end = sCmd.next();
  String analog_start_out = sCmd.next();
  String analog_end_out = sCmd.next();
  String page_number = sCmd.next();
  jsonWrite(optionJson, "analog_start", analog_start);
  jsonWrite(optionJson, "analog_end", analog_end);
  jsonWrite(optionJson, "analog_start_out", analog_start_out);
  jsonWrite(optionJson, "analog_end_out", analog_end_out);
  choose_viget_and_create(viget_name, page_name, page_number, type, "analog");
  ts.add(ANALOG_, analog_update_int, [&](void*) {
    static int analog_old;
#ifdef ESP32
    //int pin_int = pin.toInt();
    int analog_in;// = analogRead(pin_int);
#endif
#ifdef ESP8266
    int analog_in = analogRead(A0);
#endif
    jsonWrite(configJson, "analog_in", analog_in);
    int analog = map(analog_in,
                     jsonReadtoInt(optionJson, "analog_start") ,
                     jsonReadtoInt(optionJson, "analog_end"),
                     jsonReadtoInt(optionJson, "analog_start_out"),
                     jsonReadtoInt(optionJson, "analog_end_out"));
    jsonWrite(configJson, "analog", analog);
    // if (analog_old != analog) {
    eventGen ("analog", "");
    sendSTATUS("analog", String(analog));
    if (client.connected()) {
      Serial.println("[i] sensor analog send date " + String(analog));
    }
    // }
    analog_old = analog;
  }, nullptr, true);
}

//===================================================================================================================================
//=========================================Модуль измерения уровня в баке============================================================
void level() {
  String viget_name = sCmd.next();
  String page_name = sCmd.next();
  String type = sCmd.next();
  String empty_level = sCmd.next();
  String full_level = sCmd.next();
  String page_number = sCmd.next();
  jsonWrite(optionJson, "empty_level", empty_level);
  jsonWrite(optionJson, "full_level", full_level);
  pinMode(14, OUTPUT);
  pinMode(12, INPUT);
  choose_viget_and_create(viget_name, page_name, page_number, type, "level");
  ts.add(LEVEL, tank_level_shooting_interval, [&](void*) {
    long duration_;
    int distance_cm;
    int level;
    static int level_old; //переменная static сохраняет свое значение между вызовами функции
    static int counter;
    digitalWrite(14, LOW);
    delayMicroseconds(2);
    digitalWrite(14, HIGH);
    delayMicroseconds(10);
    digitalWrite(14, LOW);
    duration_ = pulseIn(12, HIGH, 30000); // 3000 µs = 50cm // 30000 µs = 5 m
    distance_cm = duration_ / 29 / 2;
    distance_cm = medianFilter.filtered(distance_cm);//отсечение промахов медианным фильтром
    counter++;
    if (counter > tank_level_times_to_send) {
      counter = 0;
      jsonWrite(configJson, "level_in", distance_cm);
      level = map(distance_cm,
                  jsonReadtoInt(optionJson, "empty_level"),
                  jsonReadtoInt(optionJson, "full_level"), 0, 100);
      //jsonWrite(configJson, "level", level);
      //if (level_old != level) {
      eventGen ("level", "");
      sendSTATUS("level", String(level));
      if (client.connected()) {
        Serial.println("[i] sensor tank level send date " + String(level));
      }
      //}
      level_old = level;
    }
  }, nullptr, true);
}

//==========================================================================================================================================
//=========================================Модуль температурного сенсора ds18b20============================================================
void dallas() {
  String pin = sCmd.next();
  String viget_name = sCmd.next();
  String page_name = sCmd.next();
  String type = sCmd.next();
  String page_number = sCmd.next();
  oneWire = new OneWire((uint8_t) pin.toInt());
  sensors.setOneWire(oneWire);
  sensors.begin();
  sensors.setResolution(12);
  choose_viget_and_create(viget_name, page_name, page_number, type, "dallas");
  ts.add(DALLAS, temp_update_int, [&](void*) {
    float temp = 0;
    static float temp_old;
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
    jsonWrite(configJson, "dallas", String(temp));
    //if (temp_old != temp) {
    eventGen ("dallas", "");
    sendSTATUS("dallas", String(temp));
    if (client.connected()) {
      Serial.println("[i] sensor dallas send date " + String(temp));
    }
    //}
    temp_old = temp;
  }, nullptr, true);
}


//======================================================================================================================
//=========================================Модуль сенсоров DHT==========================================================
void dhtT() {
  String pin = sCmd.next();
  String viget_name = sCmd.next();
  String page_name = sCmd.next();
  String type = sCmd.next();
  String page_number = sCmd.next();
  dht.setup(pin.toInt());
  choose_viget_and_create(viget_name, page_name, page_number, type, "dhtT");
  ts.add(DHTT, dhtT_update_int, [&](void*) {
    int value = 0;
    static int value_old;
    value = dht.getTemperature();
    jsonWrite(configJson, "dhtT", String(value));
    //if (value_old != value) {
    eventGen ("dhtT", "");
    sendSTATUS("dhtT", String(value));
    if (client.connected()) {
      Serial.println("[i] sensor dhtT send date " + String(value));
    }
    //}
    value_old = value;
  }, nullptr, true);
}


void dhtH() {
  String pin = sCmd.next();
  String viget_name = sCmd.next();
  String page_name = sCmd.next();
  String type = sCmd.next();
  String page_number = sCmd.next();
  dht.setup(pin.toInt());
  choose_viget_and_create(viget_name, page_name, page_number, type, "dhtH");
  ts.add(DHTH, dhtH_update_int, [&](void*) {
    int value = 0;
    static int value_old;
    value = dht.getHumidity();
    jsonWrite(configJson, "dhtH", String(value));
    //if (value_old != value) {
    eventGen ("dhtH", "");
    sendSTATUS("dhtH", String(value));
    if (client.connected()) {
      Serial.println("[i] sensor dhtH send date " + String(value));
    }
    //}
    value_old = value;
  }, nullptr, true);
}



void choose_viget_and_create(String viget_name, String page_name, String page_number, String type, String topik) {

  if (type == "any-data") createViget (viget_name, page_name, page_number, "vigets/viget.anydata.json", topik);
  if (type == "progress-line") createViget (viget_name, page_name, page_number, "vigets/viget.progressL.json", topik);
  if (type == "progress-round") createViget (viget_name, page_name, page_number, "vigets/viget.progressR.json", topik);


}
//======================================================================================================================
//===============================================Логирование============================================================

void logging() {

  static boolean flag = true;

  String sensor_name = sCmd.next();
  String period_min = sCmd.next();
  String maxCount = sCmd.next();
  String viget_name = sCmd.next();
  viget_name.replace("#", " ");
  String page_name = sCmd.next();
  String page_number = sCmd.next();

  if (sensor_name == "analog") jsonWrite(optionJson, "analog_logging_count", maxCount);
  if (sensor_name == "level") jsonWrite(optionJson, "level_logging_count", maxCount);
  if (sensor_name == "dallas") jsonWrite(optionJson, "dallas_logging_count", maxCount);
  if (sensor_name == "ph") jsonWrite(optionJson, "ph_logging_count", maxCount);
/*
  if (sensor_name == "analog") createViget (viget_name, page_name, page_number, "vigets/viget.chart.json", "loganalog", "maxCount", maxCount);
  if (sensor_name == "level") createViget (viget_name, page_name, page_number, "vigets/viget.chart.json", "loglevel", "maxCount", maxCount);
  if (sensor_name == "dallas") createViget (viget_name, page_name, page_number, "vigets/viget.chart.json", "logdallas", "maxCount", maxCount);
  if (sensor_name == "ph") createViget (viget_name, page_name, page_number, "vigets/viget.chart.json", "logph", "maxCount", maxCount);
*/
  if (sensor_name == "analog") {
    flagLoggingAnalog = true;
    ts.remove(ANALOG_LOG);
    ts.add(ANALOG_LOG, period_min.toInt() * 1000 * 60, [&](void*) {
      deleteOldDate("log.analog.txt", jsonReadtoInt(optionJson, "analog_logging_count"), jsonRead(configJson, "analog"), false);
    }, nullptr, true);
  }

  if (sensor_name == "level") {
    flagLoggingLevel = true;
    ts.remove(LEVEL_LOG);
    ts.add(LEVEL_LOG, period_min.toInt() * 1000 * 60, [&](void*) {
      deleteOldDate("log.level.txt", jsonReadtoInt(optionJson, "level_logging_count"), jsonRead(configJson, "level"), false);
    }, nullptr, true);
  }

  if (sensor_name == "dallas") {
    flagLoggingDallas = true;
    ts.remove(DALLAS_LOG);
    ts.add(DALLAS_LOG, period_min.toInt() * 1000 * 60, [&](void*) {
      deleteOldDate("log.dallas.txt", jsonReadtoInt(optionJson, "dallas_logging_count"), jsonRead(configJson, "dallas"), false);
    }, nullptr, true);
  }

  if (sensor_name == "ph") {
    flagLoggingPh = true;
    ts.remove(PH_LOG);
    ts.add(PH_LOG, period_min.toInt() * 1000 * 60, [&](void*) {
      deleteOldDate("log.ph.txt", jsonReadtoInt(optionJson, "ph_logging_count"), jsonRead(configJson, "ph"), false);
    }, nullptr, true);
  }
}

void deleteOldDate(String file, int seted_number_of_lines, String date_to_add, boolean date_time) {

  String current_time;

  if (date_time) {
    current_time = GetDataDigital() + " " + GetTimeWOsec();
    current_time.replace(".", "");
    current_time.replace(":", "");
  } else {
    current_time = "";
  }

  String log_date = readFile(file, 5000);
  getMemoryLoad("[i] after logging procedure");

  //предел количества строк 255

  log_date.replace("\r\n", "\n");
  log_date.replace("\r", "\n");

  int current_number_of_lines = count(log_date, "\n");
  Serial.println("[i] in log file " + file + " " + current_number_of_lines + " lines");


  if (current_number_of_lines > seted_number_of_lines + 1) {
    SPIFFS.remove("/" + file);
    current_number_of_lines = 0;
  }
  if (current_number_of_lines == 0) {
    SPIFFS.remove("/" + file);
    current_number_of_lines = 0;
  }
  if (current_number_of_lines > seted_number_of_lines) {
    log_date = deleteBeforeDelimiter(log_date, "\n");
    log_date += current_time + " " +  date_to_add + "\n";
    writeFile(file, log_date);

  } else {
    if (date_time) {
      addFile(file, current_time + " " +  date_to_add);
    } else {
      addFile(file, date_to_add);
    }
  }
}
