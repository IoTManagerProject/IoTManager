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
struct Ina219Value
{
    float shunt = 0;
    float maxV = 0;
};

// глобальные списки необходимы для хранения Модуля Настроек. Ключ - адрес
std::map<uint8_t, Ina219Value *> ina219SettingArray;

// глобальные списки необходимы для хранения объектов  используемых разными датчиками из модуля. Ключ - адрес
std::map<uint8_t, INA219 *> ina219Array;

// Функция инициализации библиотечного класса, возвращает Единстрвенный указать на библиотеку
INA219 *instanceIna219(uint8_t ADDR)
{
    /** default I2C address **/
    if (ADDR == 0)
        ADDR = 0x40; // 1000000 (A0+A1=GND)

    // учитываем, что библиотека может работать с несколькими линиями на разных пинах, поэтому инициируем библиотеку, если линия ранее не использовалась
    if (ina219Array.find(ADDR) == ina219Array.end())
    {
        if (ina219SettingArray.find(ADDR) != ina219SettingArray.end())
            ina219Array[ADDR] = new INA219(ina219SettingArray[ADDR]->shunt, ina219SettingArray[ADDR]->maxV, (uint8_t)ADDR);
        else
            ina219Array[ADDR] = new INA219(0.1f, 3.2f, (uint8_t)ADDR); // Стандартные значения для модуля INA219 (0.1 Ом, 3.2А, адрес 0x40)
        ina219Array[ADDR]->begin();
        //     ina219ValueArray[ADDR] = new Ina219Value;
    }
    return ina219Array[ADDR];
}

class Ina219voltage : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina219voltage(String parameters) : IoTItem(parameters)
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
        regEvent(instanceIna219(_addr)->getVoltage(), "Ina219voltage");
    }

    ~Ina219voltage(){};
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
        regEvent(instanceIna219(_addr)->getShuntVoltage(), "Ina219shuntvoltage");
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
        regEvent(instanceIna219(_addr)->getCurrent(), "Ina219curr");
    }

    ~Ina219curr(){};
};

class Ina219Power : public IoTItem
{
private:
    uint8_t _addr = 0;

public:
    Ina219Power(String parameters) : IoTItem(parameters)
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
        regEvent(instanceIna219(_addr)->getPower(), "Ina219power"); // TODO: найти способ понимания ошибки получения данных
    }

    ~Ina219Power(){};
};

class Ina219Setting : public IoTItem
{
private:
    uint8_t _addr = 0;
    int adjClbr = 0;
    int resol = 1;


public:
    Ina219Setting(String parameters) : IoTItem(parameters)
    {
        String sAddr;
        jsonRead(parameters, "addr", sAddr);
        jsonRead(parameters, "adjClbr", adjClbr);
        jsonRead(parameters, "resol", resol);


        if (sAddr == "")
            scanI2C();
        else
            _addr = hexStringToUint8(sAddr);

        ina219SettingArray[_addr] = new Ina219Value;
        jsonRead(parameters, "shunt", ina219SettingArray[_addr]->shunt);
        jsonRead(parameters, "maxV", ina219SettingArray[_addr]->maxV);

        instanceIna219(_addr)->adjCalibration(adjClbr);
        if (resol == 1)
            resol = 0b0011;
        else
            resol += 0b0111;

        instanceIna219(_addr)->setResolution(INA219_VBUS, resol);   // Напряжение в 12ти битном режиме
        instanceIna219(_addr)->setResolution(INA219_VSHUNT, resol); // Ток в 12ти битном режиме
    }

    void onModuleOrder(String &key, String &value)
    {
        if (key == "getClbr")
        {
            SerialPrint("i", F("Ina219"), "addr: " + String(_addr) + ", Value Calibration:" + instanceIna219(_addr)->getCalibration());
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (command == "sleep")
        {
            if (param.size() == 1)
            {
                if (param[0].valD == 0)
                    instanceIna219(_addr)->sleep(false);
                if (param[0].valD == 1)
                    instanceIna219(_addr)->sleep(true);
                return {};
            }
        }
        /*
        else if (command == "getCalibration")
        {
            SerialPrint("i", F("Ina219"), "addr: " + String(_addr) + ", Value Calibration:" + instanceIna219(_addr)->getCalibration());
            return {};
        }*/
        return {};
    }

    ~Ina219Setting(){};
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
    else if (subtype == F("Ina219power"))
    {
        return new Ina219Power(param);
    }
    else if (subtype == F("Ina219voltage"))
    {
        return new Ina219voltage(param);
    }
    else if (subtype == F("Ina219setting"))
    {
        return new Ina219Setting(param);
       // Ina219Setting *ptr = new Ina219Setting(param);
       // ina219SettingArray[ptr->getAddr()] = ptr;
       // return ptr;
    }
    else
    {
        return nullptr;
    }
}
