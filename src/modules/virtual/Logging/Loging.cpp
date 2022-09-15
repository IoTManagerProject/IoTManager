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
            SerialPrint("E", F("Loging"), "'" + id + "' loging object not exist");
            return;
        }

        String value = getItemValue(logid);
        //если значение логгирования пустое
        if (value == "") {
            SerialPrint("E", F("Loging"), "'" + id + "' loging value is empty");
            return;
        }

        //если время не было получено из интернета
        if (!isTimeSynch) {
            SerialPrint("E", F("Loging"), "'" + id + "' Сant loging - time not synchronized");
            return;
        }

        regEvent(value, F("Loging"));

        String logData = String(unixTimeShort) + " " + value;

        //прочитаем путь к файлу последнего сохранения
        String filePath = readDataDB(id);

        //если данные о файле отсутствуют, создадим новый
        if (filePath == "failed" || filePath == "") {
            SerialPrint("E", F("Loging"), "'" + id + "' file path not found");
            createNewFileWithData(logData);
            return;
        }

        //считаем количество строк
        int lines = countLines(filePath);
        SerialPrint("i", F("Loging"), "'" + id + "' " + String(lines) + " lines found in file");

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
        String path = "/lg/" + id + "/" + String(unixTimeShort) + ".txt";  //создадим путь вида /lg/id/133256622333.txt
        addFileLn(path, logData);                                          //запишем файл и данные в него
        saveDataDB(id, path);                                              //запишем путь к файлу в базу данных
        SerialPrint("i", F("Loging"), "'" + id + "' file created http://" + WiFi.localIP().toString() + path);
    }

    void addNewDataToExistingFile(String &path, String &logData) {
        addFileLn(path, logData);
        SerialPrint("i", F("Loging"), "'" + id + "' loging in file http://" + WiFi.localIP().toString() + path);
    }

    bool hasDayChanged() {
        bool changed = false;
        String currentDate = getDateDotFormated();
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

    void sendChart() {
        String dir = "lg/" + id;
        filesList = getFilesList(dir);
        int f = 0;

        bool noData = true;

        while (filesList.length()) {
            String buf = selectToMarker(filesList, ";");

            f++;
            int i = 0;

            unsigned long fileUnixTimeGMT = selectToMarkerLast(deleteToMarkerLast(buf, "."), "/").toInt() + START_DATETIME;
            unsigned long fileUnixTimeLocal = gmtTimeToLocal(fileUnixTimeGMT);

            unsigned long reqUnixTime = strDateToUnix(getItemValue(id + "-date"));
            if (fileUnixTimeLocal > reqUnixTime && fileUnixTimeLocal < reqUnixTime + 86400) {
                noData = false;
                createJson(buf, i);
                SerialPrint("i", F("Loging"), String(f) + ") " + buf + ", " + String(i) + ", " + getDateTimeDotFormatedFromUnix(fileUnixTimeLocal) + ", sent");
            } else {
                SerialPrint("i", F("Loging"), String(f) + ") " + buf + ", nil, " + getDateTimeDotFormatedFromUnix(fileUnixTimeLocal) + ", skipped");
            }

            filesList = deleteBeforeDelimiter(filesList, ";");
        }
        //если данных нет отправляем пустой грфик
        if (noData) {
            cleanChart();
        }
    }

    void cleanChart() {
        SerialPrint("i", F("Loging"), "clear chart");
        String cleanJson = createEmtyJson();
        publishJson(cleanJson);
    }

    void cleanData() {
        String dir = "lg/" + id;
        filesList = getFilesList(dir);
        int i = 0;
        while (filesList.length()) {
            String buf = selectToMarker(filesList, ";");

            i++;
            removeFile(buf);
            SerialPrint("i", "Files", String(i) + ") " + buf + " => deleted");

            filesList = deleteBeforeDelimiter(filesList, ";");
        }
    }

    void deleteLastFile() {
        IoTFSInfo tmp = getFSInfo();
        SerialPrint("i", "Loging", String(tmp.freePer) + " % free flash remaining");
        if (tmp.freePer <= 10.00) {
            String dir = "lg/" + id;
            filesList = getFilesList(dir);
            int i = 0;
            while (filesList.length()) {
                String buf = selectToMarker(filesList, ";");

                i++;
                if (i == 1) {
                    removeFile(buf);
                    SerialPrint("!", "Loging", String(i) + ") " + buf + " => oldest files been deleted");
                    return;
                }

                filesList = deleteBeforeDelimiter(filesList, ";");
            }
        }
    }

    void createJson(String file, int &i) {
        File configFile = FileFS.open(file, "r");
        if (!configFile) {
            SerialPrint("E", F("Loging"), "'" + id + "' open file error");
            return;
        }
        configFile.seek(0, SeekSet);
        String buf = "{}";
        String oneSingleJson;
        String unix_time;
        String value;
        unsigned int psn;
        unsigned int sz = configFile.size();
        do {
            i++;
            psn = configFile.position();
            String line = configFile.readStringUntil('\n');
            unix_time = selectToMarker(line, " ");
            jsonWriteInt(buf, "x", unix_time.toInt() + START_DATETIME);
            value = deleteBeforeDelimiter(line, " ");
            jsonWriteFloat(buf, "y1", value.toFloat());
            if (unix_time != "" || value != "") {
                oneSingleJson += buf + ",";
            }
        } while (psn < sz);

        configFile.close();

        String topic = mqttRootDevice + "/" + id;
        oneSingleJson = "{\"maxCount\":" + String(calculateMaxCount()) + ",\"topic\":\"" + topic + "\",\"status\":[" + oneSingleJson + "]}";
        oneSingleJson.replace("},]}", "}]}");

        publishJson(oneSingleJson);
    }

    // publishType 1 - в mqtt, 2 - в ws, 3 - mqtt и ws
    // wsNum = -1 => broadcast
    void setPublishType(int publishType, int wsNum) {
        _publishType = publishType;
        _wsNum = wsNum;
    }

    void publishJson(String &oneSingleJson) {
        if (_publishType == 1) {
            publishChartMqtt(id, oneSingleJson);
        } else if (_publishType == 2) {
            publishChartWs(_wsNum, oneSingleJson);
        } else if (_publishType == 3) {
            publishChartMqtt(id, oneSingleJson);
            publishChartWs(_wsNum, oneSingleJson);
        } else {
            SerialPrint("E", F("Loging"), "wrong publishType");
        }
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
        String currentDate = getDateDotFormated();
        //отправляем в график данные только когда выбран сегодняшний день
        if (userDate == currentDate) {
            generateEvent(_id, value);
            publishStatusMqtt(_id, value);
            String json = createSingleJson(_id, value);
            publishChartWs(-1, json);
            SerialPrint("i", "Sensor " + consoleInfo, "'" + _id + "' data: " + value + "'");
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
                    (*it)->setPublishType(3, -1);
                    (*it)->cleanChart();
                    (*it)->sendChart();
                }
            }
        }
    }

    void doByInterval() {
        if (isTimeSynch) {
            if (firstTime) {
                setValue(getDateDotFormated());
                SerialPrint("E", F("Loging"), "today date set " + getDateDotFormated());
                firstTime = false;
            }
        }
    }
};

void *getAPI_Date(String param) {
    return new Date(param);
}
