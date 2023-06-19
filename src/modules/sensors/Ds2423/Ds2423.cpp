
#include "Global.h"
#include "classes/IoTItem.h"

#include <DS2423.h>
#include <OneWire.h>
#include <map>

typedef uint8_t DeviceAddressDS2423[8];
// глобальные списки необходимы для хранения объектов об активных линиях 1-wire используемых разными датчиками из модуля. Ключ - номер пина
std::map<int, OneWire *> oneWireDS2423Array;

// Функция инициализации библиотечного класса, возвращает Единстрвенный указать на библиотеку
OneWire *instanceOneWire_DS2423(uint8_t ONE_WIRE_PIN)
{
    // учитываем, что библиотека может работать с несколькими линиями на разных пинах, поэтому инициируем библиотеку, если линия ранее не использовалась
    if (oneWireDS2423Array.find(ONE_WIRE_PIN) == oneWireDS2423Array.end())
        oneWireDS2423Array[ONE_WIRE_PIN] = new OneWire((uint8_t)ONE_WIRE_PIN);
    return oneWireDS2423Array[ONE_WIRE_PIN];
}

// Определяем адрес.
bool getDeviceAddressDS2423(uint8_t pin, uint8_t *deviceAddress, int index)
{
    OneWire *_wire = instanceOneWire_DS2423(pin);
    uint8_t depth = 0;
    _wire->reset_search();
    while (depth <= index && _wire->search(deviceAddress))
    {
        if (depth == index && _wire->crc8((uint8_t *)deviceAddress, 7) == deviceAddress[7])
            return true;
        depth++;
    }
    return false;
}


class Ds2423a : public IoTItem
{
private:
    // описание параметров передаваемых из настроек датчика из веба
    String _addr;
    int _pin;
    int _index;
    DS2423 *ds2423;
    DeviceAddressDS2423 _deviceAddress;

public:
    Ds2423a(String parameters) : IoTItem(parameters)
    {

        jsonRead(parameters, "pin", _pin);
        jsonRead(parameters, "index", _index, false);
        jsonRead(parameters, "addr", _addr, false);

        // Определяем адрес. Если параметр addr не установлен, то узнаем адрес по индексу
        if (_addr == "")
        {
            if (getDeviceAddressDS2423(_pin, _deviceAddress, _index))
            {
                char addrStr[20] = "";
                hex2string(_deviceAddress, 8, addrStr);
                SerialPrint("I", "Sensor " + (String)_id, "index: " + (String)_index + " addr: " + String(addrStr));
            }
            else
            {
                SerialPrint("E", "Sensor " + (String)_id, "index: " + (String)_index + " addres not search");
            }
        }
        else
        {
            string2hex(_addr.c_str(), _deviceAddress);
        }

        ds2423 = new DS2423(instanceOneWire_DS2423(_pin), _deviceAddress);
        ds2423->begin(DS2423_COUNTER_A | DS2423_COUNTER_B);

    }

    void doByInterval()
    {
        ds2423->update(); 
        if (ds2423->isError())
        {
            Serial.println("Error reading counter");
        }
        else
        {
            value.valD = ds2423->getCount(DS2423_COUNTER_A);
            // if (value.valD != -127)
            regEvent(value.valD, "Ds2423a"); // обязательный вызов для отправки результата работы
            // else
            //   SerialPrint("E", "Sensor Ds2423a", "Error");
        }
    }
    //=======================================================================================================

    ~Ds2423a(){};
};

class Ds2423b : public IoTItem
{
private:
    // описание параметров передаваемых из настроек датчика из веба
    String _addr;
    int _pin;
    int _index;
    DS2423 *ds2423;
    DeviceAddressDS2423 _deviceAddress;

public:
    Ds2423b(String parameters) : IoTItem(parameters)
    {
        jsonRead(parameters, "pin", _pin);
        jsonRead(parameters, "index", _index, false);
        jsonRead(parameters, "addr", _addr, false);

        // Определяем адрес. Если параметр addr не установлен, то узнаем адрес по индексу
        if (_addr == "")
        {
            if (getDeviceAddressDS2423(_pin, _deviceAddress, _index))
            {
                char addrStr[20] = "";
                hex2string(_deviceAddress, 8, addrStr);
                SerialPrint("I", "Sensor " + (String)_id, "index: " + (String)_index + " addr: " + String(addrStr));
            }
            else
            {
                SerialPrint("E", "Sensor " + (String)_id, "index: " + (String)_index + " addres not search");
            }
        }
        else
        {
            string2hex(_addr.c_str(), _deviceAddress);
        }

        ds2423 = new DS2423(instanceOneWire_DS2423(_pin), _deviceAddress);
        ds2423->begin(DS2423_COUNTER_A | DS2423_COUNTER_B);
    }

    void doByInterval()
    {
        ds2423->update();

        if (ds2423->isError())
        {
            Serial.println("Error reading counter");
        }
        else
        {
            // запускаем опрос измерений у всех датчиков на линии
            value.valD = ds2423->getCount(DS2423_COUNTER_B);
            //  if (value.valD != -127)
            regEvent(value.valD, "Ds2423b"); // обязательный вызов для отправки результата работы
            //  else
            //   SerialPrint("E", "Sensor Ds2423b", "Error");
        }
    }
    //=======================================================================================================

    ~Ds2423b(){};
};

void *getAPI_Ds2423(String subtype, String param)
{
    if (subtype == F("Ds2423a"))
    {
        return new Ds2423a(param);
    }
    else if (subtype == F("Ds2423b"))
    {
        return new Ds2423b(param);
    }
    else
    {
        return nullptr;
    }
}
