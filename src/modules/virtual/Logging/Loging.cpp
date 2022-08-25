#include "Global.h"
#include "classes/IoTItem.h"

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
                    if (lines <= 255) {
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
