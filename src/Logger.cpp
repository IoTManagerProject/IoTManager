#include "Logger.h"

#include "MqttClient.h"

//
#include <LITTLEFS.h>

// Logging
String logging_value_names_list;
size_t enter_to_logging_counter;

static const char* MODULE = "Log";

namespace Logger {

size_t add(String name) {
    logging_value_names_list += name + ",";
    //считаем количество
    enter_to_logging_counter++;
    return enter_to_logging_counter;
}
void init() {
    logging_value_names_list = "";
    enter_to_logging_counter = LOG1 - 1;
}

const String getLogFilename(String name) {
    return "log." + name + ".txt";
}

void deleteOldDataTask(LoggerLog_t log) {
    String name = selectFromMarkerToMarker(logging_value_names_list, ",", log);
    pm.info("task: " + name);
    int lines = options.readInt(name + "_c");
    String value = liveData.read(name);
    deleteOldData(getLogFilename(name), lines, value);
}
/*
* Удаление стрых данных и запись новых
*/
void deleteOldData(const String filename, size_t max_lines_cnt, String payload) {
    String buf;
    if (!readFile(filename.c_str(), buf, 5120)) {
        return;
    }
    size_t lines_cnt = itemsCount(buf, "\r\n");

    pm.info("log " + filename + " (" + String(lines_cnt, DEC) + ")");

    if ((lines_cnt > max_lines_cnt + 1) || !lines_cnt) {
        removeFile(filename);
        lines_cnt = 0;
    }

    if (lines_cnt > max_lines_cnt) {
        buf = deleteBeforeDelimiter(buf, "\r\n");
        if (timeNow->hasTimeSynced()) {
            buf += timeNow->getTimeUnix() + " " + payload + "\r\n";
            writeFile(filename, buf);
        }
    } else {
        if (timeNow->hasTimeSynced()) {
            addFile(filename, timeNow->getTimeUnix() + " " + payload);
        }
    }
}

void sendLogData(String file, String topic) {
    String data;
    if (!readFile(file.c_str(), data, 5120)) {
        pm.error("read " + file);
        return;
    }
    data.replace("\r\n", "\n");
    data.replace("\r", "\n");
    String buf = "{}";
    String json_array;
    String unix_time;
    String value;
    while (data.length()) {
        String tmp = selectToMarker(data, "\n");
        data = deleteBeforeDelimiter(data, "\n");
        unix_time = selectToMarker(tmp, " ");
        jsonWriteInt(buf, "x", unix_time.toInt());
        value = deleteBeforeDelimiter(tmp, " ");
        jsonWriteFloat(buf, "y1", value.toFloat());
        if (data.length() < 3) {
            json_array += buf;
        } else {
            json_array += buf + ",";
        }
        buf = "{}";
    }
    unix_time = "";
    value = "";
    data = "";
    json_array = "{\"status\":[" + json_array + "]}";
    pm.info(json_array);

    MqttClient::publishChart(topic, json_array);
}

void choose_log_date_and_send() {
    String all_line = logging_value_names_list;
    while (all_line.length() != 0) {
        String tmp = selectToMarker(all_line, ",");
        sendLogData("log." + tmp + ".txt", tmp + "_ch");
        all_line = deleteBeforeDelimiter(all_line, ",");
    }
}
void clear() {
    String all_line = logging_value_names_list;
    while (all_line.length()) {
        String tmp = selectToMarker(all_line, ",");
        removeFile("log." + tmp + ".txt");
        all_line = deleteBeforeDelimiter(all_line, ",");
    }
}

}  // namespace Logger