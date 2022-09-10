#include "Global.h"
#include "classes/IoTItem.h"
#include "ESPConfiguration.h"
#include "NTP.h"

void *getAPI_Date(String params);

String date;
class Loging : public IoTItem {
   private:
    String logid;
    String id;
    String filesList = "";

    bool firstTime = true;

    int points;
    int keepdays;

    IoTItem *dateIoTItem;

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
        dateIoTItem = (IoTItem *)getAPI_Date("{\"id\": \"" + id + "-date\"}");
        IoTItems.push_back(dateIoTItem);
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
        generateEvent(_id, value);
        publishStatusMqtt(_id, value);
        String json = createSingleJson(_id, value);
        publishStatusWsJson(json);
        SerialPrint("i", "Sensor " + consoleInfo, "'" + _id + "' data: " + value + "'");
    }

    String createSingleJson(String id, String value) {
        String topic = mqttRootDevice + "/" + _id;
        return "{\"topic\":\"" + topic + "\",\"status\":[{\"x\":" + String(unixTime) + ",\"y1\":" + value + "}]}";
    }

    void doByInterval() {
        if (firstTime) {
            firstTime = false;
        }

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
        } else {
            // SerialPrint("i", F("Loging"), "'" + id + "' file path found " + filePath);
        }

        //считаем количество строк
        int lines = countLines(filePath);
        SerialPrint("i", F("Loging"), "'" + id + "' " + String(lines) + " lines found in file");

        //если количество строк до заданной величины
        if (lines <= points) {
            //просто добавим в существующий файл новые данные
            addNewDataToExistingFile(filePath, logData);
            //если больше создадим следующий файл
        } else {
            createNewFileWithData(logData);
        }
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

#if defined(ESP8266)
    void getFilesList8266() {
        filesList = "";
        String directory = "lg/" + id;
        auto dir = FileFS.openDir(directory);
        while (dir.next()) {
            String fname = dir.fileName();
            if (fname != "") filesList += directory + "/" + fname + ";";
        }
    }
#endif

#if defined(ESP32)
    void getFilesList32() {
        filesList = "";
        String directory = "/lg/" + id;
        File root = FileFS.open(directory);
        directory = String();
        if (root.isDirectory()) {
            File file = root.openNextFile();
            while (file) {
                String fname = file.name();
                if (fname != "") filesList += fname + ";";
                file = root.openNextFile();
            }
        }
    }
