#include "Global.h"

void sendLogData(String file, String topic);

#ifdef logging_enable
//===============================================Логирование============================================================
//logging temp1 1 10 Температура Датчики 2
void logging() {
    String value_name = sCmd.next();
    String period_min = sCmd.next();
    String maxCount = sCmd.next();
    String widget_name = sCmd.next();
    widget_name.replace("#", " ");
    String page_name = sCmd.next();
    String page_number = sCmd.next();
    logging_value_names_list += value_name + ",";
    enter_to_logging_counter++;                                                                                   //считаем количество входов в эту функцию
    jsonWriteStr(configOptionJson, value_name + "_c", maxCount);                                                  //создаем в файловой системе переменную количества точек на графике с отметкой _c что значит count
    createChart(widget_name, page_name, page_number, "widgets/widget.chart.json", value_name + "_ch", maxCount);  //создаем график в приложении с топиком _ch /prefix/3234045-1589487/value_name_ch
    if (enter_to_logging_counter == LOG1) {
        ts.add(
            LOG1, period_min.toInt() * 1000 * 60, [&](void*) {
                String tmp_buf_1 = selectFromMarkerToMarker(logging_value_names_list, ",", 0);
                deleteOldDate("log." + tmp_buf_1 + ".txt", jsonReadInt(configOptionJson, tmp_buf_1 + "_c"), jsonReadStr(configLiveJson, tmp_buf_1));
                Serial.println("[i] LOGGING for sensor '" + tmp_buf_1 + "' done");
            },
            nullptr, false);
    }
    if (enter_to_logging_counter == LOG2) {
        ts.add(
            LOG2, period_min.toInt() * 1000 * 60, [&](void*) {
                String tmp_buf_2 = selectFromMarkerToMarker(logging_value_names_list, ",", 1);
                deleteOldDate("log." + tmp_buf_2 + ".txt", jsonReadInt(configOptionJson, tmp_buf_2 + "_c"), jsonReadStr(configLiveJson, tmp_buf_2));
                Serial.println("[i] LOGGING for sensor '" + tmp_buf_2 + "' done");
            },
            nullptr, false);
    }
    if (enter_to_logging_counter == LOG3) {
        ts.add(
            LOG3, period_min.toInt() * 1000 * 60, [&](void*) {
                String tmp_buf_3 = selectFromMarkerToMarker(logging_value_names_list, ",", 2);
                deleteOldDate("log." + tmp_buf_3 + ".txt", jsonReadInt(configOptionJson, tmp_buf_3 + "_c"), jsonReadStr(configLiveJson, tmp_buf_3));
                Serial.println("[i] LOGGING for sensor '" + tmp_buf_3 + "' done");
            },
            nullptr, false);
    }
    if (enter_to_logging_counter == LOG4) {
        ts.add(
            LOG4, period_min.toInt() * 1000 * 60, [&](void*) {
                String tmp_buf_4 = selectFromMarkerToMarker(logging_value_names_list, ",", 3);
                deleteOldDate("log." + tmp_buf_4 + ".txt", jsonReadInt(configOptionJson, tmp_buf_4 + "_c"), jsonReadStr(configLiveJson, tmp_buf_4));
                Serial.println("[i] LOGGING for sensor '" + tmp_buf_4 + "' done");
            },
            nullptr, false);
    }
    if (enter_to_logging_counter == LOG5) {
        ts.add(
            LOG5, period_min.toInt() * 1000 * 60, [&](void*) {
                String tmp_buf_5 = selectFromMarkerToMarker(logging_value_names_list, ",", 4);
                deleteOldDate("log." + tmp_buf_5 + ".txt", jsonReadInt(configOptionJson, tmp_buf_5 + "_c"), jsonReadStr(configLiveJson, tmp_buf_5));
                Serial.println("[i] LOGGING for sensor '" + tmp_buf_5 + "' done");
            },
            nullptr, false);
    }
}

//=========================================Удаление стрых данных и запись новых==================================================================
void deleteOldDate(String file, int seted_number_of_lines, String date_to_add) {
    String log_date = readFile(file, 5000);
    int current_number_of_lines = itemsCount(log_date, "\r\n");
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
        if (getTimeUnix() != "failed") {
            log_date += getTimeUnix() + " " + date_to_add + "\r\n";
            writeFile(file, log_date);
        }
    } else {
        if (getTimeUnix() != "failed") {
            addFile(file, getTimeUnix() + " " + date_to_add);
        }
    }
    log_date = "";
}

//=========================================Выбор какие данные отправлять==================================================================
void choose_log_date_and_send() {
    String all_line = logging_value_names_list;
    while (all_line.length() != 0) {
        String tmp = selectToMarker(all_line, ",");
        sendLogData("log." + tmp + ".txt", tmp + "_ch");
        all_line = deleteBeforeDelimiter(all_line, ",");
    }
    all_line = "";
}
//=========================================Отправка данных===================================================================================
void sendLogData(String file, String topic) {
    String log_date = readFile(file, 5000);
    if (log_date != "Failed") {
        log_date.replace("\r\n", "\n");
        log_date.replace("\r", "\n");
        String buf = "{}";
        String json_array;
        String unix_time;
        String value;
        while (log_date.length() != 0) {
            String tmp = selectToMarker(log_date, "\n");
            log_date = deleteBeforeDelimiter(log_date, "\n");
            unix_time = selectToMarker(tmp, " ");
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
}

/*
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
*/
//=========================================Очистка данных===================================================================================
void clean_log_date() {
    String all_line = logging_value_names_list;
    while (all_line.length() != 0) {
        String tmp = selectToMarker(all_line, ",");
        SPIFFS.remove("/log." + tmp + ".txt");
        all_line = deleteBeforeDelimiter(all_line, ",");
    }
    all_line = "";
}
#endif