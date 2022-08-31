#include "Global.h"
#include "classes/IoTItem.h"
#include "NTP.h"

class Loging : public IoTItem {
   private:
    String logid;
    String id;
    int points;
    int keepdays;

    int interval;
    bool firstTime = true;

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
        jsonRead(parameters, F("keepdays"), keepdays);
    }

    String getValue() {
        return "";
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

        // regEvent(value, F("Loging"));

        String logData = String(unixTimeShort) + " " + value;

        //прочитаем путь к файлу последнего сохранения
        String filePath = readDataDB(id);

        // Serial.println("filePath " + filePath);

        //если данные о файле отсутствуют, создадим новый
        if (filePath == "failed" || filePath == "") {
            SerialPrint("E", F("Loging"), "'" + id + "' file path not found");
            createNewFileWithData(logData);
            return;
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
        String path = "/lg/" + id + "-" + String(unixTimeShort) + ".txt";  //создадим путь
        addFileLn(path, logData);                                          //запишем файл и данные в него
        saveDataDB(id, path);                                              //запишем путь к файлу в базу данных
        SerialPrint("i", F("Loging"), "'" + id + "' file created http://" + WiFi.localIP().toString() + path);
    }

    void addNewDataToExistingFile(String &path, String &logData) {
        addFileLn(path, logData);
        SerialPrint("i", F("Loging"), "'" + id + "' loging in file http://" + WiFi.localIP().toString() + path);
    }

    void sendChart(bool mqtt) {
        SerialPrint("i", F("Loging"), "'" + id + "'----------------------------");
        String reqUnixTimeStr = "27.08.2022";  //нужно получить эту дату из окна ввода под графиком.
        unsigned long reqUnixTime = strDateToUnix(reqUnixTimeStr);

        int i = 0;
#if defined(ESP8266)
        String directory = "lg";
        auto dir = FileFS.openDir(directory);
        while (dir.next()) {
            String fname = dir.fileName();
#endif
#if defined(ESP32)
            String directory = "/lg";
            File root = FileFS.open(directory);
            directory = String();
            if (root.isDirectory()) {
                File file = root.openNextFile();
                while (file) {
                    String fname = file.name();
                    fname = selectToMarkerLast(fname, "/");
                    file = root.openNextFile();
#endif
                    String idInFileName = selectToMarker(fname, "-");
                    unsigned long fileUnixTime = deleteBeforeDelimiter(deleteToMarkerLast(fname, "."), "-").toInt() + START_DATETIME;
                    if (isItemExist(id)) {
                        //если id в имени файла совпадает с id данного экземпляра, пусть каждый экземпляр класса шлет только свое
                        if (idInFileName == id) {
                            //выбираем только те файлы которые входят в выбранные пользователем сутки
                            // if (fileUnixTime > reqUnixTime && fileUnixTime < reqUnixTime + 86400) {
                            SerialPrint("i", F("Loging"), "'" + id + "' matching file found '" + fname + "'");
                            //выгрузка по частям, по одному файлу
                            createJson("/lg/" + fname, i, mqtt);
                            //}
                            //удаление старых файлов
                            if ((fileUnixTime + (points * interval)) < (unixTime - (keepdays * 86400))) {
                                SerialPrint("i", F("Loging"), "'" + id + "' file '" + fname + "' too old, deleted");
                                removeFile(directory + "/" + fname);
                            }
                        }
                    } else {
                        SerialPrint("i", F("Loging"), "'" + id + "' file '" + fname + "' not used, deleted");
                        removeFile(directory + "/" + fname);
                    }
                }
#if defined(ESP32)
            }
#endif

            SerialPrint("i", F("Loging"), "'" + id + "'--------------'" + String(i) + "'--------------");
        }

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

        void publishJson(String & oneSingleJson, bool mqtt) {
            if (mqtt) {
                publishChart(id, oneSingleJson);
            } else {
                publishStatusWsJson(oneSingleJson);
            }
        }

        //примерный подсчет максимального количества точек
        int calculateMaxCount() {
            return 86400 / interval;
        }
    };

    void *getAPI_Loging(String subtype, String param) {
        if (subtype == F("Loging")) {
            return new Loging(param);
        } else {
            return nullptr;
        }
    }

    //то что не пригодилось но пригодится потом может быть
    // void createOneSingleJson(String &oneSingleJson, String file, int &maxCount, int &i) {
    //         File configFile = FileFS.open(file, "r");
    //         if (!configFile) {
    //             return;
    //         }
    //         configFile.seek(0, SeekSet);
    //         String buf = "{}";
    //         String unix_time;
    //         String value;
    //         unsigned int psn;
    //         unsigned int sz = configFile.size();
    //         do {
    //             maxCount++;
    //             i++;
    //             psn = configFile.position();
    //             String line = configFile.readStringUntil('\n');
    //             unix_time = selectToMarker(line, " ");
    //             jsonWriteInt(buf, "x", unix_time.toInt() + START_DATETIME);
    //             value = deleteBeforeDelimiter(line, " ");
    //             jsonWriteFloat(buf, "y1", value.toFloat());
    //             if (unix_time != "" || value != "") {
    //                 oneSingleJson += buf + ",";
    //             }
    //
    //         } while (psn < sz);
    //
    //         configFile.close();
    //     }
