//===============================================Логирование============================================================
void logging() {

  static boolean flag = true;

  String sensor_name = sCmd.next();
  String period_min = sCmd.next();
  String maxCount = sCmd.next();

  String optimization = sCmd.next();

  String widget_name = sCmd.next();
  widget_name.replace("#", " ");
  String page_name = sCmd.next();
  String page_number = sCmd.next();

  if (optimization == "fast") chart_data_in_solid_array = true;
  if (optimization == "slow") chart_data_in_solid_array = false;

  if (sensor_name == "analog") jsonWriteStr(optionJson, "analog_logging_count", maxCount);
  if (sensor_name == "level") jsonWriteStr(optionJson, "level_logging_count", maxCount);
  if (sensor_name == "dallas") jsonWriteStr(optionJson, "dallas_logging_count", maxCount);
  if (sensor_name == "dhtT") jsonWriteStr(optionJson, "dhtT_logging_count", maxCount);
  if (sensor_name == "dhtH") jsonWriteStr(optionJson, "dhtH_logging_count", maxCount);

  if (sensor_name == "analog") createChart (widget_name, page_name, page_number, "widgets/widget.chart.json", "loganalog", maxCount);
  if (sensor_name == "level") createChart (widget_name, page_name, page_number, "widgets/widget.chart.json", "loglevel", maxCount);
  if (sensor_name == "dallas") createChart (widget_name, page_name, page_number, "widgets/widget.chart.json", "logdallas", maxCount);
  if (sensor_name == "dhtT") createChart (widget_name, page_name, page_number, "widgets/widget.chart.json", "logdhtT", maxCount);
  if (sensor_name == "dhtH") createChart (widget_name, page_name, page_number, "widgets/widget.chart.json", "logdhtH", maxCount);

  if (sensor_name == "analog") {
    flagLoggingAnalog = true;
    ts.remove(ANALOG_LOG);
    ts.add(ANALOG_LOG, period_min.toInt() * 1000 * 60, [&](void*) {
      deleteOldDate("log.analog.txt",  jsonReadtoInt(optionJson, "analog_logging_count"), jsonRead(configJson, "analog"));
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
      deleteOldDate("log.dallas.txt",  jsonReadtoInt(optionJson, "dallas_logging_count"), jsonRead(configJson, "dallas"));
    }, nullptr, true);
  }

  if (sensor_name == "dhtT") {
    flagLoggingdhtT = true;
    ts.remove(dhtT_LOG);
    ts.add(dhtT_LOG, period_min.toInt() * 1000 * 60, [&](void*) {
      deleteOldDate("log.dhtT.txt",  jsonReadtoInt(optionJson, "dhtT_logging_count"), jsonRead(configJson, "dhtT"));
    }, nullptr, true);
  }

  if (sensor_name == "dhtH") {
    flagLoggingdhtH = true;
    ts.remove(dhtH_LOG);
    ts.add(dhtH_LOG, period_min.toInt() * 1000 * 60, [&](void*) {
      deleteOldDate("log.dhtH.txt",  jsonReadtoInt(optionJson, "dhtH_logging_count"), jsonRead(configJson, "dhtH"));
    }, nullptr, true);
  }
}

//=========================================Удаление стрых данных и запись новых==================================================================
void deleteOldDate(String file, int seted_number_of_lines, String date_to_add) {
  String log_date = readFile(file, 5000);
  int current_number_of_lines = count(log_date, "\r\n");
  Serial.println("=====> [i] in log file " + file + " " + current_number_of_lines + " lines");

  if (current_number_of_lines > seted_number_of_lines + 1) {
    SPIFFS.remove("/" + file);
    current_number_of_lines = 0;
  }
  if (current_number_of_lines == 0) {
    SPIFFS.remove("/" + file);
    current_number_of_lines = 0;
  }
  if (current_number_of_lines > seted_number_of_lines) {
    log_date = deleteBeforeDelimiter(log_date, "\r\n");
    if (GetTimeUnix() != "failed") {
      log_date += GetTimeUnix() + " " +  date_to_add + "\r\n";
      writeFile(file, log_date);
    }
  } else {
    if (GetTimeUnix() != "failed") {
      addFile(file, GetTimeUnix() + " " +  date_to_add);
    }
  }
  log_date = "";
}

//=========================================Выбор какие данные отправлять==================================================================
void choose_log_date_and_send() {

  if (flagLoggingAnalog) sendLogData("log.analog.txt", "loganalog", chart_data_in_solid_array);
  if (flagLoggingLevel) sendLogData("log.level.txt", "loglevel", chart_data_in_solid_array);
  if (flagLoggingDallas) sendLogData("log.dallas.txt", "logdallas", chart_data_in_solid_array);
  if (flagLoggingdhtT) sendLogData("log.dhtT.txt", "logdhtT", chart_data_in_solid_array);
  if (flagLoggingdhtH) sendLogData("log.dhtH.txt", "logdhtH", chart_data_in_solid_array);
}

//=========================================Отправка данных===================================================================================
void sendLogData(String file, String topic, boolean type) {
  if (type) {
    //----------------------------------------------
    String log_date = readFile(file, 5000);
    if (log_date != "Failed") {
      log_date.replace("\r\n", "\n");
      log_date.replace("\r", "\n");
      String buf = "{}";
      String json_array;
      String unix_time;
      String value;
      while (log_date.length() != 0) {
        String tmp = selectToMarker (log_date, "\n");
        log_date = deleteBeforeDelimiter(log_date, "\n");
        unix_time = selectToMarker (tmp, " ");
        jsonWriteInt(buf, "x", unix_time.toInt());
        value = deleteBeforeDelimiter(tmp, " ");
        jsonWriteFloat(buf, "y1", value.toFloat());
        if (log_date.length() < 3) {
          json_array += buf;
        } else {
          json_array += buf + ",";
        }
        buf = "{}";
      }
      unix_time = "";
      value = "";
      log_date = "";
      json_array = "{\"status\":[" + json_array + "]}";
      Serial.println(json_array);
      sendCHART(topic, json_array);
      json_array = "";
      getMemoryLoad("[i] after send log date");
    }
    //----------------------------------------------
  } else {
    //----------------------------------------------
    File configFile = SPIFFS.open("/" + file, "r");
    if (!configFile) {
      return;
    }
    configFile.seek(0, SeekSet); //поставим курсор в начало файла
    while (configFile.position() != configFile.size()) {
      String tmp = configFile.readStringUntil('\r\n');
      String unix_time = selectToMarker (tmp, " ");
      String value = deleteBeforeDelimiter(tmp, " ");
      String final_line = "{\"status\":{\"x\":" + unix_time + ",\"y1\":" + value + "}}";
      //Serial.println(final_line);
      sendCHART(topic, final_line);
    }
    getMemoryLoad("[i] after send log date");
  }
  //----------------------------------------------
}
//=========================================Очистка данных===================================================================================
void clean_log_date() {
  SPIFFS.remove("/log.analog.txt");
  SPIFFS.remove("/log.level.txt");
  SPIFFS.remove("/log.dallas.txt");
  SPIFFS.remove("/log.dhtT.txt");
  SPIFFS.remove("/log.dhtH.txt");
}
