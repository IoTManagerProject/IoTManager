#include "Global.h"

//
#include <LITTLEFS.h>

void sendLogData(String file, String topic);

static const char* MODULE = "Log";

#ifdef LOGGING_ENABLED
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
    //считаем количество входов в эту функцию
    enter_to_logging_counter++;
    //создаем в файловой системе переменную количества точек на графике с отметкой _c что значит count
    jsonWriteStr(configOptionJson, value_name + "_c", maxCount);

    //создаем график в приложении с топиком _ch /prefix/3234045-1589487/value_name_ch
    createChart(widget_name, page_name, page_number, "chart", value_name + "_ch", maxCount);

    if (enter_to_logging_counter == LOG1) {
        ts.add(
            LOG1, period_min.toInt() * 1000 * 60, [&](void*) {
                String tmp_buf_1 = selectFromMarkerToMarker(logging_value_names_list, ",", 0);
                deleteOldDate("log." + tmp_buf_1 + ".txt", jsonReadInt(configOptionJson, tmp_buf_1 + "_c"), jsonReadStr(configLiveJson, tmp_buf_1));
                pm.info("logging for " + tmp_buf_1 + " done");
            },
            nullptr, false);
    }
    if (enter_to_logging_counter == LOG2) {
        ts.add(
            LOG2, period_min.toInt() * 1000 * 60, [&](void*) {
                String tmp_buf_2 = selectFromMarkerToMarker(logging_value_names_list, ",", 1);
                deleteOldDate("log." + tmp_buf_2 + ".txt", jsonReadInt(configOptionJson, tmp_buf_2 + "_c"), jsonReadStr(configLiveJson, tmp_buf_2));
                pm.info("logging for " + tmp_buf_2 + " done");
            },
            nullptr, false);
    }
    if (enter_to_logging_counter == LOG3) {
        ts.add(
            LOG3, period_min.toInt() * 1000 * 60, [&](void*) {
                String tmp_buf_3 = selectFromMarkerToMarker(logging_value_names_list, ",", 2);
                deleteOldDate("log." + tmp_buf_3 + ".txt", jsonReadInt(configOptionJson, tmp_buf_3 + "_c"), jsonReadStr(configLiveJson, tmp_buf_3));
                pm.info("logging for " + tmp_buf_3 + " done");
            },
            nullptr, false);
    }
    if (enter_to_logging_counter == LOG4) {
        ts.add(
            LOG4, period_min.toInt() * 1000 * 60, [&](void*) {
                String tmp_buf_4 = selectFromMarkerToMarker(logging_value_names_list, ",", 3);
                deleteOldDate("log." + tmp_buf_4 + ".txt", jsonReadInt(configOptionJson, tmp_buf_4 + "_c"), jsonReadStr(configLiveJson, tmp_buf_4));
                pm.info("logging for " + tmp_buf_4 + " done");
            },
            nullptr, false);
    }
    if (enter_to_logging_counter == LOG5) {
        ts.add(
            LOG5, period_min.toInt() * 1000 * 60, [&](void*) {
                String tmp_buf_5 = selectFromMarkerToMarker(logging_value_names_list, ",", 4);
                deleteOldDate("log." + tmp_buf_5 + ".txt", jsonReadInt(configOptionJson, tmp_buf_5 + "_c"), jsonReadStr(configLiveJson, tmp_buf_5));
                pm.info("logging for " + tmp_buf_5 + " done");
            },
            nullptr, false);
    }
}

/*
* Удаление стрых данных и запись новых
*/
void deleteOldDate(const String filename, size_t max_lines_cnt, String payload) {
    String log_date = readFile(filename, 5120);
    size_t lines_cnt = itemsCount(log_date, "\r\n");

    pm.info("log " + filename + " (" + String(lines_cnt, DEC) + ")");

    if ((lines_cnt > max_lines_cnt + 1) || !lines_cnt) {
        removeFile(filename);
        lines_cnt = 0;
    }

    if (lines_cnt > max_lines_cnt) {
        log_date = deleteBeforeDelimiter(log_date, "\r\n");
        if (timeNow->hasTimeSynced()) {
            log_date += timeNow->getTimeUnix() + " " + payload + "\r\n";
            writeFile(filename, log_date);
        }
    } else {
        if (timeNow->hasTimeSynced()) {
            addFile(filename, timeNow->getTimeUnix() + " " + payload);
        }
    }
}

//=========================================Выбор какие данные отправлять==================================================================
void choose_log_date_and_send() {
    String all_line = logging_value_names_list;
    while (all_line.length() != 0) {
        String tmp = selectToMarker(all_line, ",");
        sendLogData("log." + tmp + ".txt", tmp + "_ch");
        all_line = deleteBeforeDelimiter(all_line, ",");
    }
}
//=========================================Отправка данных===================================================================================
void sendLogData(String file, String topic) {
    String log_date = readFile(file, 5120);
    if (log_date != "failed") {
        log_date.replace("\r\n", "\n");
        log_date.replace("\r", "\n");
        String buf = "{}";
        String json_array;
        String unix_time;
        String value;
        while (log_date.length()) {
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
        pm.info(json_array);

        MqttClient::publishChart(topic, json_array);
    }
}

void clean_log_date() {
    String all_line = logging_value_names_list;
    while (all_line.length()) {
        String tmp = selectToMarker(all_line, ",");
        removeFile("log." + tmp + ".txt");
        all_line = deleteBeforeDelimiter(all_line, ",");
    }
}
#endif