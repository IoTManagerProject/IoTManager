#include "Main.h"
#include <time.h>
#include "classes/IoTDB.h"
#include "utils/Statistic.h"
#include "classes/IoTBench.h"
#ifndef LIBRETINY
#include <Wire.h>
#endif
#include "DebugTrace.h"
#if defined(ESP32)
#include <esp_task_wdt.h>
#endif
#if defined(esp32s2_4mb) || defined(esp32s3_16mb)
#include <USB.h>
#endif

IoTScenario iotScen;  // объект управления сценарием

String volStrForSave = "";
// unsigned long currentMillis; // это сдесь лишнее
// unsigned long prevMillis;

void elementsLoop() {
    // передаем управление каждому элементу конфигурации для выполнения своих функций
    for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
        if (benchTaskItem) benchTaskItem->preTaskFunction((*it)->getID());
        (*it)->loop();
        if (benchTaskItem) benchTaskItem->postTaskFunction((*it)->getID());
        // if ((*it)->iAmDead) {
        if (!((*it)->iAmLocal) && (*it)->getIntFromNet() == -1) {
            delete *it;
            IoTItems.erase(it);
            break;
        }
    }

    handleOrder();
    handleEvent();
}

#define SETUPBASE_ERRORMARKER 0
#define SETUPCONF_ERRORMARKER 1
#define SETUPSCEN_ERRORMARKER 2
#define SETUPINET_ERRORMARKER 3
#define SETUPLAST_ERRORMARKER 4
#define TICKER_ERRORMARKER 5
#define HTTP_ERRORMARKER 6
#define SOCKETS_ERRORMARKER 7
#define MQTT_ERRORMARKER 8
#define MODULES_ERRORMARKER 9

#define COUNTER_ERRORMARKER 4       // количество шагов счетчика
#define STEPPER_ERRORMARKER 100000  // размер шага счетчика интервала доверия выполнения блока кода мкс

#if defined(esp32_4mb) || defined(esp32_4mb3f) || defined(esp32_16mb) || defined(esp32cam_4mb)

static int IRAM_ATTR initErrorMarkerId = 0;  // ИД маркера
static int IRAM_ATTR errorMarkerId = 0;
static int IRAM_ATTR errorMarkerCounter = 0;

hw_timer_t *My_timer = NULL;
void IRAM_ATTR onTimer() {
    if (errorMarkerCounter >= 0) {
        if (errorMarkerCounter >= COUNTER_ERRORMARKER) {
            errorMarkerId = initErrorMarkerId;
            errorMarkerCounter = -1;
        } else
            errorMarkerCounter++;
    }
}
#endif

void initErrorMarker(int id) {
#if defined(esp32_4mb) || defined(esp32_4mb3f) || defined(esp32_16mb) || defined(esp32cam_4mb)
    initErrorMarkerId = id;
    errorMarkerCounter = 0;
#endif
}

void stopErrorMarker(int id) {
#if defined(esp32_4mb) || defined(esp32_4mb3f) || defined(esp32_16mb) || defined(esp32cam_4mb)
    errorMarkerCounter = -1;
    if (errorMarkerId)
        SerialPrint("I", "WARNING!", "A lazy (freezing loop more than " + (String)(COUNTER_ERRORMARKER * STEPPER_ERRORMARKER / 1000) + " ms) section has been found! With ID=" + (String)errorMarkerId);
    errorMarkerId = 0;
    initErrorMarkerId = 0;
#endif
}

