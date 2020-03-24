//===============================================================================================================================
//=========================================Модуль аналогового сенсора============================================================
void analog() {
  String pin = sCmd.next();
  String widget_name = sCmd.next();
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
  choose_widget_and_create(widget_name, page_name, page_number, type, "analog");
  ts.add(ANALOG_, analog_update_int, [&](void*) {
    static int analog_old;
#ifdef ESP32
    //int pin_int = pin.toInt();
    int analog_in = analogRead(34);
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
      Serial.println("[i] sensor 'analog' send date " + String(analog));
    }
    // }
    analog_old = analog;
  }, nullptr, true);
}

//===================================================================================================================================
//=========================================Модуль измерения уровня в баке============================================================
void level() {
  String widget_name = sCmd.next();
  String page_name = sCmd.next();
  String type = sCmd.next();
  String empty_level = sCmd.next();
  String full_level = sCmd.next();
  String page_number = sCmd.next();
  jsonWrite(optionJson, "empty_level", empty_level);
  jsonWrite(optionJson, "full_level", full_level);
  pinMode(14, OUTPUT);
  pinMode(12, INPUT);
  choose_widget_and_create(widget_name, page_name, page_number, type, "level");
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
      jsonWrite(configJson, "level", level);
      //if (level_old != level) {
      eventGen ("level", "");
      sendSTATUS("level", String(level));
      if (client.connected()) {
        Serial.println("[i] sensor tank 'level' send date " + String(level));
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
  String widget_name = sCmd.next();
  String page_name = sCmd.next();
  String type = sCmd.next();
  String page_number = sCmd.next();
  oneWire = new OneWire((uint8_t) pin.toInt());
  sensors.setOneWire(oneWire);
  sensors.begin();
  sensors.setResolution(12);
  choose_widget_and_create(widget_name, page_name, page_number, type, "dallas");
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
      Serial.println("[i] sensor 'dallas' send date " + String(temp));
    }
    //}
    temp_old = temp;
  }, nullptr, true);
}


//======================================================================================================================
//=========================================Модуль сенсоров DHT==========================================================
void dhtT() {
  String sensor_type = sCmd.next();
  String pin = sCmd.next();
  String widget_name = sCmd.next();
  String page_name = sCmd.next();
  String type = sCmd.next();
  String page_number = sCmd.next();
  if (sensor_type == "DHT11") {
    dht.setup(pin.toInt(), DHTesp::DHT11);
  }
  if (sensor_type == "DHT22") {
    dht.setup(pin.toInt(), DHTesp::DHT22);
  }
  choose_widget_and_create(widget_name, page_name, page_number, type, "dhtT");
  ts.add(DHTT, dhtT_update_int, [&](void*) { //dht.getMinimumSamplingPeriod()
    float value = 0;
    static float value_old;
    static int counter;
    if (dht.getStatus() != 0 && counter < 5) {
      sendSTATUS("dhtT", String(dht.getStatusString()));
      counter++;
    } else {
      counter = 0;
      value = dht.getTemperature();
      jsonWrite(configJson, "dhtT", String(value));
      //if (value_old != value) {
      eventGen ("dhtT", "");
      sendSTATUS("dhtT", String(value));
      if (client.connected()) {
        Serial.println("[i] sensor 'dhtT' send date " + String(value));
      }
      //}
      value_old = value;
    }
  }, nullptr, true);
}


void dhtH() {
  String sensor_type = sCmd.next();
  String pin = sCmd.next();
  String widget_name = sCmd.next();
  String page_name = sCmd.next();
  String type = sCmd.next();
  String page_number = sCmd.next();
  if (sensor_type == "DHT11") {
    dht.setup(pin.toInt(), DHTesp::DHT11);
  }
  if (sensor_type == "DHT22") {
    dht.setup(pin.toInt(), DHTesp::DHT22);
  }
  choose_widget_and_create(widget_name, page_name, page_number, type, "dhtH");
  ts.add(DHTH, dhtH_update_int , [&](void*) {  //dht.getMinimumSamplingPeriod()
    int value = 0;
    static int value_old;
    static int counter;
    if (dht.getStatus() != 0 && counter < 5) {
      sendSTATUS("dhtH", String(dht.getStatusString()));
      counter++;
    } else {
      counter = 0;
      value = dht.getHumidity();
      jsonWrite(configJson, "dhtH", String(value));
      //if (value_old != value) {
      eventGen ("dhtH", "");
      sendSTATUS("dhtH", String(value));
      if (client.connected()) {
        Serial.println("[i] sensor 'dhtH' send date " + String(value));
      }
      //}
      value_old = value;
    }
  }, nullptr, true);
}

