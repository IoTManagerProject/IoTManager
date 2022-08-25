#include "Global.h"
#include "classes/IoTItem.h"
#include "NTP.h"

//раскомментировать если нужна цельная выгрузка графиков
//#define SOLID_UPLOADING

class Loging : public IoTItem {
   private:
    String logid;
    String id;
    int points;
    String fileName = "";
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
        if (!isItemExist(logid)) {
            SerialPrint("E", "Loging " + id, F("logging value not exist"));
            return;
        }
        String value = getItemValue(logid);
        int lines = 0;
        //если задано не правильное logid величины для логгирования
        if (value == "") {
            SerialPrint("E", "Loging " + id, F("logging value empty"));
            return;
        } else {
            //если время было получено из интернета
            if (isTimeSynch) {
                // regEvent(value, "Loging");
                String logData = String(unixTimeShort) + " " + value;
                //если зашли сюда первый раз то создаем файл с именем id-timestamp
                if (firstTime) {
                    firstTime = false;
                    createFileWithIdDatatimeName(logData);
                    //если 2 ой раз и последующие разы
                } else {
                    lines = countLines(fileName);
                    if (lines <= points) {
                        addFileLn(fileName, logData);
                    } else {
                        createFileWithIdDatatimeName(logData);
                    }
                }
                SerialPrint("i", "Loging " + id, "loging in file (" + String(lines) + ") http://" + WiFi.localIP().toString() + fileName);

            } else {
                SerialPrint("E", "Loging " + id, F("Сant loging - time not synchronized"));
            }
        }
    }

    void createFileWithIdDatatimeName(String &logData) {
        fileName = "/logs/" + id + "-" + String(unixTimeShort) + ".txt";
        addFileLn(fileName, logData);
        SerialPrint("i", "Loging " + id, "file created http://" + WiFi.localIP().toString() + fileName);
    }

    void sendChart() {
        SerialPrint("I", "Loging " + id, "----------------------------");
        String reqUnixTimeStr = "25.08.2022";  //нужно получить эту дату из окна ввода под графиком.
        unsigned long reqUnixTime = strDateToUnix(reqUnixTimeStr);

        String directory = "logs";
        SerialPrint("I", "Loging " + id, "in directory '" + directory + "' files:");
        auto dir = FileFS.openDir(directory);
        String oneSingleJson;
        int maxCount = 0;
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
                        SerialPrint("I", "Loging " + id, "matching file found '" + fname + "'");
#ifdef SOLID_UPLOADING
                        //если активированна выгрузка целеком
                        createOneSingleJson(oneSingleJson, "/logs/" + fname, maxCount, i);
#else
                        //выгрузка по частям, по одному файлу
                        publishJsonPartly("/logs/" + fname, calculateMaxCount(), i);
#endif
                    }
                }
            } else {
                SerialPrint("i", "Loging " + id, "file '" + fname + "' not used, deleted");
                removeFile(directory + "/" + fname);
            }
        }
#ifdef SOLID_UPLOADING
        publishJson(oneSingleJson, maxCount);
#endif
        SerialPrint("I", "Loging " + id, "--------------'" + String(i) + "'--------------");
    }

    void createOneSingleJson(String &oneSingleJson, String file, int &maxCount, int &i) {
        File configFile = FileFS.open(file, "r");
        if (!configFile) {
            return;
        }
        configFile.seek(0, SeekSet);
        String buf = "{}";
        String unix_time;
        String value;
        unsigned int psn;
        unsigned int sz = configFile.size();
        do {
            maxCount++;
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
        // SerialPrint("i", "Loging " + id, "publish chart, maxCount: '" + String(maxCount) + "'");
        publishChart(id, oneSingleJson);
    }

    void cleanLog() {
        String directory = "logs/" + id;
        auto dir = FileFS.openDir(directory);
        while (dir.next()) {
            String fname = dir.fileName();
            removeFile(directory + "/" + fname);
            SerialPrint("I", "Loging " + id, fname + " deleted");
        }
    }

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