void setup() {
#if defined(esp32s2_4mb) || defined(esp32s3_16mb)
    USB.begin();
#endif
#if defined(esp32_4mb) || defined(esp32_4mb3f) || defined(esp32_16mb) || defined(esp32cam_4mb)
    My_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(My_timer, &onTimer, true);
    timerAlarmWrite(My_timer, STEPPER_ERRORMARKER, true);
    timerAlarmEnable(My_timer);
    // timerAlarmDisable(My_timer);

    initErrorMarker(SETUPBASE_ERRORMARKER);
#endif

    Serial.begin(115200);
    Serial.flush();
    //----------- Отладка EXCEPTION (функции с заглушками для отключения) ---------
#if defined(RESTART_DEBUG_INFO)  
    //Привязка коллбэк функции для вызова при перезагрузке
    esp_register_shutdown_handler(debugUpdate);
#endif // RESTART_DEBUG_INFO
    // Печать или оправка отладочной информации
    printDebugTrace();
    startWatchDog();
    Serial.println();
    Serial.println(F("--------------started----------------"));

    // создание экземпляров классов
    // myNotAsyncActions = new NotAsync(do_LAST);

    // инициализация файловой системы
    fileSystemInit();
    Serial.println(F("------------------------"));
    Serial.println("FIRMWARE NAME     " + String(FIRMWARE_NAME));
    Serial.println("FIRMWARE VERSION  " + String(FIRMWARE_VERSION));
    Serial.println("WEB VERSION       " + getWebVersion());
    const String buildTime = String(BUILD_DAY) + "." + String(BUILD_MONTH) + "." + String(BUILD_YEAR) + " " + String(BUILD_HOUR) + ":" + String(BUILD_MIN) + ":" + String(BUILD_SEC);
    Serial.println("BUILD TIME        " + buildTime);
    jsonWriteStr_(errorsHeapJson, F("bt"), buildTime);
    Serial.println(F("------------------------"));

    // получение chip id
    setChipId();

    verifyFirmware();

    // синхронизация глобальных переменных с flash
    globalVarsSync();

    stopErrorMarker(SETUPBASE_ERRORMARKER);

    initErrorMarker(SETUPCONF_ERRORMARKER);

#ifndef LIBRETINY
    // настраиваем i2c шину
    // ВАЖНО: jsonRead при ошибке чтения НЕ меняет переменную (см. utils/JsonUtils.cpp),
    // поэтому при недоступном settings.json сюда попадает мусор со стека. Инициализируем
    // значениями по умолчанию и проверяем валидность пинов перед Wire.begin().
    // Для LibreTiny (bk72xx) шины I2C в Arduino-ядре нет: Wire.h не подключается,
    // макросы SCL/SDA не определены, вызовы Wire.* ниже и так только под ESP32/ESP8266.
    int i2c = 0, pinSCL = SCL, pinSDA = SDA, i2cFreq = 100000;
    jsonRead(settingsFlashJson, "pinSCL", pinSCL, false);
    jsonRead(settingsFlashJson, "pinSDA", pinSDA, false);
    jsonRead(settingsFlashJson, "i2cFreq", i2cFreq, false);
    jsonRead(settingsFlashJson, "i2c", i2c, false);
    //jsonWriteStr_(ssidListHeapJson, "0", "Scaning...");
    if (i2c != 0 && pinSCL >= 0 && pinSDA >= 0 && pinSCL != pinSDA && i2cFreq >= 10000) {
#ifdef ESP32
        Wire.end();
        Wire.begin(pinSDA, pinSCL, (uint32_t)i2cFreq);
#elif defined(ESP8266)
        Wire.begin(pinSDA, pinSCL);
        Wire.setClock(i2cFreq);
#endif
        SerialPrint("i", "i2c", F("i2c pins overriding done"));
    } else if (i2c != 0) {
        // защита от невалидных пинов (scl==sda или мусор из-за отсутствия settings.json):
        // инициализируем шину пинами по умолчанию, чтобы датчики продолжали работать
        SerialPrint("E", "i2c", F("i2c pins invalid (scl==sda or out of range), using defaults"));
#ifdef ESP32
        Wire.end();
        Wire.begin(SDA, SCL, 100000);
#elif defined(ESP8266)
        Wire.begin(SDA, SCL);
        Wire.setClock(100000);
#endif
    }
#endif  // !LIBRETINY

#if defined(RESTART_DEBUG_INFO)
  esp_reset_reason_t esp_reason = esp_reset_reason();
  if (esp_reason == ESP_RST_UNKNOWN || esp_reason == ESP_RST_POWERON) 
    bootloop_panic_count = 0;
/*   else if (bootloop_panic_count == 3 || bootloop_panic_count == 0 ) bootloop_panic_count = 1;
  else if (bootloop_panic_count == 2) bootloop_panic_count = 3;
  else if (bootloop_panic_count == 1) bootloop_panic_count = 2;
  else bootloop_panic_count = 0; */
     Serial.println("bootloop_panic_count     " + String(bootloop_panic_count));
    if (bootloop_panic_count >3 )
    {
        //resetSettingsFlashByPanic();
        bootloop_panic_count = 0;
    }      
    if (bootloop_panic_count >= 3)
    {
        resetSettingsFlashByPanic();
        bootloop_panic_count = -1;
    } 
    if (bootloop_panic_count == -1)
    {
        SerialPrint("E", "CORE", F("CONFIG and SCENARIO reset !!!"));
        bootloop_panic_count = 0;
        ESP.restart();
    } 
#endif // RESTART_DEBUG_INFO
    // настраиваем микроконтроллер
    configure("/config.json");

    stopErrorMarker(SETUPCONF_ERRORMARKER);

    initErrorMarker(SETUPSCEN_ERRORMARKER);

    // подготавливаем сценарии
    iotScen.loadScenario("/scenario.txt");
    // создаем событие завершения инициализации основных моментов для возможности выполнения блока кода при загрузке
    createItemFromNet("onInit", "1", 1);
//    elementsLoop(); //Для работы MQTT Брокера перенес ниже, иначе брокер падает если вызван до routerConnect();

    stopErrorMarker(SETUPSCEN_ERRORMARKER);

    initErrorMarker(SETUPINET_ERRORMARKER);

// подключаемся к роутеру
#ifndef WIFI_ASYNC
    routerConnect();
#else
    WiFiUtilsItit();
#endif
// инициализация асинхронного веб сервера и веб сокетов
#ifdef ASYNC_WEB_SERVER
    asyncWebServerInit();
#endif
#ifdef ASYNC_WEB_SOCKETS
    asyncWebSocketsInit();
#endif

// инициализация стандартного веб сервера и веб сокетов
#ifdef STANDARD_WEB_SERVER
    standWebServerInit();
#endif
#ifdef STANDARD_WEB_SOCKETS
    standWebSocketsInit();
#endif

    stopErrorMarker(SETUPINET_ERRORMARKER);

    // bool postMsgTelegram;
    // if (!jsonRead(settingsFlashJson, "debugTraceMsgTlgrm", postMsgTelegram, false)) postMsgTelegram = 1;
    // sendDebugTraceAndFreeMemory(postMsgTelegram);
    
    initErrorMarker(SETUPLAST_ERRORMARKER);

    elementsLoop();
    // NTP
    ntpInit();

    // инициализация задач переодического выполнения
    periodicTasksInit();

#if !defined(WIFI_ASYNC)
    // Перенесли после получения IP, так как теперь работа WiFi асинхронная
    // запуск работы udp
    addThisDeviceToList();
    #ifdef UDP_ENABLED
    udpListningInit();
    udpBroadcastInit();
    #endif
#endif    
    // создаем событие завершения конфигурирования для возможности выполнения блока кода при загрузке
    createItemFromNet("onStart", "1", 1);

    stInit();

    // настраиваем секундные обслуживания системы
    ts.add(
        TIMES, 1000, [&](void *) {
            // сброс WDT
#if defined(ESP32)
            //SerialPrint("i", "Task", "reset wdt");
             #if !defined(esp32c6_4mb) && !defined(esp32c6_8mb) //TODO esp32-c6 переписать esp_task_wdt_init
            esp_task_wdt_reset();
            #endif
#endif
            // сохраняем значения IoTItems в файл каждую секунду, если были изменения (установлены маркеры на сохранение)
            if (needSaveValues) {
                syncValuesFlashJson();
                needSaveValues = false;
            }

            // проверяем все элементы на тухлость
            for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
                (*it)->checkIntFromNet();

                // Serial.printf("[ITEM] size: %d, id: %s, int: %d, intnet: %d\n", sizeof(**it), (*it)->getID(), (*it)->getInterval(), (*it)->getIntFromNet());
            }
        },
        nullptr, true);

    // ловим пинги от WS (2сек) и дисконнектим если их нет 3 раза 3сек*2прохода = 6сек
    ts.add(
        PiWS, 3000, [&](void*) {
            if (isNetworkActive()) {
                for (size_t i = 0; i < WEBSOCKETS_CLIENT_MAX; i++)
                {
                    if (ws_clients[i] == 0) {
                        disconnectWSClient(i);
                        ws_clients[i]=-1;
                    }
                    if (ws_clients[i] > 0) { 
                        ws_clients[i]=0;
                    }

                }
            }
        },
        nullptr, true);

    // test
    //Serial.println("-------test start--------");
    //Serial.println("--------test end---------");

    stopErrorMarker(SETUPLAST_ERRORMARKER);
