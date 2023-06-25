/******************************************************************
  Used Adafruit INA219 Current Sensor
  Support for INA219
  https://github.com/adafruit/Adafruit_INA219

  adapted for version 4dev @Serghei63
 ******************************************************************/

#include "Global.h"
#include "classes/IoTItem.h"

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <map>

// Adafruit_INA219 ina219;
// Структура для хранения данных с датчика
struct Ina219Value
{
    float shuntvoltage = 0;
    float busvoltage = 0;
};
// глобальные списки необходимы для хранения данных, полученных разными датчиками из модуля. Ключ - адрес
std::map<uint8_t, Ina219Value *> ina219ValueArray;

// глобальные списки необходимы для хранения объектов  используемых разными датчиками из модуля. Ключ - адрес
std::map<uint8_t, Adafruit_INA219 *> ina219Array;

// Функция инициализации библиотечного класса, возвращает Единстрвенный указать на библиотеку
Adafruit_INA219 *instanceIna219(uint8_t ADDR)
{
    /** default I2C address **/
    if (ADDR == 0)
        ADDR = INA219_ADDRESS; // 1000000 (A0+A1=GND)

    // учитываем, что библиотека может работать с несколькими линиями на разных пинах, поэтому инициируем библиотеку, если линия ранее не использовалась
    if (ina219Array.find(ADDR) == ina219Array.end())
    {
        ina219Array[ADDR] = new Adafruit_INA219((uint8_t)ADDR);
        ina219Array[ADDR]->begin();
        ina219ValueArray[ADDR] = new Ina219Value;
    }
    return ina219Array[ADDR];
}

/*
float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;
*/
// shuntvoltage = ina219.getShuntVoltage_mV();       // Получение напряжение на шунте
// busvoltage = ina219.getBusVoltage_V();            // Получение значение напряжения V
// current_mA = ina219.getCurrent_mA();              // Получение значение тока в мА
// power_mW = ina219.getPower_mW();                  // Получение значение мощности
// loadvoltage = busvoltage + (shuntvoltage / 1000); // Расчет напряжение на нагрузки

class Ina219loadvoltage : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina219loadvoltage(String parameters) : IoTItem(parameters)
    {
        String sAddr;
        jsonRead(parameters, "addr", sAddr);
        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);
    }

    void doByInterval()
    {
        value.valD = ina219ValueArray[_addr]->busvoltage + (ina219ValueArray[_addr]->shuntvoltage / 1000);
        regEvent(value.valD, "Ina219loadvoltage");
    }

    ~Ina219loadvoltage(){};
};

class Ina219busvoltage : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina219busvoltage(String parameters) : IoTItem(parameters)
    {
        String sAddr;
        jsonRead(parameters, "addr", sAddr);
        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);
    }

    void doByInterval()
    {
        ina219ValueArray[_addr]->busvoltage = instanceIna219(_addr)->getBusVoltage_V();
        value.valD = ina219ValueArray[_addr]->busvoltage;
        regEvent(value.valD, "Ina219busvoltage");
    }

    ~Ina219busvoltage(){};
};

class Ina219shuntvoltage : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina219shuntvoltage(String parameters) : IoTItem(parameters)
    {
        String sAddr;
        jsonRead(parameters, "addr", sAddr);
        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);
    }
    void doByInterval()
    {
        ina219ValueArray[_addr]->shuntvoltage = instanceIna219(_addr)->getShuntVoltage_mV();
        value.valD = ina219ValueArray[_addr]->shuntvoltage;
        regEvent(value.valD, "Ina219shuntvoltage");
    }

    ~Ina219shuntvoltage(){};
};

class Ina219curr : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina219curr(String parameters) : IoTItem(parameters)
    {
        String sAddr;
        jsonRead(parameters, "addr", sAddr);
        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);
    }
    void doByInterval()
    {
        value.valD = instanceIna219(_addr)->getCurrent_mA();
        regEvent(value.valD, "Ina219curr");
    }

    ~Ina219curr(){};
};

class Ina219Power_mW : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina219Power_mW(String parameters) : IoTItem(parameters)
    {
        String sAddr;
        jsonRead(parameters, "addr", sAddr);
        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);
    }
    void doByInterval()
    {
        value.valD = instanceIna219(_addr)->getPower_mW();
        regEvent(value.valD, "Ina219power"); // TODO: найти способ понимания ошибки получения данных
    }

    ~Ina219Power_mW(){};
};
void *getAPI_Ina219(String subtype, String param)
{
    if (subtype == F("Ina219curr"))
    {
        return new Ina219curr(param);
    }
    else if (subtype == F("Ina219shuntvoltage"))
    {
        return new Ina219shuntvoltage(param);
    }
    else if (subtype == F("Ina219power_mW"))
    {
        return new Ina219Power_mW(param);
    }
    else if (subtype == F("Ina219busvoltage"))
    {
        return new Ina219busvoltage(param);
    }
    else if (subtype == F("Ina219loadvoltage"))
    {
        return new Ina219loadvoltage(param);
    }
    else
    {
        return nullptr;
    }
}
