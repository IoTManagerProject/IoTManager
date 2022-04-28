#include "Main.h"
#include "classes/IoTRTC.h"
#include <time.h>

IoTScenario iotScen;  // объект управления сценарием

IoTRTC *watch;     // объект хранения времени, при старте часы 00:00
time_t iotTimeNow;

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

    // настраиваем получение времени из сети Интернет
    String ntpServer = jsonReadStr(settingsFlashJson, F("ntp"));
    int timezone = jsonReadInt(settingsFlashJson, F("timezone"));
    configTime(3600*timezone, 0, ntpServer.c_str(), "ru.pool.ntp.org", "pool.ntp.org");
    Serial.print("Syncing NTP time");
    int i = 0;
    while (isNetworkActive() && iotTimeNow < 1510592825 && i < 200)
    {
        time(&iotTimeNow);
        delay(300);
        Serial.print(".");
        i++;
    }
    Serial.println();

    // настраиваем локальный RTC
    watch = new IoTRTC(0);    // создаем объект главного хранилища времени, но с заглушкой (0) для получения системного времени
    watch->period(60);    // время в минутах для синхронизации с часами реального времени или системным 
    watch->begin();
    Serial.print(F("Time from Local: "));
    Serial.println(watch->gettime("d-m-Y, H:i:s, M"));

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
    // создаем событие завершения конфигурирования для возможности выполнения блока кода при загрузке
    IoTItems.push_back((IoTItem*)new externalVariable("{\"id\":\"onConfig\",\"val\":1,\"int\":60}"));
    generateEvent("onConfig", "");

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
    for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
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
    currentMillis = millis();
    if (currentMillis - prevMillis >= 1000) {
        prevMillis = millis();
        volStrForSave = "";
        for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
            if ((*it)->needSave) {
                (*it)->needSave = false;
                volStrForSave = volStrForSave + (*it)->getID() + "=" + (*it)->getValue() + ";";
            }
        }

        if (volStrForSave != "") {
            Serial.print("volStrForSave: ");
            Serial.println(volStrForSave.c_str());    
        }
    }
    
    
    
}
