#include "Global.h"
#include "classes/IoTItem.h"

#include "DallasTemperature.h"
#include <OneWire.h>
#include <map>

//глобальные списки необходимы для хранения объектов об активных линиях 1-wire используемых разными датчиками из модуля. Ключ - номер пина
std::map<int, OneWire*> oneWireTemperatureArray;
std::map<int, DallasTemperature*> sensorsTemperatureArray;

class Ds18b20 : public IoTItem {
   private:
    //для работы библиотеки с несколькими линиями  необходимо обеспечить каждый экземпляр класса ссылками на объекты настроенные на эти линии
    OneWire* oneWire;
    DallasTemperature* sensors;

    //описание параметров передаваемых из настроек датчика из веба
    String _addr;
    int _pin;
    int _index;

   public:
    //=======================================================================================================
    // setup()
    //это аналог setup из arduino. Здесь вы можете выполнять методы инициализации сенсора.
    //Такие как ...begin и подставлять в них параметры полученные из web интерфейса.
    //Все параметры хранятся в перемененной parameters, вы можете прочитать любой параметр используя jsonRead функции:
    // jsonReadStr, jsonReadBool, jsonReadInt
    Ds18b20(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, "pin", _pin);
        jsonRead(parameters, "index", _index, false);
        jsonRead(parameters, "addr", _addr, false);

        //учитываем, что библиотека может работать с несколькими линиями на разных пинах, поэтому инициируем библиотеку, если линия ранее не использовалась
        if (oneWireTemperatureArray.find(_pin) == oneWireTemperatureArray.end()) {
            oneWire = new OneWire((uint8_t)_pin);
            sensors = new DallasTemperature();
            sensors->setOneWire(oneWire);
            sensors->begin();
            sensors->setResolution(12);

            oneWireTemperatureArray[_pin] = oneWire;
            sensorsTemperatureArray[_pin] = sensors;
        } else {
            oneWire = oneWireTemperatureArray[_pin];
            sensors = sensorsTemperatureArray[_pin];
        }
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
        //запускаем опрос измерений у всех датчиков на линии
        sensors->requestTemperatures();

        //Определяем адрес. Если парамтер addr не установлен, то узнаем адрес по индексу
        // TODO: понять как лучше. в текущей реализации адрес вычисляется каждый раз при опросе шины, это хорошо при отладке,
        // но при постоянном контакте и использовании правильнее генерировать адрес при инициализации модуля. Но тогда нужно перезагружать устройство при новом датчике
        DeviceAddress deviceAddress;
        if (_addr == "") {
            sensors->getAddress(deviceAddress, _index);
        } else {
            string2hex(_addr.c_str(), deviceAddress);
        }
        //получаем температуру по адресу
        value.valD = sensors->getTempC(deviceAddress);

        char addrStr[20] = "";
        hex2string(deviceAddress, 8, addrStr);

        if (value.valD != -127)
            regEvent(value.valD, "addr: " + String(addrStr));  //обязательный вызов для отправки результата работы
        else
            SerialPrint("E", "Sensor Ds18b20", "Error");
    }
    //=======================================================================================================

    ~Ds18b20(){};
};

//после замены названия сенсора, на функцию можно не обращать внимания
//если сенсор предполагает использование общего объекта библиотеки для нескольких экземпляров сенсора, то в данной функции необходимо предусмотреть
//создание и контроль соответствующих глобальных переменных
void* getAPI_Ds18b20(String subtype, String param) {
    if (subtype == F("Ds18b20")) {
        return new Ds18b20(param);
    } else {
        return nullptr;
    }
}
