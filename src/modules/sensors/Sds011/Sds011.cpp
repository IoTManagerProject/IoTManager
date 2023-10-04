/******************************************************************
 Supports Sds011, implements whole Laser Dust Sensor Control Protocol V1.3,
  should also work with other Sds sensors.

  https://github.com/lewapek/sds-dust-sensors-arduino-library

  adapted for version 4 @cmche
 ******************************************************************/

#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;

#include "SdsDustSensor.h"

//#define __DEBUG_SDS_DUST_SENSOR__

int rxPinSDS; // Esp8266: 14/D5  – подключаем к Tx сенсора
int txPinSDS; // Esp8266: 16/D0  – подключаем к Rx сенсора

// SdsDustSensor sds(rxPinSDS, txPinSDS);
SdsDustSensor *sds = nullptr;

#ifdef ESP32
#include <HardwareSerial.h>
HardwareSerial sdsSerial(2);
// SdsDustSensor sds(sdsSerial);
#endif

int retryDelayMs = 5;
int maxRetriesNotAvailable = 100;
unsigned int purge = 30;
unsigned int interval = 300;
unsigned int purgeDelay = 270;
unsigned int continuous = 0;
bool startUp = true;

TickerScheduler ts_sds(2);

enum TimerTask_t_sds
{
    WAKEUP,
    PRINT
};

int firstSensor = 0;
bool SDS011_init_flag = true;
void SDS011_init();
void Sds011request(int sensorID);

IoTItem *item_Sds011_25 = nullptr; // pointer
IoTItem *item_Sds011_10 = nullptr;

//Это файл сенсора, в нем осуществляется чтение сенсора.
//для добавления сенсора вам нужно скопировать этот файл и заменить в нем текст AnalogAdc на название вашего сенсора
//Название должно быть уникальным, коротким и отражать суть сенсора.

class Sds011_25 : public IoTItem
{
private:
    //=======================================================================================================
    // Секция переменных.
    //Это секция где Вы можете объявлять переменные и объекты arduino библиотек, что бы
    //впоследствии использовать их в loop и setup
    // unsigned int _pin;

public:
    //=======================================================================================================
    // setup()
    //это аналог setup из arduino. Здесь вы можете выполнять методы инициализации сенсора.
    //Такие как ...begin и подставлять в них параметры полученные из web интерфейса.
    //Все параметры хранятся в перемененной parameters, вы можете прочитать любой параметр используя jsonRead функции:
    // jsonReadStr, jsonReadBool, jsonReadInt
    Sds011_25(String parameters) : IoTItem(parameters)
    {
        item_Sds011_25 = this;

        rxPinSDS = jsonReadInt(parameters, "rxPin");
        txPinSDS = jsonReadInt(parameters, "txPin");

        purge = jsonReadInt(parameters, "purge");                                   // сек. пробужнение должен быть больше
        interval = jsonReadInt(parameters, "int");                                  // сек. время зарогрева/продувки, затем идут замеры
        continuous = jsonReadInt(parameters, "continuousMode");                     // сек. время зарогрева/продувки, затем идут замеры
        maxRetriesNotAvailable = jsonReadInt(parameters, "maxRetriesNotAvailable"); // сек. время зарогрева/продувки, затем идут замеры
        retryDelayMs = jsonReadInt(parameters, "retryDelayMs");                     // сек. время зарогрева/продувки, затем идут замеры
        if (continuous)
        {
            SerialPrint("i", "Sensor Sds011", "Continuous mode");
            ts_sds.remove(PRINT);
            ts_sds.remove(WAKEUP);
        }
        purgeDelay = interval - purge;

        SDS011_init();
        firstSensor = 0;
    }
    //=======================================================================================================
    // doByInterval()
    //это аналог loop из arduino, но вызываемый каждые int секунд, заданные в настройках. Здесь вы должны выполнить чтение вашего сенсора
    //а затем выполнить regEvent - это регистрация произошедшего события чтения
    //здесь так же доступны все переменные из секции переменных, и полученные в setup
    //если у сенсора несколько величин то делайте несколько regEvent
    //не используйте delay - помните, что данный loop общий для всех модулей. Если у вас планируется длительная операция, постарайтесь разбить ее на порции
    //и выполнить за несколько тактов
    void doByInterval()
    {
        SDS011_init();
        // SerialPrint("i", "Sensor Sds011", "Request for 2.5");
        Sds011request(25);
        // value.valD = Sds011request(25);
        // regEvent(value.valD, "Sds011_25"); //обязательный вызов хотяб один
    }
    ~Sds011_25(){};
};

//////////////////////////////////// for PM 10//=

class Sds011_10 : public IoTItem
{
private:
    //=======================================================================================================
    // Секция переменных.
    //Это секция где Вы можете объявлять переменные и объекты arduino библиотек, что бы
    //впоследствии использовать их в loop и setup

public:
    //=======================================================================================================
    // setup()

    Sds011_10(String parameters) : IoTItem(parameters)
    {
        item_Sds011_10 = this;

        rxPinSDS = jsonReadInt(parameters, "rxPin");
        txPinSDS = jsonReadInt(parameters, "txPin");

        purge = jsonReadInt(parameters, "purge");               // сек. пробужнение должен быть больше
        interval = jsonReadInt(parameters, "int");              // сек. время зарогрева/продувки, затем идут замеры
        continuous = jsonReadInt(parameters, "continuousMode"); // сек. время зарогрева/продувки, затем идут замеры
        if (continuous)
        {
            SerialPrint("i", "Sensor Sds011", "Continuous mode");
            ts_sds.remove(PRINT);
            ts_sds.remove(WAKEUP);
        }
        else
        {
            SerialPrint("i", "Sensor Sds011", "WakeUp mode");
        }
        purgeDelay = interval - purge;
        SDS011_init();
        firstSensor = 0;
    }

