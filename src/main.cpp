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

    //инициализация асинхронного веб сервера
    webServerInit();

    //инициализация веб сокетов асинхронного веб сервера
    webSocketsInit();

    //подключаемся к роутеру
    routerConnect();

    //выводим остаток оперативной памяти после старта
    // 22.12.21 пустой код без wifi остаток    = 50.28 kB
    // 22.12.21 запустил wifi       остаток    = 48.59 kB
    // 22.12.21 добавил асинхронный веб сервер = 38.36 kB
    // 22.12.21 добавил web sockets            = 37.63 kB
    SerialPrint(F("i"), F("HEAP"), prettyBytes(ESP.getFreeHeap()));
}

void loop() {
}