#if defined(RESTART_DEBUG_INFO)    
    bootloop_panic_count = 0;
#endif // RESTART_DEBUG_INFO
}

void loop() {
#if defined(WIFI_ASYNC)
    static bool udpFirstFlag = true;
    // Перенесли после получения IP, так как теперь работа WiFi асинхронная
    if (isNetworkActive() && udpFirstFlag) {
        udpFirstFlag = false;
        // запуск работы udp
        addThisDeviceToList();
        #ifdef UDP_ENABLED
        udpListningInit();
        udpBroadcastInit();
        #endif
    }
#endif    

#ifdef LOOP_DEBUG
    unsigned long st = millis();
#endif
    if (benchLoadItem) benchLoadItem->preLoadFunction();
    if (benchTaskItem) benchTaskItem->preTaskFunction("TickerScheduler");
    initErrorMarker(TICKER_ERRORMARKER);
    ts.update();
    stopErrorMarker(TICKER_ERRORMARKER);
    if (benchTaskItem) benchTaskItem->postTaskFunction("TickerScheduler");
    if (benchTaskItem) benchTaskItem->preTaskFunction("webServer");
#ifdef STANDARD_WEB_SERVER
    initErrorMarker(HTTP_ERRORMARKER);
    HTTP.handleClient();
    stopErrorMarker(HTTP_ERRORMARKER);
#endif
    if (benchTaskItem) benchTaskItem->postTaskFunction("webServer");
    if (benchTaskItem) benchTaskItem->preTaskFunction("webSocket");
#ifdef STANDARD_WEB_SOCKETS
    initErrorMarker(SOCKETS_ERRORMARKER);
    standWebSocket.loop();
    stopErrorMarker(SOCKETS_ERRORMARKER);
#endif
#ifdef ASYNC_WEB_SOCKETS
    // асинхронный сервер обслуживает соединения сам (в прерываниях AsyncTCP),
    // здесь только разбор накопленных сообщений и служебные операции
    initErrorMarker(SOCKETS_ERRORMARKER);
    asyncWebSocketsLoop();
    stopErrorMarker(SOCKETS_ERRORMARKER);
#endif
    if (benchTaskItem) benchTaskItem->postTaskFunction("webSocket");
    if (benchTaskItem) benchTaskItem->preTaskFunction("mqtt");
    initErrorMarker(MQTT_ERRORMARKER);
    mqttLoop();
    stopErrorMarker(MQTT_ERRORMARKER);
    if (benchTaskItem) benchTaskItem->postTaskFunction("mqtt");
    initErrorMarker(MODULES_ERRORMARKER);
    elementsLoop();
    stopErrorMarker(MODULES_ERRORMARKER);
    if (benchLoadItem) benchLoadItem->postLoadFunction();
    // #ifdef LOOP_DEBUG
    //     loopPeriod = millis() - st;
    //     if (loopPeriod > 2) Serial.println(loopPeriod);
    // #endif
}

// отправка json
// #ifdef QUEUE_FROM_STR
//     if (sendJsonFiles) sendJsonFiles->loop();
// #endif

// if(millis()%2000==0){
//     //watch->settimeUnix(time(&iotTimeNow));
//     Serial.println(watch->gettime("d-m-Y, H:i:s, M"));
//     delay(1);
// }

// File dir = FileFS.open("/", "r");
// String out;
// printDirectory(dir, out);
// Serial.println(out);

//=======проверка очереди из структур=================

// myDB = new IoTDB;
// QueueItems myItem;
// myItem.myword = "word1";
// myDB->push(myItem);
// myItem.myword = "word2";
// myDB->push(myItem);
// myItem.myword = "word3";
// myDB->push(myItem);
// Serial.println(myDB->front().myword);
// Serial.println(myDB->front().myword);
// Serial.println(myDB->front().myword);

// Serial.println(FileList("lg"));