void dhtPerception() {
  String widget_name = sCmd.next();
  String page_name = sCmd.next();
  String page_number = sCmd.next();
  choose_widget_and_create(widget_name, page_name, page_number, "any-data", "dhtPerception");
  ts.add(DHTP, dht_calculation_update_int, [&](void*) {
    byte value;
    if (dht.getStatus() != 0) {
      sendSTATUS("dhtPerception", String(dht.getStatusString()));
    } else {
      value = dht.computePerception(jsonRead(configJson, "dhtT").toFloat(), jsonRead(configJson, "dhtH").toFloat(), false);
      String final_line = perception(value);
      jsonWrite(configJson, "dhtPerception", final_line);
      eventGen ("dhtPerception", "");
      sendSTATUS("dhtPerception", final_line);
      if (client.connected()) {
        Serial.println("[i] sensor 'dhtPerception' send date " + final_line);
      }
    }
  }, nullptr, true);
}

String perception(byte value) {
  if (value == 0) return "Сухой воздух";
  if (value == 1) return "Комфортно";
  if (value == 2) return "Уютно";
  if (value == 3) return "Хорошо";
  if (value == 4) return "Неудобно";
  if (value == 5) return "Довольно неудобно";
  if (value == 6) return "Очень неудобно";
  if (value == 7) return "Сильно неудобно, полный звиздец";
}

void dhtComfort() {
  String widget_name = sCmd.next();
  String page_name = sCmd.next();
  String page_number = sCmd.next();
  choose_widget_and_create(widget_name, page_name, page_number, "any-data", "dhtComfort");
  ts.add(DHTC, dht_calculation_update_int, [&](void*) {
    float value;
    ComfortState cf;
    if (dht.getStatus() != 0) {
      sendSTATUS("dhtComfort", String(dht.getStatusString()));
    } else {
      value = dht.getComfortRatio(cf, jsonRead(configJson, "dhtT").toFloat(), jsonRead(configJson, "dhtH").toFloat(), false);
      String comfortStatus;
      switch (cf) {
        case Comfort_OK:
          comfortStatus = "Отлично";
          break;
        case Comfort_TooHot:
          comfortStatus = "Очень жарко";
          break;
        case Comfort_TooCold:
          comfortStatus = "Очень холодно";
          break;
        case Comfort_TooDry:
          comfortStatus = "Очень сухо";
          break;
        case Comfort_TooHumid:
          comfortStatus = "Очень влажно";
          break;
        case Comfort_HotAndHumid:
          comfortStatus = "Жарко и влажно";
          break;
        case Comfort_HotAndDry:
          comfortStatus = "Жарко и сухо";
          break;
        case Comfort_ColdAndHumid:
          comfortStatus = "Холодно и влажно";
          break;
        case Comfort_ColdAndDry:
          comfortStatus = "Холодно и сухо";
          break;
        default:
          comfortStatus = "Неизвестно";
          break;
      };
      String final_line = comfortStatus;
      jsonWrite(configJson, "dhtComfort", final_line);
      eventGen ("dhtComfort", "");
      sendSTATUS("dhtComfort", final_line);
      if (client.connected()) {
        Serial.println("[i] sensor 'dhtComfort' send date " + final_line);
      }
    }
  }, nullptr, true);
}

