#include "Main.h"

IoTScenario iotScen;  // объект управления сценарием

void setup() {
    Serial.begin(115200);
    Serial.flush();
    Serial.println();
    Serial.println(F("--------------started----------------"));

    //создание экземпляров классов
    myNotAsyncActions = new NotAsync(do_LAST);

    //инициализация файловой системы
    fileSystemInit();

    //получение chip id
    setChipId();

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

    //настраиваем микроконтроллер
    configure("/config.json");

    //инициализация задач переодического выполнения
    periodicTasksInit();

    //синхронизация списка устройств
    addThisDeviceToList();

    //запуск работы udp
    asyncUdpInit();

    //загрузка сценария
    iotScen.loadScenario("/scenario.txt");
    iotScen.ExecScenario();

    // test
    Serial.println("-------test start--------");
    Serial.println("--------test end---------");
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

    for (unsigned int i = 0; i < IoTItems.size(); i++) {
        IoTItems[i]->loop();
    }

    // iotScen.ExecScenario();
}
