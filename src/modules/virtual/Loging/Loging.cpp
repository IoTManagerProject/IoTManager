#include "Global.h"
#include "classes/IoTItem.h"
#include "ESPConfiguration.h"
#include "NTP.h"

void *getAPI_Date(String params);

class Loging : public IoTItem {
   private:
    String logid;
    String id;
    String tmpValue;
    String filesList = "";

    int _publishType = -2;
    int _wsNum = -1;

    int points;
    int keepdays;

    IoTItem *dateIoTItem;

    String prevDate = "";
    bool firstTimeDate = true;

    long interval;

   public:
    Loging(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, F("logid"), logid);
        jsonRead(parameters, F("id"), id);
        jsonRead(parameters, F("points"), points);
        if (points > 300) {
            points = 300;
            SerialPrint("E", F("Loging"), "'" + id + "' user set more points than allowed, value reset to 300");
        }
        jsonRead(parameters, F("int"), interval);
        interval = interval * 1000 * 60;  //приводим к милисекундам
        jsonRead(parameters, F("keepdays"), keepdays);

        //создадим экземпляр класса даты
        dateIoTItem = (IoTItem *)getAPI_Date("{\"id\": \"" + id + "-date\",\"int\":\"20\",\"subtype\":\"date\"}");
        IoTItems.push_back(dateIoTItem);
        SerialPrint("E", F("Loging"), "created date instance " + id);
    }

    void doByInterval() {
        //если объект логгирования не был создан
        if (!isItemExist(logid)) {
            SerialPrint("E", F("Loging"), "'" + id + "' loging object not exist, return");
            return;
        }

        String value = getItemValue(logid);
        //если значение логгирования пустое
        if (value == "") {
            SerialPrint("E", F("Loging"), "'" + id + "' loging value is empty, return");
            return;
        }

        //если время не было получено из интернета
        if (!isTimeSynch) {
            SerialPrint("E", F("Loging"), "'" + id + "' Сant loging - time not synchronized, return");
            return;
        }

        regEvent(value, F("Loging"));

        String logData;

        jsonWriteInt(logData, "x", unixTime);
        jsonWriteFloat(logData, "y1", value.toFloat());

        //прочитаем путь к файлу последнего сохранения
        String filePath = readDataDB(id);

        //если данные о файле отсутствуют, создадим новый
        if (filePath == "failed" || filePath == "") {
            SerialPrint("E", F("Loging"), "'" + id + "' file path not found, start create new file");
            createNewFileWithData(logData);
            return;
        } else {
            //если файл все же есть но был создан не сегодня, то создаем сегодняшний
            if (getTodayDateDotFormated() != getDateDotFormatedFromUnix(getFileUnixLocalTime(filePath))) {
                SerialPrint("E", F("Loging"), "'" + id + "' file too old, start create new file");
                createNewFileWithData(logData);
                return;
            }
        }

        //считаем количество строк и определяем размер файла
        size_t size = 0;
        int lines = countJsonObj(filePath, size);
        SerialPrint("i", F("Loging"), "'" + id + "' " + "lines = " + String(lines) + ", size = " + String(size));

        //если количество строк до заданной величины и дата не менялась
        if (lines <= points && !hasDayChanged()) {
            //просто добавим в существующий файл новые данные
            addNewDataToExistingFile(filePath, logData);
            //если больше или поменялась дата то создадим следующий файл
        } else {
            createNewFileWithData(logData);
        }
        //запускаем процедуру удаления старых файлов если память переполняется
        deleteLastFile();
    }

    void SetDoByInterval(String valse) {
        String value = valse;
        //если значение логгирования пустое
        if (value == "") {
            SerialPrint("E", F("LogingEvent"), "'" + id + "' loging value is empty, return");
            return;
        }
        //если время не было получено из интернета
        if (!isTimeSynch) {
            SerialPrint("E", F("LogingEvent"), "'" + id + "' Сant loging - time not synchronized, return");
            return;
        }
        regEvent(value, F("LogingEvent"));
        String logData;
        jsonWriteInt(logData, "x", unixTime);
        jsonWriteFloat(logData, "y1", value.toFloat());
        //прочитаем путь к файлу последнего сохранения
        String filePath = readDataDB(id);

        //если данные о файле отсутствуют, создадим новый
        if (filePath == "failed" || filePath == "") {
            SerialPrint("E", F("LogingEvent"), "'" + id + "' file path not found, start create new file");
            createNewFileWithData(logData);
            return;
        } else {
            //если файл все же есть но был создан не сегодня, то создаем сегодняшний
            if (getTodayDateDotFormated() != getDateDotFormatedFromUnix(getFileUnixLocalTime(filePath))) {
                SerialPrint("E", F("LogingEvent"), "'" + id + "' file too old, start create new file");
                createNewFileWithData(logData);
                return;
            }
        }

        //считаем количество строк и определяем размер файла
        size_t size = 0;
        int lines = countJsonObj(filePath, size);
        SerialPrint("i", F("LogingEvent"), "'" + id + "' " + "lines = " + String(lines) + ", size = " + String(size));

        //если количество строк до заданной величины и дата не менялась
        if (lines <= points && !hasDayChanged()) {
            //просто добавим в существующий файл новые данные
            addNewDataToExistingFile(filePath, logData);
            //если больше или поменялась дата то создадим следующий файл
        } else {
            createNewFileWithData(logData);
        }
        //запускаем процедуру удаления старых файлов если память переполняется
        deleteLastFile();
    }
    void createNewFileWithData(String &logData) {
        logData = logData + ",";
        String path = "/lg/" + id + "/" + String(unixTimeShort) + ".txt";  //создадим путь вида /lg/id/133256622333.txt
        //создадим пустой файл
        if (writeEmptyFile(path) != "sucсess") {
            SerialPrint("E", F("Loging"), "'" + id + "' file writing error, return");
            return;
        }

        //запишем в него данные
        if (addFile(path, logData) != "sucсess") {
            SerialPrint("E", F("Loging"), "'" + id + "' data writing error, return");
            return;
        }
        //запишем путь к нему в базу данных
        if (saveDataDB(id, path) != "sucсess") {
            SerialPrint("E", F("Loging"), "'" + id + "' db file writing error, return");
            return;
        }
        SerialPrint("i", F("Loging"), "'" + id + "' file created http://" + WiFi.localIP().toString() + path);
    }

    void addNewDataToExistingFile(String &path, String &logData) {
        logData = logData + ",";
        if (addFile(path, logData) != "sucсess") {
            SerialPrint("i", F("Loging"), "'" + id + "' file writing error, return");
            return;
        };
        SerialPrint("i", F("Loging"), "'" + id + "' loging in file http://" + WiFi.localIP().toString() + path);
    }

    bool hasDayChanged() {
        bool changed = false;
        String currentDate = getTodayDateDotFormated();
        if (!firstTimeDate) {
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
        firstTimeDate = false;
        prevDate = currentDate;
        return changed;
    }

    void publishValue() {
        String dir = "/lg/" + id;
        filesList = getFilesList(dir);

        SerialPrint("i", F("Loging"), "file list: " + filesList);

        int f = 0;

        bool noData = true;

        while (filesList.length()) {
            String path = selectToMarker(filesList, ";");

            path = "/lg/" + id + path;

            f++;

            unsigned long fileUnixTimeLocal = getFileUnixLocalTime(path);

            unsigned long reqUnixTime = strDateToUnix(getItemValue(id + "-date"));
            if (fileUnixTimeLocal > reqUnixTime && fileUnixTimeLocal < reqUnixTime + 86400) {
                noData = false;
                String json = getAdditionalJson();
                if (_publishType == TO_MQTT) {
                    publishChartFileToMqtt(path, id, calculateMaxCount());
                } else if (_publishType == TO_WS) {
                    sendFileToWsByFrames(path, "charta", json, _wsNum, WEB_SOCKETS_FRAME_SIZE);
                } else if (_publishType == TO_MQTT_WS) {
                    sendFileToWsByFrames(path, "charta", json, _wsNum, WEB_SOCKETS_FRAME_SIZE);
                    publishChartFileToMqtt(path, id, calculateMaxCount());
                }
                SerialPrint("i", F("Loging"), String(f) + ") " + path + ", " + getDateTimeDotFormatedFromUnix(fileUnixTimeLocal) + ", sent");
            } else {
                SerialPrint("i", F("Loging"), String(f) + ") " + path + ", " + getDateTimeDotFormatedFromUnix(fileUnixTimeLocal) + ", skipped");
            }

            filesList = deleteBeforeDelimiter(filesList, ";");
        }
        //если данных нет отправляем пустой грфик
        if (noData) {
            clearValue();
        }
    }

    String getAdditionalJson() {
        String topic = mqttRootDevice + "/" + id;
        String json = "{\"maxCount\":" + String(calculateMaxCount()) + ",\"topic\":\"" + topic + "\"}";
        return json;
    }

    void publishChartToWsSinglePoint(String value) {
        String topic = mqttRootDevice + "/" + id;
        String json = "{\"maxCount\":" + String(calculateMaxCount()) + ",\"topic\":\"" + topic + "\",\"status\":[{\"x\":" + String(unixTime) + ",\"y1\":" + value + "}]}";
        sendStringToWs("chartb", json, -1);
    }

    void clearValue() {
        String topic = mqttRootDevice + "/" + id;
        String json = "{\"maxCount\":0,\"topic\":\"" + topic + "\",\"status\":[]}";
        sendStringToWs("chartb", json, -1);
    }

    void clearHistory() {
        String dir = "/lg/" + id;
        cleanDirectory(dir);
    }

    void deleteLastFile() {
        IoTFSInfo tmp = getFSInfo();
        SerialPrint("i", "Loging", String(tmp.freePer) + " % free flash remaining");
        if (tmp.freePer <= 20.00) {
            String dir = "/lg/" + id;
            filesList = getFilesList(dir);
            int i = 0;
            while (filesList.length()) {
                String path = selectToMarker(filesList, ";");
                path = dir + path;
                i++;
                if (i == 1) {
                    removeFile(path);
                    SerialPrint("!", "Loging", String(i) + ") " + path + " => oldest files been deleted");
                    return;
                }

                filesList = deleteBeforeDelimiter(filesList, ";");
            }
        }
    }

    void setPublishDestination(int publishType, int wsNum) {
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
                if (interval != 0) {
                    this->doByInterval();
                }
            }
        }
    }

    void regEvent(const String &value, const String &consoleInfo, bool error = false, bool genEvent = true) {
        String userDate = getItemValue(id + "-date");
        String currentDate = getTodayDateDotFormated();
        //отправляем в график данные только когда выбран сегодняшний день
        if (userDate == currentDate) {
            // generateEvent(_id, value);
            // publishStatusMqtt(_id, value);

            publishChartToWsSinglePoint(value);
            // SerialPrint("i", "Sensor " + consoleInfo, "'" + _id + "' data: " + value + "'");
        }
    }

    //просто максимальное количество точек
    int calculateMaxCount() {
        return 86400;
    }

    //путь вида: /lg/log/1231231.txt
    unsigned long getFileUnixLocalTime(String path) {
        return gmtTimeToLocal(selectToMarkerLast(deleteToMarkerLast(path, "."), "/").toInt() + START_DATETIME);
    }
    void setValue(const IoTValue &Value, bool genEvent = true) {
        value = Value;
        this->SetDoByInterval(String(value.valD));
        SerialPrint("i", "Loging", "setValue:" + String(value.valD));
        regEvent(value.valS, "Loging", false, genEvent);
    }
};

