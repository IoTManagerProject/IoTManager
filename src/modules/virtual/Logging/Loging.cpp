#include "Global.h"
#include "classes/IoTItem.h"
#include "NTP.h"

#define SOLID_UPLOADING

class Loging : public IoTItem {
   private:
    String logval;
    String id;
    int points;
    String fileName = "";

   public:
    Loging(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, F("logid"), logval);
        jsonRead(parameters, F("id"), id);
        jsonRead(parameters, F("points"), points);
    }

    void setValue(IoTValue Value) {
        value = Value;
        regEvent((String)(int)value.valD, "VButton");
    }

    String getValue() {
        return "";
        // return (String)(int)value.valD;
    }

    void doByInterval() {
        String value = getItemValue(logval);
        size_t lines = 0;
        //если задано не правильное logid величины для логгирования
        if (value == "") {
            SerialPrint("E", F("Loging"), F("no value set"));
        } else {
            //если время было получено из интернета
            if (isTimeSynch) {
                // regEvent(value, "Loging");
                String logData = String(unixTimeShort) + " " + value;

                static bool firstTime = true;
                //если зашли сюда первый раз то создаем файл с именем id-timestamp
                if (firstTime) {
                    firstTime = false;
                    createFileWithIdDatatimeName(logData);
                    //если 2 ой раз и последующие разы
                } else {
                    lines = countLines(fileName);
                    if (lines <= 100) {
                        addFileLn(fileName, logData);
                    } else {
                        createFileWithIdDatatimeName(logData);
                    }
                }
                SerialPrint("i", F("Loging"), "loging in file (" + String(lines) + ") http://" + WiFi.localIP().toString() + fileName);

            } else {
                SerialPrint("E", F("Loging"), F("Сant loging - time not synchronized"));
            }
        }
    }

    void createFileWithIdDatatimeName(String &logData) {
        fileName = "/logs/" + id + "-" + String(unixTimeShort) + ".txt";
        addFileLn(fileName, logData);
        SerialPrint("i", F("Loging"), "file created http://" + WiFi.localIP().toString() + fileName);
    }

    void sendChart() {
        String reqUnixTimeStr = "25.08.2022";  //нужно получить эту дату из окна ввода под графиком.
        unsigned long reqUnixTime = strDateToUnix(reqUnixTimeStr);

        String directory = "logs";
        SerialPrint("I", "Loging", "in directory '" + directory + "' files:");
        auto dir = FileFS.openDir(directory);
        String oneSingleJson;
        int maxCount = 0;

        while (dir.next()) {
            String fname = dir.fileName();
            String id = selectToMarker(fname, "-");
            unsigned long fileUnixTime = deleteBeforeDelimiter(deleteToMarkerLast(fname, "."), "-").toInt() + START_DATETIME;

            SerialPrint("I", "Loging", "found file '" + fname + "'");

            if (isItemExist(id)) {
                //выбираем только те файлы которые входят в выбранные сутки
                if (fileUnixTime > reqUnixTime && fileUnixTime < reqUnixTime + 86400) {
                    Serial.println("matched!");
#ifdef SOLID_UPLOADING
                    createOneSingleJson(oneSingleJson, "/logs/" + fname, maxCount);
#else

#endif
                }
            } else {
                SerialPrint("i", "Loging", "file '" + fname + "' not used, deleted");
                removeFile(directory + "/" + fname);
            }
        }
#ifdef SOLID_UPLOADING
        sendJson(oneSingleJson, maxCount);
#endif
    }

    void createOneSingleJson(String &oneSingleJson, String file, int &maxCount) {
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

    void sendJson(String &oneSingleJson, int &maxCount) {
        oneSingleJson = "{\"maxCount\":" + String(maxCount) + ",\"status\":[" + oneSingleJson + "]}";
        oneSingleJson.replace("},]}", "}]}");
        Serial.println("final: ");
        Serial.println(oneSingleJson);
        publishChart(id, oneSingleJson);
    }

    void cleanLog() {
        String directory = "logs/" + id;
        auto dir = FileFS.openDir(directory);
        while (dir.next()) {
            String fname = dir.fileName();
            removeFile(directory + "/" + fname);
            SerialPrint("I", "Loging", fname + " deleted");
        }
    }
};

void *getAPI_Loging(String subtype, String param) {
    if (subtype == F("Loging")) {
        return new Loging(param);
    } else {
        return nullptr;
    }
}
