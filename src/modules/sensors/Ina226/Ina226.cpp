/******************************************************************
  Used GyverINA Current Sensor
  Support for INA219 INA226
  https://github.com/GyverLibs/GyverINA

  adapted for version 4dev @Mit4bmw
 ******************************************************************/

#include "Global.h"
#include "classes/IoTItem.h"

#include <Wire.h>
#include <GyverINA.h>
#include <map>



// Структура для хранения настроек датчика
struct Ina226Value
{
    float shunt = 0;
    float maxV = 0;
};

// глобальные списки необходимы для хранения Модуля Настроек. Ключ - адрес
std::map<uint8_t, Ina226Value *> ina226SettingArray;

// глобальные списки необходимы для хранения объектов  используемых разными датчиками из модуля. Ключ - адрес
std::map<uint8_t, INA226 *> ina226Array;

// Функция инициализации библиотечного класса, возвращает Единстрвенный указать на библиотеку
INA226 *instanceIna226(uint8_t ADDR)
{
    /** default I2C address **/
    if (ADDR == 0)
        ADDR = 0x40; // 1000000 (A0+A1=GND)

    // учитываем, что библиотека может работать с несколькими линиями на разных пинах, поэтому инициируем библиотеку, если линия ранее не использовалась
    if (ina226Array.find(ADDR) == ina226Array.end())
    {
        if (ina226SettingArray.find(ADDR) != ina226SettingArray.end())
            ina226Array[ADDR] = new INA226(ina226SettingArray[ADDR]->shunt, ina226SettingArray[ADDR]->maxV, (uint8_t)ADDR);
        else
            ina226Array[ADDR] = new INA226(0.1f, 0.8f, (uint8_t)ADDR); // Стандартные значения для модуля INA226 (0.1 Ом, 0.8А, адрес 0x40)
        ina226Array[ADDR]->begin();
        //     ina226ValueArray[ADDR] = new Ina226Value;
    }
    return ina226Array[ADDR];
}

class Ina226voltage : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina226voltage(String parameters) : IoTItem(parameters)
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
        regEvent(instanceIna226(_addr)->getVoltage(), "Ina226voltage");
    }

    ~Ina226voltage(){};
};

class Ina226shuntvoltage : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina226shuntvoltage(String parameters) : IoTItem(parameters)
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
        regEvent(instanceIna226(_addr)->getShuntVoltage(), "Ina226shuntvoltage");
    }

    ~Ina226shuntvoltage(){};
};

class Ina226curr : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina226curr(String parameters) : IoTItem(parameters)
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
        regEvent(instanceIna226(_addr)->getCurrent(), "Ina226curr");
    }

    ~Ina226curr(){};
};

class Ina226Power : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina226Power(String parameters) : IoTItem(parameters)
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
        regEvent(instanceIna226(_addr)->getPower(), "Ina226power"); // TODO: найти способ понимания ошибки получения данных
    }

    ~Ina226Power(){};
};

class Ina226Setting : public IoTItem
{
private:
    uint8_t _addr = 0;
    int adjClbr = 0;
    int resol = 1;


public:
    Ina226Setting(String parameters) : IoTItem(parameters)
    {
        String sAddr;
        jsonRead(parameters, "addr", sAddr);
        jsonRead(parameters, "adjClbr", adjClbr);
        jsonRead(parameters, "resol", resol);


        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);
            
        ina226SettingArray[_addr] = new Ina226Value;
        jsonRead(parameters, "shunt", ina226SettingArray[_addr]->shunt);
        jsonRead(parameters, "maxV", ina226SettingArray[_addr]->maxV);
        
        instanceIna226(_addr)->adjCalibration(adjClbr);

        instanceIna226(_addr)->setAveraging(resol);   // Напряжение в 12ти битном режиме
    }

    void onModuleOrder(String &key, String &value)
    {
        if (key == "getClbr")
        {
            SerialPrint("i", F("Ina226"), "addr: " + String(_addr) + ", Value Calibration:" + instanceIna226(_addr)->getCalibration());
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (command == "sleep")
        {
            if (param.size() == 1)
            {
                if (param[0].valD == 0)
                    instanceIna226(_addr)->sleep(false);
                if (param[0].valD == 1)
                    instanceIna226(_addr)->sleep(true);
                return {};
            }
        }
        /*
        else if (command == "getCalibration")
        {
            SerialPrint("i", F("Ina226"), "addr: " + String(_addr) + ", Value Calibration:" + instanceIna226(_addr)->getCalibration());
            return {};
        }*/
        return {};
    }

    ~Ina226Setting(){};
};

void *getAPI_Ina226(String subtype, String param)
{
    if (subtype == F("Ina226curr"))
    {
        return new Ina226curr(param);
    }
    else if (subtype == F("Ina226shuntvoltage"))
    {
        return new Ina226shuntvoltage(param);
    }
    else if (subtype == F("Ina226power"))
    {
        return new Ina226Power(param);
    }
    else if (subtype == F("Ina226voltage"))
    {
        return new Ina226voltage(param);
    }
    else if (subtype == F("Ina226setting"))
    {
        return new Ina226Setting(param);
       // Ina226Setting *ptr = new Ina226Setting(param);
       // ina226SettingArray[ptr->getAddr()] = ptr;
       // return ptr;
    }
    else
    {
        return nullptr;
    }
}