void *getAPI_Loging(String subtype, String param) {
    if (subtype == F("Loging")) {
        return new Loging(param);
    } else {
        return nullptr;
    }
}

class Date : public IoTItem {
   private:
    bool firstTime = true;

   public:
    String id;
    Date(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, F("id"), id);
        value.isDecimal = false;
    }

    void setValue(const String &valStr, bool genEvent = true) {
        value.valS = valStr;
        setValue(value, genEvent);
    }

    void setValue(const IoTValue &Value, bool genEvent = true) {
        value = Value;
        regEvent(value.valS, "", false, genEvent);
        //отправка данных при изменении даты
        for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
            if ((*it)->getSubtype() == "Loging") {
                if ((*it)->getID() == selectToMarker(id, "-")) {
                    (*it)->setPublishDestination(TO_MQTT_WS, -1);
                    (*it)->publishValue();
                }
            }
        }
    }

    void setTodayDate() {
        setValue(getTodayDateDotFormated());
        SerialPrint("E", F("Loging"), "today date set " + getTodayDateDotFormated());
    }

    void doByInterval() {
        if (isTimeSynch) {
            if (firstTime) {
                setTodayDate();
                firstTime = false;
            }
        }
    }
};

void *getAPI_Date(String param) {
    return new Date(param);
}
