#include "Global.h"
#include "classes/IoTItem.h"
#include "ESPConfiguration.h"
#include "NTP.h"

class LogingDaily : public IoTItem {
   private:
    String logid;
    String id;
    String filesList = "";

    String descr;

    int _publishType = -2;
    int _wsNum = -1;

    int points;

    int testMode;

    int telegram;

    IoTItem *dateIoTItem;

    String prevDate = "";
    bool firstTimeInit = true;

    long interval;

   public:
    LogingDaily(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, F("logid"), logid);
        jsonRead(parameters, F("id"), id);
        jsonRead(parameters, F("points"), points);
        jsonRead(parameters, F("test"), testMode);
        jsonRead(parameters, F("telegram"), telegram);
        jsonRead(parameters, F("descr"), descr);

        if (points > 365) {
            points = 365;
            SerialPrint("E", F("LogingDaily"), "'" + id + "' user set more points than allowed, value reset to 365");
        }
        jsonRead(parameters, F("int"), interval);
        interval = interval * 1000 * 60;  // приводим к милисекундам
    }

    void doByInterval() {
        if (hasDayChanged() || testMode == 1) {
            execute();
        }
    }

    void execute() {
        // если объект логгирования не был создан
        if (!isItemExist(logid)) {
            SerialPrint("E", F("LogingDaily"), "'" + id + "' LogingDaily object not exist, return");
            return;
        }

        String value = getItemValue(logid);

        // если значение логгирования пустое
        if (value == "") {
            SerialPrint("E", F("LogingDaily"), "'" + id + "' LogingDaily value is empty, return");
            return;
        }

        // если время не было получено из интернета
        if (!isTimeSynch) {
            SerialPrint("E", F("LogingDaily"), "'" + id + "' Cant LogingDaily - time not synchronized, return");
            return;
        }

        String logData;

        float currentValue = value.toFloat();
        // прочитаем предудущее значение
        float prevValue = readDataDB(id + "-v").toFloat();
        // сохраним в базу данных текущее значение, понадобится в следующие сутки
        saveDataDB(id + "-v", value);

        float difference = currentValue - prevValue;

        if (telegram == 1) {
            String msg = descr + " " + String(currentValue) + " " + String(difference);
            for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
                if ((*it)->getSubtype() == "TelegramLT" || "Telegram") {
                    (*it)->sendTelegramMsg(false, msg);
                }
            }
        }

        jsonWriteInt(logData, "x", unixTime - 120);
        jsonWriteFloat(logData, "y1", difference);

        // прочитаем путь к файлу последнего сохранения
        String filePath = readDataDB(id);

        // если данные о файле отсутствуют, создадим новый
        if (filePath == "failed" || filePath == "") {
            SerialPrint("E", F("LogingDaily"), "'" + id + "' file path not found, start create new file");
            createNewFileWithData(logData);
            return;
        }

        // считаем количество строк и определяем размер файла
        size_t size = 0;
        int lines = countJsonObj(filePath, size);
        SerialPrint("i", F("LogingDaily"), "'" + id + "' " + "lines = " + String(lines) + ", size = " + String(size));

        // если количество строк до заданной величины и дата не менялась
        if (lines <= points && !hasDayChanged()) {
            // просто добавим в существующий файл новые данные
            addNewDataToExistingFile(filePath, logData);
            // если больше или поменялась дата то создадим следующий файл
        } else {
            createNewFileWithData(logData);
        }
    }

    void createNewFileWithData(String &logData) {
        logData = logData + ",";

        String path = "/lgd/" + id + "/" + id + ".txt";  // создадим путь вида /lgd/id/id.txt
        // создадим пустой файл
        if (writeEmptyFile(path) != "success") {
            SerialPrint("E", F("LogingDaily"), "'" + id + "' file writing error, return");
            return;
        }

        // запишем в него данные
        if (addFile(path, logData) != "success") {
            SerialPrint("E", F("LogingDaily"), "'" + id + "' data writing error, return");
            return;
        }
        // запишем путь к нему в базу данных
        if (saveDataDB(id, path) != "success") {
            SerialPrint("E", F("LogingDaily"), "'" + id + "' db file writing error, return");
            return;
        }
        SerialPrint("i", F("LogingDaily"), "'" + id + "' file created http://" + WiFi.localIP().toString() + path);
    }

    void addNewDataToExistingFile(String &path, String &logData) {
        logData = logData + ",";
        if (addFile(path, logData) != "success") {
            SerialPrint("i", F("LogingDaily"), "'" + id + "' file writing error, return");
            return;
        };
        SerialPrint("i", F("LogingDaily"), "'" + id + "' LogingDaily in file http://" + WiFi.localIP().toString() + path);
    }

    bool hasDayChanged() {
        bool changed = false;
        String currentDate = getTodayDateDotFormated();
        if (!firstTimeInit) {
            if (prevDate != currentDate) {
                changed = true;
                SerialPrint("i", F("NTP"), "Change day event");
#if defined(ESP8266)
                FileFS.gc();
#endif
#if defined(ESP32)
#endif
            }
        }
        firstTimeInit = false;
        prevDate = currentDate;
        return changed;
    }

    void publishValue() {
        String dir = "/lgd/" + id;
        filesList = getFilesList(dir);

        SerialPrint("i", F("LogingDaily"), "file list: " + filesList);

        int f = 0;

        while (filesList.length()) {
            String path = selectToMarker(filesList, ";");

            path = "/lgd/" + id + path;

            f++;
            String json = getAdditionalJson();
            if (_publishType == TO_MQTT) {
                publishChartFileToMqtt(path, id, calculateMaxCount());
            } else if (_publishType == TO_WS) {
                sendFileToWsByFrames(path, "charta", json, _wsNum, WEB_SOCKETS_FRAME_SIZE);
            } else if (_publishType == TO_MQTT_WS) {
                publishChartFileToMqtt(path, id, calculateMaxCount());
                sendFileToWsByFrames(path, "charta", json, _wsNum, WEB_SOCKETS_FRAME_SIZE);
            }
            SerialPrint("i", F("LogingDaily"), String(f) + ") " + path + ", sent");

            filesList = deleteBeforeDelimiter(filesList, ";");
        }
    }

    String getAdditionalJson() {
        String topic = mqttRootDevice + "/" + id;
        String json = "{\"maxCount\":" + String(calculateMaxCount()) + ",\"topic\":\"" + topic + "\"}";
        return json;
    }

    void clearHistory() {
        String dir = "/lgd/" + id;
        cleanDirectory(dir);
    }

    // void publishChartToWsSinglePoint(String value) {
    //     String topic = mqttRootDevice + "/" + id;
    //     String json = "{\"maxCount\":" + String(calculateMaxCount()) + ",\"topic\":\"" + topic + "\",\"status\":[{\"x\":" + String(unixTime) + ",\"y1\":" + value + "}]}";
    //     String pk = "/string/chart.json|" + json;
    //     standWebSocket.broadcastTXT(pk);
    // }

    void setPublishDestination(int publishType, int wsNum = -1) {
        _publishType = publishType;
        _wsNum = wsNum;
    }

    String getValue() {
        return "";
    }

    void loop() {
        if (enableDoByInt) {
            currentMillis = millis();
            difference = currentMillis - prevMillis;
            if (difference >= interval) {
                prevMillis = millis();
                this->doByInterval();
            }
        }
    }

    // просто максимальное количество точек
    int calculateMaxCount() {
        return 86400;
    }
};

void *getAPI_LogingDaily(String subtype, String param) {
    if (subtype == F("LogingDaily")) {
        return new LogingDaily(param);
    } else {
        return nullptr;
    }
}
