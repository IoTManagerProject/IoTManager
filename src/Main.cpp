#include "Main.h"

void setup() {
    Serial.begin(115200);
    Serial.flush();
    Serial.println();
    Serial.println(F("--------------started----------------"));

    //создание экземпляров классов
    myNotAsyncActions = new NotAsync(do_LAST);

    //инициализация файловой системы
    fileSystemInit();

    //синхронизация глобальных переменных с flash
    globalVarsSync();

    //подключаемся к роутеру
    routerConnect();

//инициализация асинхронного веб сервера и веб сокетов
#ifdef ASYNC_WEB_SERVER
    asyncWebServerInit();
#endif
#ifdef ASYNC_WEB_SOCKETS
    asyncWebSocketsInit();
#endif

//инициализация стандартного веб сервера и веб сокетов
#ifdef STANDARD_WEB_SERVER
    standWebServerInit();
#endif
#ifdef STANDARD_WEB_SOCKETS
    standWebSocketsInit();
#endif

    //инициализация mqtt
    mqttInit();

    configure("/config.json");

    // readFileBin("/config.json");

    //создали задачу которая будет выполняться каждые 30 секунд
    ts.add(
        MYTEST, 1000 * 30, [&](void*) {
            SerialPrint(F("i"), F("HEAP"), prettyBytes(ESP.getFreeHeap()));
        },
        nullptr, true);
}

void loop() {
    //обновление задач таскера
    ts.update();

//отправка json
#ifdef QUEUE_FROM_STR
    if (sendJsonFiles) sendJsonFiles->loop();
#endif

#ifdef STANDARD_WEB_SERVER
    //обработка web сервера
    HTTP.handleClient();
#endif

#ifdef STANDARD_WEB_SOCKETS
    //обработка web сокетов
    standWebSocket.loop();
#endif

    //обновление mqtt
    mqttLoop();

    for (unsigned int i = 0; i < iotSensors.size(); i++) {
        iotSensors[i]->loop();
    }
}