    //луп выполняющий переодическое дерганье
    void loop()
    {
        ts_sds.update();
        IoTItem::loop();
        // if (enableDoByInt)
        // {
        //     currentMillis = millis();
        //     difference = currentMillis - prevMillis;
        //     if (difference >= _interval)
        //     {
        //         prevMillis = millis();
        //         this->doByInterval();
        //     }
        // }
    }
    //=======================================================================================================
    // doByInterval()
    //это аналог loop из arduino, но вызываемый каждые int секунд, заданные в настройках. Здесь вы должны выполнить чтение вашего сенсора
    //а затем выполнить regEvent - это регистрация произошедшего события чтения
    //здесь так же доступны все переменные из секции переменных, и полученные в setup
    //если у сенсора несколько величин то делайте несколько regEvent
    //не используйте delay - помните, что данный loop общий для всех модулей. Если у вас планируется длительная операция, постарайтесь разбить ее на порции
    //и выполнить за несколько тактов

    void doByInterval()
    {
        SDS011_init();
        // SerialPrint("i", "Sensor Sds011", "Request for 10ppm");
        Sds011request(10);
        // value.valD = Sds011request(10);
        // regEvent(value.valD, "Sds011_10"); //обязательный вызов хотяб один
    }
    ~Sds011_10(){};
};

//после замены названия сенсора, на функцию можно не обращать внимания
//если сенсор предполагает использование общего объекта библиотеки для нескольких экземпляров сенсора, то в данной функции необходимо предусмотреть
//создание и контроль соответствующих глобальных переменных
void *getAPI_Sds011(String subtype, String param)
{
    if (subtype == F("Sds011_25"))
    {
        return new Sds011_25(param);
    }
    else if (subtype == F("Sds011_10"))
    {
        return new Sds011_10(param);
    }
    else
    {
        return nullptr;
    }
}

void Sds011request(int sensorID)
{
    float pm25 = 0;
    float pm10 = 0;

    if (firstSensor == 0)
        firstSensor = sensorID;

    if (firstSensor == sensorID)
    {
        if (!continuous)
        {
            ts_sds.remove(PRINT);
            ts_sds.remove(WAKEUP);
        }

        PmResult pm = sds->readPm();
        // SerialPrint("i", "Sensor Sds011", "sds.readPm()");
        if (pm.isOk())
        {
            pm25 = pm.pm25;
            pm10 = pm.pm10;
            // SerialPrint("i", "Sensor Sds011", pm.toString());

            if (item_Sds011_25 && pm25)
            {
                item_Sds011_25->value.valD = pm25;
                item_Sds011_25->regEvent(item_Sds011_25->value.valD, "Sds011_25");
            }

            if (item_Sds011_10 && pm10)
            {
                item_Sds011_10->value.valD = pm10;
                item_Sds011_10->regEvent(item_Sds011_10->value.valD, "Sds011_10");
            }
        }
        else
        {
            String msg = "Could not read values from sensor. Reason: " + pm.statusToString();
            SerialPrint("E", "Sensor Sds011", msg);
            SDS011_init_flag = true;
        }

        if (!continuous)
        {
            sds->sleep();
            SerialPrint("i", "Sensor Sds011", "sleep");
            String msg = "wakeUp planned in " + String(purgeDelay) + " seconds";
            SerialPrint("i", "Sensor Sds011", msg);
            ts_sds.add(
                PRINT, purgeDelay * 1000, [](void *)
                { SerialPrint("i", "Sensor Sds011", "delayed wakeUp"); },
                nullptr, false);
            ts_sds.add(
                WAKEUP, purgeDelay * 1000, [](void *)
                { sds->wakeup(); },
                nullptr, false);
        }
    }
    else
    {
        //пропускаем вызов от второго сенсора
    }

    return;
}

void SDS011_init()
{
    if (SDS011_init_flag)
    {

#ifdef ESP8266

        if (!sds)
        {
            Serial.println("no sds, creating");
            sds = new SdsDustSensor(rxPinSDS, txPinSDS, retryDelayMs, maxRetriesNotAvailable);
        }
        else
        {
            Serial.println("sds already created");
        }

        sds->begin(9600);
        delay(200); //
#endif

#ifdef ESP32
        sdsSerial.begin(9600, SERIAL_8N1, rxPinSDS, txPinSDS);
        delay(200);
        if (!sds)
        {
            Serial.println("no sds, creating");
            sds = new SdsDustSensor(sdsSerial, retryDelayMs, maxRetriesNotAvailable);
        }
        else
        {
            Serial.println("sds already created");
        }
#endif

        if (startUp)
        {
            sds->wakeup();
            SerialPrint("i", "Sensor Sds011", "wakeup on startUp"); // уже начинаем продувку
            startUp = false;
        }
        String msg = sds->queryFirmwareVersion().toString(); //
        SerialPrint("i", "Sensor Sds011", msg);
        String ReportingMode = sds->setActiveReportingMode().toString();

        if (ReportingMode == "Mode: active")
        {
            SerialPrint("i", "Sensor Sds011", ReportingMode);
            SDS011_init_flag = false;
            if (continuous)
            {
                sds->wakeup();
                SerialPrint("i", "Sensor Sds011", "wakeUp if continuous");
            }
        }
        else
        {
            ReportingMode += " - check wiring!";
            SerialPrint("E", "Sensor Sds011", ReportingMode);
        }
    }
}
