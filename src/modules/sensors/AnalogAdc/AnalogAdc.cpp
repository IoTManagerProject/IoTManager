#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;

// Это файл сенсора, в нем осуществляется чтение сенсора.
// для добавления сенсора вам нужно скопировать этот файл и заменить в нем текст AnalogAdc на название вашего сенсора
// Название должно быть уникальным, коротким и отражать суть сенсора.

// ребенок       -       родитель
class AnalogAdc : public IoTItem
{
private:
    //=======================================================================================================
    // Секция переменных.
    // Это секция где Вы можете объявлять переменные и объекты arduino библиотек, что бы
    // впоследствии использовать их в loop и setup
    unsigned int _pin;
    unsigned int _avgSteps;
    unsigned int realSteps;
    float _adcAverage = 0;
    unsigned int _period = 0;
    unsigned long _lastSoundingMillis = 0;

public:
    //=======================================================================================================
    // setup()
    // это аналог setup из arduino. Здесь вы можете выполнять методы инициализации сенсора.
    // Такие как ...begin и подставлять в них параметры полученные из web интерфейса.
    // Все параметры хранятся в перемененной parameters, вы можете прочитать любой параметр используя jsonRead функции:
    // jsonReadStr, jsonReadBool, jsonReadInt
    AnalogAdc(String parameters) : IoTItem(parameters)
    {
        _pin = jsonReadInt(parameters, "pin");
        _avgSteps = jsonReadInt(parameters, "avgSteps");
        if (!_avgSteps)
        {
            jsonRead(parameters, F("int"), _interval, false);
        }
        else
        {
            _period = _interval / _avgSteps;
        }
        //   Serial.println("_period = " + String(_period));
    }

    //=======================================================================================================
    // doByInterval()
    // это аналог loop из arduino, но вызываемый каждые int секунд, заданные в настройках. Здесь вы должны выполнить чтение вашего сенсора
    // а затем выполнить regEvent - это регистрация произошедшего события чтения
    // здесь так же доступны все переменные из секции переменных, и полученные в setup
    // если у сенсора несколько величин то делайте несколько regEvent
    // не используйте delay - помните, что данный loop общий для всех модулей. Если у вас планируется длительная операция, постарайтесь разбить ее на порции
    // и выполнить за несколько тактов
    void doByInterval()
    {
        if (_avgSteps <= 1)
            value.valD = IoTgpio.analogRead(_pin);
        else
        {
            value.valD = _adcAverage / (float)realSteps;
            //Serial.print("value= " + String(value.valD) + " \t");
            //Serial.print("adcAverage = " + String(_adcAverage) + " \t");
            //Serial.println("realSteps = " + String(realSteps));
            realSteps = 0;
            _adcAverage = 0;
        }
        regEvent(value.valD, "AnalogAdc"); // обязательный вызов хотяб один
    }

    //=======================================================================================================
    // loop()
    // полный аналог loop() из arduino. Нужно помнить, что все модули имеют равный поочередный доступ к центральному loop(), поэтому, необходимо следить
    // за задержками в алгоритме и не создавать пауз. Кроме того, данная версия перегружает родительскую, поэтому doByInterval() отключается, если
    // не повторить механизм расчета интервалов.
    void loop()
    {
        if (_avgSteps > 1)
        {

            if (millis() > _lastSoundingMillis + _period)
            {
                realSteps++;
                int sounding = IoTgpio.analogRead(_pin);
                _adcAverage = _adcAverage + sounding; // IoTgpio.analogRead(_pin);
                _lastSoundingMillis = millis();
                //Serial.print("adc= " + String(sounding) + " \t");
                //Serial.println("_adcAverage = " + String(_adcAverage));
            }
        }
        IoTItem::loop();
    }

    ~AnalogAdc(){};
};

// после замены названия сенсора, на функцию можно не обращать внимания
// если сенсор предполагает использование общего объекта библиотеки для нескольких экземпляров сенсора, то в данной функции необходимо предусмотреть
// создание и контроль соответствующих глобальных переменных
void *getAPI_AnalogAdc(String subtype, String param)
{
    if (subtype == F("AnalogAdc"))
    {
        return new AnalogAdc(param);
    }
    else
    {
        return nullptr;
    }
}
