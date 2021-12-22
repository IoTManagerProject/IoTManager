#include "Main.h"

void setup() {
    Serial.begin(115200);
    Serial.flush();
    Serial.println();
    Serial.println(F("--------------started----------------"));

    //инициализация файловой системы
    fileSystemInit();

    //синхронизация глобальных переменных с flash
    globalVarsSync();

    //подключаемся к роутеру
    routerConnect();

    //выводим остаток оперативной памяти после старта
    // 22.12.21 пустой код без wifi остаток = 50.28 kB
    SerialPrint(F("i"), F("HEAP"), prettyBytes(ESP.getFreeHeap()));
}

void loop() {
}
