#include "Main.h"

void setup() {
    Serial.begin(115200);
    Serial.flush();
    Serial.println();
    Serial.println(F("--------------started----------------"));

    //инициализация файловой системы
    fileSystemInit();

    //выводим остаток оперативной памяти после старта (пустой код без wifi остаток = 50.28 kB)
    SerialPrint(F("i"), F("HEAP"), prettyBytes(ESP.getFreeHeap()));
}

void loop() {
}