#endif

    void getFilesList() {
#if defined(ESP8266)
        getFilesList8266();
#endif
#if defined(ESP32)
        getFilesList32();
#endif
    }

    void sendChart(bool mqtt) {
        getFilesList();
        int f = 0;

        while (filesList.length()) {
            String buf = selectToMarker(filesList, ";");

            f++;
            int i = 0;

            unsigned long fileUnixTime = selectToMarkerLast(deleteToMarkerLast(buf, "."), "/").toInt() + START_DATETIME;

            //удаление старых файлов
            if ((fileUnixTime + (points * (interval / 1000))) < (unixTime - (keepdays * 86400))) {
                SerialPrint("i", F("Loging"), "file '" + buf + "' too old, deleted");
                removeFile(buf);
            } else {
                unsigned long reqUnixTime = strDateToUnix(date);
                if (fileUnixTime > reqUnixTime && fileUnixTime < reqUnixTime + 86400) {
                    createJson(buf, i, mqtt);
                    SerialPrint("i", F("Loging"), "file '" + buf + "' sent, user requested " + date);
                } else {
                    SerialPrint("i", F("Loging"), "file '" + buf + "' skipped, user requested " + date);
                }
            }

            SerialPrint("i", F("Loging"), String(f) + ") path: " + buf + ", lines №: " + String(i) + ", created: " + getDateTimeDotFormatedFromUnix(fileUnixTime));
            filesList = deleteBeforeDelimiter(filesList, ";");
        }
    }

    void cleanData() {
        getFilesList();
        int i = 0;
        while (filesList.length()) {
            String buf = selectToMarker(filesList, ";");

            i++;
            removeFile(buf);
            SerialPrint("i", "Files", String(i) + ") " + buf + " => deleted");

            filesList = deleteBeforeDelimiter(filesList, ";");
        }
    }

    //    void sendChart2(bool mqtt) {
    //        //отправка графика может происходить только если время синхронизированно
    //        if (!isTimeSynch) {
    //            SerialPrint("E", F("Loging"), "'" + id + "' Сant send chart - time not synchronized");
    //            return;
    //        }
    //        SerialPrint("i", F("Loging"), "'" + id + "'----------------------------");
    //        String reqUnixTimeStr = "27.08.2022";  //нужно получить эту дату из окна ввода под графиком.
    //        unsigned long reqUnixTime = strDateToUnix(reqUnixTimeStr);
    //
    //        int i = 0;
    //#if defined(ESP8266)
    //        String directory = "lg";
    //        auto dir = FileFS.openDir(directory);
    //        while (dir.next()) {
    //            String fname = dir.fileName();
    //#endif
    //#if defined(ESP32)
    //            String directory = "/lg";
    //            File root = FileFS.open(directory);
    //            directory = String();
    //            if (root.isDirectory()) {
    //                File file = root.openNextFile();
    //                while (file) {
    //                    String fname = file.name();
    //                    fname = selectToMarkerLast(fname, "/");
    //                    file = root.openNextFile();
    //#endif
    //                    String idInFileName = selectToMarker(fname, "-");
    //                    unsigned long fileUnixTime = deleteBeforeDelimiter(deleteToMarkerLast(fname, "."), "-").toInt() + START_DATETIME;
    //                    if (isItemExist(id)) {
    //                        //если id в имени файла совпадает с id данного экземпляра, пусть каждый экземпляр класса шлет только свое
    //                        if (idInFileName == id) {
    //                            //выбираем только те файлы которые входят в выбранные пользователем сутки
    //                            // if (fileUnixTime > reqUnixTime && fileUnixTime < reqUnixTime + 86400) {
    //                            SerialPrint("i", F("Loging"), "'" + id + "' matching file found '" + fname + "'");
    //                            //выгрузка по частям, по одному файлу
    //                            createJson("/lg/" + fname, i, mqtt);
    //                            //}
    //                            //удаление старых файлов
    //                            if ((fileUnixTime + (points * interval)) < (unixTime - (keepdays * 86400))) {
    //                                SerialPrint("i", F("Loging"), "'" + id + "' file '" + fname + "' too old, deleted");
    //                                removeFile(directory + "/" + fname);
    //                            }
    //                        }
    //                    } else {
    //                        SerialPrint("i", F("Loging"), "'" + id + "' file '" + fname + "' not used, deleted");
    //                        removeFile(directory + "/" + fname);
    //                    }
    //                }
    //#if defined(ESP32)
    //            }
    //#endif
    //
    //            SerialPrint("i", F("Loging"), "'" + id + "'--------------'" + String(i) + "'--------------");
    //        }

    void createJson(String file, int &i, bool mqtt) {
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

        publishJson(oneSingleJson, mqtt);
    }

    void publishJson(String &oneSingleJson, bool mqtt) {
        if (mqtt) {
            publishChart(id, oneSingleJson);
        } else {
            publishStatusWsJson(oneSingleJson);
        }
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
   public:
    String id;
    Date(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, F("id"), id);
        value.isDecimal = false;
    }

    void setValue(String valStr) {
        value.valS = valStr;
        date = valStr;
        setValue(value);
    }

    void setValue(IoTValue Value) {
        value = Value;
        regEvent(value.valS, "");
        for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
            if ((*it)->getSubtype() == "Loging") {
                (*it)->sendChart(true);
            }
        }
    }

    void doByInterval() {}
};

void *getAPI_Date(String param) {
    return new Date(param);
}
