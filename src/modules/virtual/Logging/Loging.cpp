#include "Global.h"
#include "classes/IoTItem.h"
#include "NTP.h"

class Loging : public IoTItem {
   private:
    String logid;
    String id;
    int points;

    int interval;
    bool firstTime = true;

   public:
    Loging(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, F("logid"), logid);
        jsonRead(parameters, F("id"), id);
        jsonRead(parameters, F("points"), points);
        jsonRead(parameters, F("int"), interval);
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

        int lines = 0;
        String filePath = "";

        // regEvent(value, F("Loging"));

        String logData = String(unixTimeShort) + " " + value;

        //прочитаем путь к файлу последнего сохранения
        filePath = readDataDB(id);
        //если данные о файле отсутствуют, создадим новый
        if (filePath == "failed") {
            SerialPrint("E", F("Loging"), "'" + id + "' file path not found");
            createNewFileWithData(logData);
            return;
        }

        //считаем количество строк
        lines = countLines(filePath);
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
        String filePath = "/lg/" + id + "-" + String(unixTimeShort) + ".txt";  //создадим путь
        addFileLn(filePath, logData);                                          //запишем файл и данные в него
        saveDataDB(id, filePath);                                              //запишем путь к файлу в базу данных
        SerialPrint("i", F("Loging"), "'" + id + "' file created http://" + WiFi.localIP().toString() + filePath);
    }

    void addNewDataToExistingFile(String &filePath, String &logData) {
        addFileLn(filePath, logData);
        SerialPrint("i", F("Loging"), "'" + id + "' loging in file http://" + WiFi.localIP().toString() + filePath);
    }

    void sendChart() {
        SerialPrint("i", F("Loging"), "'" + id + "'----------------------------");
        String reqUnixTimeStr = "26.08.2022";  //нужно получить эту дату из окна ввода под графиком.
        unsigned long reqUnixTime = strDateToUnix(reqUnixTimeStr);

        String directory = "lg";
        SerialPrint("i", F("Loging"), "'" + id + "' in directory '" + directory + "' files:");
        auto dir = FileFS.openDir(directory);
        String oneSingleJson;
        int i = 0;

        while (dir.next()) {
            String fname = dir.fileName();
            String idInFileName = selectToMarker(fname, "-");
            unsigned long fileUnixTime = deleteBeforeDelimiter(deleteToMarkerLast(fname, "."), "-").toInt() + START_DATETIME;
            if (isItemExist(id)) {
                //если id в имени файла совпадает с id данного экземпляра, пусть каждый экземпляр класса шлет только свое
                if (idInFileName == id) {
                    //выбираем только те файлы которые входят в выбранные пользователем сутки
                    if (fileUnixTime > reqUnixTime && fileUnixTime < reqUnixTime + 86400) {
                        SerialPrint("i", F("Loging"), "'" + id + "' matching file found '" + fname + "'");
                        //выгрузка по частям, по одному файлу
                        publishJsonPartly("/lg/" + fname, calculateMaxCount(), i);
                    }
                }
            } else {
                SerialPrint("i", F("Loging"), "'" + id + "' file '" + fname + "' not used, deleted");
                removeFile(directory + "/" + fname);
            }
        }
        SerialPrint("i", F("Loging"), "'" + id + "'--------------'" + String(i) + "'--------------");
    }

    void publishJsonPartly(String file, int maxCount, int &i) {
        File configFile = FileFS.open(file, "r");
        if (!configFile) {
            return;
        }
        configFile.seek(0, SeekSet);
        String buf = "{}";
        String dividedJson;
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
                dividedJson += buf + ",";
            }
        } while (psn < sz);

        configFile.close();
        publishJson(dividedJson, maxCount);
    }

    void publishJson(String &oneSingleJson, int &maxCount) {
        oneSingleJson = "{\"maxCount\":" + String(maxCount) + ",\"status\":[" + oneSingleJson + "]}";
        oneSingleJson.replace("},]}", "}]}");
        publishChart(id, oneSingleJson);
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
