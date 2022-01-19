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

    //создаем объект класса выгружающего json массив из файла
    sendConfigJson = new SendJson;
    sendWigdetsJson = new SendJson;

    // myBuf = new CommandBuf;
    //
    // myBuf->addCommand("zero");
    // myBuf->addCommand("one");
    // myBuf->addCommand("two");
    // myBuf->printCommands();
    //
    // myBuf->getLastCommand();
    // myBuf->getLastCommand();
    // myBuf->getLastCommand();
    // myBuf->printCommands();

    myQueue = new QueueBuf;

    myQueue->push(*new QueueInstance("text1"));
    myQueue->push(*new QueueInstance("text2"));
    myQueue->push(*new QueueInstance("text3"));

    Serial.println(myQueue->front().get());
    Serial.println(myQueue->front().get());
    Serial.println(myQueue->front().get());

    // myQueue->push(20);
    // myQueue->push(30);
    //
    // Serial.println(myQueue->front());
    // Serial.println(myQueue->front());
    // Serial.println(myQueue->front());

    configure("/config.json");

    //выводим остаток оперативной памяти после старта
    // 22.12.21 пустой код без wifi остаток    = 50.28 kB
    // 22.12.21 запустил wifi       остаток    = 48.59 kB
    // 22.12.21 добавил асинхронный веб сервер = 38.36 kB
    // 22.12.21 добавил web sockets            = 37.63 kB

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
    if (sendConfigJson) sendConfigJson->loop();
    if (sendWigdetsJson) sendWigdetsJson->loop();

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
