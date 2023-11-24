#include "Main.h"
#include <time.h>
#include "classes/IoTDB.h"
#include "utils/Statistic.h"
#include <Wire.h>
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
        (*it)->loop();

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

#if defined(esp32_4mb) || defined(esp32_16mb) || defined(esp32cam_4mb)

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
#if defined(esp32_4mb) || defined(esp32_16mb) || defined(esp32cam_4mb)
    initErrorMarkerId = id;
    errorMarkerCounter = 0;
#endif
}

void stopErrorMarker(int id) {
#if defined(esp32_4mb) || defined(esp32_16mb) || defined(esp32cam_4mb)
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
#if defined(esp32_4mb) || defined(esp32_16mb) || defined(esp32cam_4mb)
    My_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(My_timer, &onTimer, true);
    timerAlarmWrite(My_timer, STEPPER_ERRORMARKER, true);
    timerAlarmEnable(My_timer);
    // timerAlarmDisable(My_timer);

    initErrorMarker(SETUPBASE_ERRORMARKER);
#endif

    Serial.begin(115200);
    Serial.flush();
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

    // синхронизация глобальных переменных с flash
    globalVarsSync();

    stopErrorMarker(SETUPBASE_ERRORMARKER);

    initErrorMarker(SETUPCONF_ERRORMARKER);

    // настраиваем i2c шину
    int i2c, pinSCL, pinSDA, i2cFreq;
    jsonRead(settingsFlashJson, "pinSCL", pinSCL, false);
    jsonRead(settingsFlashJson, "pinSDA", pinSDA, false);
    jsonRead(settingsFlashJson, "i2cFreq", i2cFreq, false);
    jsonRead(settingsFlashJson, "i2c", i2c, false);
    if (i2c != 0) {
#ifdef ESP32
        Wire.end();
        Wire.begin(pinSDA, pinSCL, (uint32_t)i2cFreq);
#else
        Wire.begin(pinSDA, pinSCL);
        Wire.setClock(i2cFreq);
#endif
        SerialPrint("i", "i2c", F("i2c pins overriding done"));
    }

    // настраиваем микроконтроллер
    configure("/config.json");

    stopErrorMarker(SETUPCONF_ERRORMARKER);

    initErrorMarker(SETUPSCEN_ERRORMARKER);

    // подготавливаем сценарии
    iotScen.loadScenario("/scenario.txt");
    // создаем событие завершения инициализации основных моментов для возможности выполнения блока кода при загрузке
    createItemFromNet("onInit", "1", 1);
    elementsLoop();

    stopErrorMarker(SETUPSCEN_ERRORMARKER);

    initErrorMarker(SETUPINET_ERRORMARKER);

    // подключаемся к роутеру
    routerConnect();

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

    initErrorMarker(SETUPLAST_ERRORMARKER);

    // NTP
    ntpInit();

    // инициализация задач переодического выполнения
    periodicTasksInit();

    // запуск работы udp
    addThisDeviceToList();
    udpListningInit();
    udpBroadcastInit();

    // создаем событие завершения конфигурирования для возможности выполнения блока кода при загрузке
    createItemFromNet("onStart", "1", 1);

    stInit();

    // настраиваем секундные обслуживания системы
    ts.add(
        TIMES, 1000, [&](void *) {
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

    // test
    Serial.println("-------test start--------");
    Serial.println("--------test end---------");

    stopErrorMarker(SETUPLAST_ERRORMARKER);
}

void loop() {
#ifdef LOOP_DEBUG
    unsigned long st = millis();
#endif

    initErrorMarker(TICKER_ERRORMARKER);
    ts.update();
    stopErrorMarker(TICKER_ERRORMARKER);

#ifdef STANDARD_WEB_SERVER
    initErrorMarker(HTTP_ERRORMARKER);
    HTTP.handleClient();
    stopErrorMarker(HTTP_ERRORMARKER);
#endif

#ifdef STANDARD_WEB_SOCKETS
    initErrorMarker(SOCKETS_ERRORMARKER);
    standWebSocket.loop();
    stopErrorMarker(SOCKETS_ERRORMARKER);
#endif

    initErrorMarker(MQTT_ERRORMARKER);
    mqttLoop();
    stopErrorMarker(MQTT_ERRORMARKER);

    initErrorMarker(MODULES_ERRORMARKER);
    elementsLoop();
    stopErrorMarker(MODULES_ERRORMARKER);

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
