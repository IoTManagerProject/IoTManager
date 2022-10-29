#include "Global.h"
#include "classes/IoTItem.h"

#include "DallasTemperature.h"
#include <OneWire.h>
#include <map>

// глобальные списки необходимы для хранения объектов об активных линиях 1-wire используемых разными датчиками из модуля. Ключ - номер пина
std::map<int, OneWire*> oneWireTemperatureArray;


class Ds18b20 : public IoTItem {
   private:
    //для работы библиотеки с несколькими линиями  необходимо обеспечить каждый экземпляр класса ссылками на объекты настроенные на эти линии
    OneWire* _oneWire;
    DallasTemperature* _sensor;
    DeviceAddress _deviceAddress;

   public:
    //=======================================================================================================
    // setup()
    // это аналог setup из arduino. Здесь вы можете выполнять методы инициализации сенсора.
    // Такие как ...begin и подставлять в них параметры полученные из web интерфейса.
    // Все параметры хранятся в перемененной parameters, вы можете прочитать любой параметр используя jsonRead функции:
    // jsonReadStr, jsonReadBool, jsonReadInt
    Ds18b20(String parameters) : IoTItem(parameters) {
        int index, pin;
        String addr;

        jsonRead(parameters, "pin", pin);
        jsonRead(parameters, "index", index, false);
        jsonRead(parameters, "addr", addr, false);

        //учитываем, что библиотека может работать с несколькими линиями на разных пинах, поэтому инициируем библиотеку, если линия ранее не использовалась
        if (oneWireTemperatureArray.find(pin) == oneWireTemperatureArray.end()) {
            _oneWire = new OneWire((uint8_t)pin);
            oneWireTemperatureArray[pin] = _oneWire;
        } else {
            _oneWire = oneWireTemperatureArray[pin];
        }

        _sensor = new DallasTemperature(_oneWire);
        _sensor->begin();
        //sensors->setResolution(12);

        //Определяем адрес. Если параметр addr не установлен, то узнаем адрес по индексу
        if (addr == "") {
            _sensor->getAddress(_deviceAddress, index);
            char addrStr[20] = "";
            hex2string(_deviceAddress, 8, addrStr);
            SerialPrint("I", "Sensor " + (String)_id, "index: " + (String)index + " addr: " + String(addrStr));
        } else {
            string2hex(addr.c_str(), _deviceAddress);
        }
    }
    //=======================================================================================================
    // doByInterval()
    // это аналог loop из arduino, но вызываемый каждые int секунд, заданные в настройках. Здесь вы должны выполнить чтение вашего сенсора
    // а затем выполнить regEvent - это регистрация произошедшего события чтения
    // здесь так же доступны все переменные из секции переменных, и полученные в setup
    // если у сенсора несколько величин то делайте несколько regEvent
    // не используйте delay - помните, что данный loop общий для всех модулей. Если у вас планируется длительная операция, постарайтесь разбить ее на порции
    // и выполнить за несколько тактов
    void doByInterval() {
        //запускаем опрос измерений у всех датчиков на линии
        _sensor->requestTemperatures();

        //получаем температуру по адресу
        value.valD = _sensor->getTempC(_deviceAddress);

        if (value.valD != DEVICE_DISCONNECTED_C)
            regEvent(value.valD, "");  //обязательный вызов для отправки результата работы
        else
            SerialPrint("E", "Sensor Ds18b20", "Error", _id);
    }
    //=======================================================================================================

    ~Ds18b20() {
        if (_sensor) delete _sensor;
    };
};

// после замены названия сенсора, на функцию можно не обращать внимания
// если сенсор предполагает использование общего объекта библиотеки для нескольких экземпляров сенсора, то в данной функции необходимо предусмотреть
// создание и контроль соответствующих глобальных переменных
void* getAPI_Ds18b20(String subtype, String param) {
    if (subtype == F("Ds18b20")) {
        return new Ds18b20(param);
    } else {
        return nullptr;
    }
}
