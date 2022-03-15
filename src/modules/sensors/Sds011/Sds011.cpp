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

//встроена в ядро для 8266, для 32 по этому же имени обращаемся к другой библиотеке plerup/EspSoftwareSerial
#include <SoftwareSerial.h>

// to do убрать глобальный экземпляр
#ifdef ESP8266
int rxPinSDS = 13;  // D7 – подключаем к Tx сенсора
int txPinSDS = 12;  // D6 – подключаем к Rx сенсора
SdsDustSensor sds(rxPinSDS, txPinSDS);
#endif
#ifdef ESP32
#include <HardwareSerial.h>
HardwareSerial sdsSerial(2);
SdsDustSensor sds(sdsSerial);
#endif

unsigned int warmUp;
unsigned int period;

bool SDS011_init_flag = true;
void SDS011_init();
float Sds011request(int sensorID);

//Это файл сенсора, в нем осуществляется чтение сенсора.
//для добавления сенсора вам нужно скопировать этот файл и заменить в нем текст AnalogAdc на название вашего сенсора
//Название должно быть уникальным, коротким и отражать суть сенсора.

class Sds011_25 : public IoTItem {
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
    Sds011_25(String parameters) : IoTItem(parameters) {
        // _pin = jsonReadInt(parameters, "pin");
#ifdef ESP8266
        rxPinSDS = jsonReadInt(parameters, "rxPin");
        txPinSDS = jsonReadInt(parameters, "txPin");
#endif
        warmUp = jsonReadInt(parameters, "warmUp");  // сек. пробужнение должен быть больше
        period = jsonReadInt(parameters, "period");  // сек. время зарогрева/продувки, затем идут замеры
    }
    //=======================================================================================================
    // doByInterval()
    //это аналог loop из arduino, но вызываемый каждые int секунд, заданные в настройках. Здесь вы должны выполнить чтение вашего сенсора
    //а затем выполнить regEvent - это регистрация произошедшего события чтения
    //здесь так же доступны все переменные из секции переменных, и полученные в setup
    //если у сенсора несколько величин то делайте несколько regEvent
    //не используйте delay - помните, что данный loop общий для всех модулей. Если у вас планируется длительная операция, постарайтесь разбить ее на порции
    //и выполнить за несколько тактов
    void doByInterval() {
        SDS011_init();
        Serial.println("request from 25");
        value.valD = Sds011request(25);

        regEvent(value.valD, "Sds011_25");  //обязательный вызов хотяб один
    }

    ~Sds011_25(){};
};

//////////////////////////////////// for PM 10//=

class Sds011_10 : public IoTItem {
   private:
    //=======================================================================================================
    // Секция переменных.
    //Это секция где Вы можете объявлять переменные и объекты arduino библиотек, что бы
    //впоследствии использовать их в loop и setup

   public:
    //=======================================================================================================
    // setup()
    //это аналог setup из arduino. Здесь вы можете выполнять методы инициализации сенсора.
    //Такие как ...begin и подставлять в них параметры полученные из web интерфейса.
    //Все параметры хранятся в перемененной parameters, вы можете прочитать любой параметр используя jsonRead функции:
    // jsonReadStr, jsonReadBool, jsonReadInt
    Sds011_10(String parameters) : IoTItem(parameters) {
// _pin = jsonReadInt(parameters, "pin");
#ifdef ESP8266
        rxPinSDS = jsonReadInt(parameters, "rxPin");
        txPinSDS = jsonReadInt(parameters, "txPin");
#endif
        warmUp = jsonReadInt(parameters, "warmUp");  // сек. пробужнение должен быть больше
        period = jsonReadInt(parameters, "period");  // сек. время зарогрева/продувки, затем идут замеры
    }
    //=======================================================================================================
    // doByInterval()
    //это аналог loop из arduino, но вызываемый каждые int секунд, заданные в настройках. Здесь вы должны выполнить чтение вашего сенсора
    //а затем выполнить regEvent - это регистрация произошедшего события чтения
    //здесь так же доступны все переменные из секции переменных, и полученные в setup
    //если у сенсора несколько величин то делайте несколько regEvent
    //не используйте delay - помните, что данный loop общий для всех модулей. Если у вас планируется длительная операция, постарайтесь разбить ее на порции
    //и выполнить за несколько тактов

    void doByInterval() {
        SDS011_init();
        Serial.println("request from 10");
        value.valD = Sds011request(10);

        regEvent(value.valD, "Sds011_10");  //обязательный вызов хотяб один
    }
    ~Sds011_10(){};
};

//после замены названия сенсора, на функцию можно не обращать внимания
//если сенсор предполагает использование общего объекта библиотеки для нескольких экземпляров сенсора, то в данной функции необходимо предусмотреть
//создание и контроль соответствующих глобальных переменных
void* getAPI_Sds011(String subtype, String param) {
    if (subtype == F("Sds011_25")) {
        return new Sds011_25(param);
    } else if (subtype == F("Sds011_10")) {
        return new Sds011_10(param);
    } else {
        return nullptr;
    }
}

float Sds011request(int sensorID) {
    float reply = 0;
    static int a = 0;
    static float pm25 = 0;
    static float pm10 = 0;
    static int startMillis = millis();

    if (a == 0) {
        Serial.print("SDS011  ... warmUp = ");
        Serial.print(warmUp);
        Serial.print("     period = ");
        Serial.println(period);
        sds.wakeup();
        startMillis = millis();
        a = a + 1;
    }
    if (a == 1 && millis() >= (startMillis + warmUp * 1000)) {
        PmResult pm = sds.readPm();
        if (pm.isOk()) {
            pm25 = pm.pm25;
            pm10 = pm.pm10;
            Serial.print("PM2.5 = ");
            Serial.print(pm25);
            Serial.print("     PM10 = ");
            Serial.println(pm10);
            a = a + 1;
            sds.sleep();
        } else {
            Serial.print("Could not read values from sensor 25, reason: ");
            Serial.println(pm.statusToString());
            a = a + 1;
        }
    }
    if (a > 1 && millis() >= (startMillis + period * 1000)) {
        Serial.println("end of period for pm25");
        a = 0;
    }
    if (sensorID == 25) {
        reply = pm25;
    }
    if (sensorID == 10) {
        reply = pm10;
    }
    return reply;
}

void SDS011_init() {
    if (SDS011_init_flag) {
        sds.begin();
        Serial.println(sds.queryFirmwareVersion().toString());  // prints firmware version
                                                                //           Serial.println(sds.setActiveReportingMode().toString()); //
        String ReportingMode = sds.setActiveReportingMode().toString();
        Serial.println(ReportingMode);
        if (ReportingMode == "Mode: active") {
            SDS011_init_flag = false;
        }
    }
}