void dhtDewpoint() {
  String widget_name = sCmd.next();
  String page_name = sCmd.next();
  String page_number = sCmd.next();
  choose_widget_and_create(widget_name, page_name, page_number, "any-data", "dhtDewpoint");
  ts.add(DHTD, dht_calculation_update_int, [&](void*) {
    float value;
    if (dht.getStatus() != 0) {
      sendSTATUS("dhtDewpoint", String(dht.getStatusString()));
    } else {
      value = dht.computeDewPoint(jsonRead(configJson, "dhtT").toFloat(), jsonRead(configJson, "dhtH").toFloat(), false);
      jsonWrite(configJson, "dhtDewpoint", value);
      eventGen ("dhtDewpoint", "");
      sendSTATUS("dhtDewpoint", String(value));
      if (client.connected()) {
        Serial.println("[i] sensor 'dhtDewpoint' send date " + String(value));
      }
    }
  }, nullptr, true);
}


void choose_widget_and_create(String widget_name, String page_name, String page_number, String type, String topik) {

  if (type == "any-data") createWidget (widget_name, page_name, page_number, "widgets/widget.anyData.json", topik);
  if (type == "progress-line") createWidget (widget_name, page_name, page_number, "widgets/widget.progLine.json", topik);
  if (type == "progress-round") createWidget (widget_name, page_name, page_number, "widgets/widget.progRound.json", topik);
  if (type == "fill-gauge") createWidget (widget_name, page_name, page_number, "widgets/widget.fillGauge.json", topik);

}
//======================================================================================================================
//===============================================Логирование============================================================

void logging() {

  static boolean flag = true;

  String sensor_name = sCmd.next();
  String period_min = sCmd.next();
  String maxCount = sCmd.next();
  String widget_name = sCmd.next();
  widget_name.replace("#", " ");
  String page_name = sCmd.next();
  String page_number = sCmd.next();

  if (sensor_name == "analog") jsonWrite(optionJson, "analog_logging_count", maxCount);
  if (sensor_name == "level") jsonWrite(optionJson, "level_logging_count", maxCount);
  if (sensor_name == "dallas") jsonWrite(optionJson, "dallas_logging_count", maxCount);

  if (sensor_name == "analog") createChart (widget_name, page_name, page_number, "widgets/widget.chart.json", "loganalog", maxCount);
  if (sensor_name == "level") createChart (widget_name, page_name, page_number, "widgets/widget.chart.json", "loglevel", maxCount);
  if (sensor_name == "dallas") createChart (widget_name, page_name, page_number, "widgets/widget.chart.json", "logdallas", maxCount);

  if (sensor_name == "analog") {
    flagLoggingAnalog = true;
    ts.remove(ANALOG_LOG);
    ts.add(ANALOG_LOG, period_min.toInt() * 1000 * 60, [&](void*) {
      deleteOldDate("log.analog.txt", jsonReadtoInt(optionJson, "analog_logging_count"), jsonRead(configJson, "analog"));
    }, nullptr, true);
  }

  if (sensor_name == "level") {
    flagLoggingLevel = true;
    ts.remove(LEVEL_LOG);
    ts.add(LEVEL_LOG, period_min.toInt() * 1000 * 60, [&](void*) {
      deleteOldDate("log.level.txt", jsonReadtoInt(optionJson, "level_logging_count"), jsonRead(configJson, "level"));
    }, nullptr, true);
  }

  if (sensor_name == "dallas") {
    flagLoggingDallas = true;
    ts.remove(DALLAS_LOG);
    ts.add(DALLAS_LOG, period_min.toInt() * 1000 * 60, [&](void*) {
      deleteOldDate("log.dallas.txt", jsonReadtoInt(optionJson, "dallas_logging_count"), jsonRead(configJson, "dallas"));
    }, nullptr, true);
  }
}

void deleteOldDate(String file, int seted_number_of_lines, String date_to_add) {

  String log_date = readFile(file, 5000);
  
  //getMemoryLoad("[i] after logging procedure");
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
    log_date += String(GetTimeUnix()) + " " +  date_to_add + "\n";
    writeFile(file, log_date);
  } else {
    addFile(file, String(GetTimeUnix()) + " " +  date_to_add);
  }
}
