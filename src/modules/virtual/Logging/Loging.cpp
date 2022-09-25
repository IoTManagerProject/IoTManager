#include "Global.h"
#include "classes/IoTItem.h"
#include "ESPConfiguration.h"
#include "NTP.h"

void *getAPI_Date(String params);

class Loging : public IoTItem {
   private:
    String logid;
    String id;
    String filesList = "";

    int _publishType = -2;
    int _wsNum = -1;

    int points;
    int keepdays;

    IoTItem *dateIoTItem;

    String prevDate = "";
    bool firstTimeDate = true;

    unsigned long interval;

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
                if (_publishType == TO_MQTT) {
                    sendChartFileToMqtt(path);
                } else if (_publishType == TO_WS) {
                    sendChartFileToWs(path, _wsNum, 1000);
                } else if (_publishType == TO_MQTT_WS) {
                    sendChartFileToMqtt(path);
                    sendChartFileToWs(path, _wsNum, 1000);
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

    void clearValue() {
        SerialPrint("i", F("Loging"), "clear chart");
        sendChartFileToWs("/empty.txt", -1, 1000);
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

    bool sendChartFileToMqtt(String path) {
        File configFile = FileFS.open(path, FILE_READ);
        if (!configFile) {
            SerialPrint("E", F("Loging"), path + " file reading error, json not created, return");
            return false;
        }
        String oneSingleJson = configFile.readString();
        configFile.close();
        // String topic = mqttRootDevice + "/" + id;
        // oneSingleJson = "{\"maxCount\":" + String(calculateMaxCount()) + ",\"topic\":\"" + topic + "\",\"status\":[" + oneSingleJson + "]}";
        oneSingleJson += "]}";
        oneSingleJson.replace("},]}", "}]}");
        SerialPrint("i", "Loging", "json size: " + String(oneSingleJson.length()));
        publishChartMqtt(id, oneSingleJson);
        return true;
    }

    void sendChartFileToWs(String filename, int num, size_t frameSize) {
        String topic = mqttRootDevice + "/" + id;
        String st = "/st/chart.json|" + topic;
        if (num == -1) {
            standWebSocket.broadcastTXT(st);
        } else {
            standWebSocket.sendTXT(num, st);
        }
        String path = filepath(filename);
        auto file = FileFS.open(path, "r");
        if (!file) {
            SerialPrint(F("E"), F("FS"), F("reed file error"));
            return;
        }
        size_t fileSize = file.size();
        SerialPrint(F("i"), F("FS"), "Send file '" + String(filename) + "', file size: " + String(fileSize));
        uint8_t payload[frameSize];
        int countRead = file.read(payload, sizeof(payload));
        while (countRead > 0) {
            if (num == -1) {
                standWebSocket.broadcastBIN(payload, countRead);
            } else {
                standWebSocket.sendBIN(num, payload, countRead);
            }
            countRead = file.read(payload, sizeof(payload));
        }
        file.close();
        String end = "/end/chart.json|" + topic;
        if (num == -1) {
            standWebSocket.broadcastTXT(end);
        } else {
            standWebSocket.sendTXT(num, end);
        }
    }

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

    void regEvent(String value, String consoleInfo = "") {
        String userDate = getItemValue(id + "-date");
        String currentDate = getTodayDateDotFormated();
        //отправляем в график данные только когда выбран сегодняшний день
        if (userDate == currentDate) {
            // generateEvent(_id, value);
            // publishStatusMqtt(_id, value);
            // String json = createSingleJson(_id, value);
            // publishChartWs(-1, json);
            // SerialPrint("i", "Sensor " + consoleInfo, "'" + _id + "' data: " + value + "'");
        }
    }

    String createSingleJson(String id, String value) {
        String topic = mqttRootDevice + "/" + _id;
        return "{\"topic\":\"" + topic + "\",\"status\":[{\"x\":" + String(unixTime) + ",\"y1\":" + value + "}]}";
    }

    String createEmtyJson() {
        String topic = mqttRootDevice + "/" + _id;
        return "{\"topic\":\"" + topic + "\",\"status\":[],\"maxCount\":\"0\"}";
    }

    //просто максимальное количество точек
    int calculateMaxCount() {
        return 86400;
    }

    //путь вида: /lg/log/1231231.txt
    unsigned long getFileUnixLocalTime(String path) {
        return gmtTimeToLocal(selectToMarkerLast(deleteToMarkerLast(path, "."), "/").toInt() + START_DATETIME);
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

    void setValue(String valStr) {
        value.valS = valStr;
        setValue(value);
    }

    void setValue(IoTValue Value) {
        value = Value;
        regEvent(value.valS, "");
        //отправка данных при изменении даты
        for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
            if ((*it)->getSubtype() == "Loging") {
                //отправляем только свои данные
                if ((*it)->getID() == selectToMarker(id, "-")) {
                    (*it)->setPublishDestination(TO_MQTT_WS);
                    (*it)->clearValue();
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
