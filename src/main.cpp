#include "main.h"

#include "rest.h"

void setup() {
    Serial.begin(115200);
    Serial.flush();
    Serial.println();
    Serial.println(F("--------------started----------------"));
    fileSystemInit();

    Serial.println("before " + prettyBytes(ESP.getFreeHeap()));
    setupESP();
    Serial.println("after " + prettyBytes(ESP.getFreeHeap()));
}

void loop() {
}

void setupESP() {
    File file1 = seekFile("/setup.json");              //читаем первый файл из памяти стримом
    File file2 = FileFS.open("/setup.json.tmp", "w");  //открыл второй файл для записи
    file2.println("[");

    // WriteBufferingStream bfile2(file2, 64);  //записываем стрим во второй файл для записи
    // ReadBufferingStream bfile1{file1, 64};   //стримим первый файл

    DynamicJsonDocument doc(1024);
    Serial.println("during " + prettyBytes(ESP.getFreeHeap()));
    int i = 0;

    file1.find("[");

    do {
        i++;

        deserializeJson(doc, file1);
        doc["web"]["order"] = i;
        serializeJsonPretty(doc, file2);

        file2.println(",");

        // DeserializationError error =
        // if (error) {
        //     Serial.print("json error: ");
        //     Serial.println(error.f_str());
        // }

        Serial.println(
            String(i) + ") " +
            doc["type"].as<String>() + " " +
            doc["set"]["gpio"].as<String>() + " " +
            doc["web"]["order"].as<String>());

    } while (file1.findUntil(",", "]"));

    file2.println("]");

    file2.close();

    // if (cutFile("/setup.json.tmp", "/setup.json")) Serial.println("file overwrited");

    Serial.println("-------------");
    Serial.println(readFile("/setup.json.tmp", 20000));
    Serial.println("-------------");
}
