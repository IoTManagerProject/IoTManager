#include "Main.h"
#include <time.h>

IoTScenario iotScen;  // объект управления сценарием

String volStrForSave = "";
unsigned long currentMillis;
unsigned long prevMillis;

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

    // NTP
    ntpInit();

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
    iotScen.loadScenario("/scenario.json");
    // создаем событие завершения конфигурирования для возможности выполнения блока кода при загрузке
    IoTItems.push_back((IoTItem *)new externalVariable("{\"id\":\"onStart\",\"val\":1,\"int\":60}"));
    generateEvent("onStart", "");

    // test
    Serial.println("-------test start--------");
    Serial.println("--------test end---------");

    // симуляция добавления внешних событий
    // IoTItems.push_back((IoTItem*)new externalVariable("{\"id\":\"rel1\",\"val\":10,\"int\":20}"));
    // IoTItems.push_back((IoTItem*)new externalVariable("{\"id\":\"rel4\",\"val\":34,\"int\":30}"));
    // пример получения JSON всех Items
    // Serial.println(getParamsJson());
    //чтение одного параметра
    // Serial.println(findIoTItem("t1")->getValue());
    //тест перебора пинов из расширения
    // for (int i = 109; i < 112; i++) {
    //     IoTgpio.pinMode(i, OUTPUT);
    //     IoTgpio.digitalWrite(i, !IoTgpio.digitalRead(i));
    //     delay(1000);
    //     IoTgpio.digitalWrite(i, !IoTgpio.digitalRead(i));
    //     delay(1000);
    // }
}

void loop() {
    // if(millis()%2000==0){
    //     //watch->settimeUnix(time(&iotTimeNow));
    //     Serial.println(watch->gettime("d-m-Y, H:i:s, M"));
    //     delay(1);
    // }

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

    // передаем управление каждому элементу конфигурации для выполнения своих функций
    for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
        (*it)->loop();
        if ((*it)->iAmDead) {
            delete *it;
            IoTItems.erase(it);
            break;
        }
    }

    handleOrder();

    handleEvent();

    // сохраняем значения IoTItems в файл каждую секунду, если были изменения (установлены маркеры на сохранение)
    // currentMillis = millis();
    // if (currentMillis - prevMillis >= 1000) {
    //    prevMillis = millis();
    //    volStrForSave = "";
    //    for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
    //        if ((*it)->needSave) {
    //            (*it)->needSave = false;
    //            volStrForSave = volStrForSave + (*it)->getID() + "=" + (*it)->getValue() + ";";
    //        }
    //    }
    //
    //    if (volStrForSave != "") {
    //        Serial.print("volStrForSave: ");
    //        Serial.println(volStrForSave.c_str());
    //    }
    //}
}
