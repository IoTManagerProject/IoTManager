#include "Global.h"
#include "classes/IoTItem.h"

#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
// #define RADAR_RX_PIN 32
// #define RADAR_TX_PIN 33

#include <ld2410.h>
// ld2410 radar;

ld2410 *radar = nullptr;

ld2410 *instanceradar(uint8_t _rx, uint8_t _tx)
{
    if (!radar)
    { // Если библиотека ранее инициализировалась, т о просто вернем указатель
      // Инициализируем библиотеку
        radar = new ld2410();
        //    radar->begin(); // При необходимости делаем begin библиотеке

        RADAR_SERIAL.begin(256000, SERIAL_8N1, _rx, _tx); // UART for monitoring the radar

        if (radar->begin(RADAR_SERIAL))
        {
            SerialPrint("i", "LD2410", "Radar sensor подключен");
            SerialPrint("i", "LD2410", "Connect LD2410 radar TX to GPIO: " + String(_rx));
            SerialPrint("i", "LD2410", "Connect LD2410 radar RX to GPIO: " + String(_tx));
            SerialPrint("i", "LD2410", "LD2410 firmware version: " + String(radar->firmware_major_version));
            SerialPrint("i", "LD2410", "LD2410 firmware version: " + String(radar->firmware_minor_version));
            SerialPrint("i", "LD2410", "LD2410 firmware version: " + String(radar->firmware_bugfix_version));
        }
        else
        {
            SerialPrint("i", "LD2410", "Radar sensor не подключен");
        }
    }
    return radar;
}

int presence = 0;
int presence_last = 0;
int movement = 0;
int movement_last = 0;
int distance = 0;
int distance_last = 0;
int energy = 0;
int energy_last = 0;
int err = 0;

uint32_t lastReadingP = 0;
uint32_t lastReadingM = 0;
uint32_t lastReadingD = 0;
uint32_t lastReadingE = 0;
bool radarConnected = false;

// датчик присутствия
class ld2410m : public IoTItem
{
private:
    int _ticker;
    uint8_t _rx, _tx;

public:
    //=======================================================================================================
    // setup()

    ld2410m(String parameters) : IoTItem(parameters)
    {
        jsonRead(parameters, "ticker", _ticker);
        String tmp;
        jsonRead(parameters, "RX", tmp);
        _rx = tmp.toInt();
        jsonRead(parameters, "TX", tmp);
        _tx = tmp.toInt();
    }

    void loop()
    {
        instanceradar(_rx, _tx)->read();
        if (instanceradar(_rx, _tx)->isConnected() && millis() - lastReadingP > 1000) // Report every 1000ms
        {
            lastReadingP = millis();
            err = 0;
            presence = instanceradar(_rx, _tx)->presenceDetected();
            if (instanceradar(_rx, _tx)->stationaryTargetDetected())
            {
                movement = 0;
                distance = instanceradar(_rx, _tx)->stationaryTargetDistance();
                energy = instanceradar(_rx, _tx)->stationaryTargetEnergy();
            }
            if (instanceradar(_rx, _tx)->movingTargetDetected())
            {
                movement = 1;
                distance = instanceradar(_rx, _tx)->movingTargetDistance();
                energy = instanceradar(_rx, _tx)->movingTargetEnergy();
            }

            if (presence != presence_last)
            {
                value.valD = int(presence);
                regEvent(value.valD, "ld2410m");
                presence_last = presence;
                //    SerialPrint("i", "LD2410", "присутствие: " + String(int(presence)));
            }

            if (presence && movement != movement_last)
            {
                value.valD = int(presence);
                regEvent(value.valD, "ld2410m");
            }
            if (presence && _ticker)
            {
                value.valD = int(presence);
                regEvent(value.valD, "ld2410m");
                presence_last = presence;
            }
        }
        if (!instanceradar(_rx, _tx)->isConnected())
        {
            if (err == 0)
            {
                //    value.valD = "Err";
                //    regEvent(value.valD, "ld2410m");
                //    SerialPrint("E", "LD2410", "Radar sensor ошибка получения данных");
            }
            err = 1;
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (command == "setmaxvalues")
        {
            if (param[0].isDecimal && param[1].isDecimal && param[2].isDecimal)
            {
                uint8_t newMovingMaxDistance = param[0].valD;
                uint8_t newStationaryMaxDistance = param[1].valD;
                uint16_t inactivityTimer = param[2].valD;
                if (newMovingMaxDistance > 0 && newStationaryMaxDistance > 0 && newMovingMaxDistance <= 8 && newStationaryMaxDistance <= 8)
                {
                    if (instanceradar(_rx, _tx)->setMaxValues(newMovingMaxDistance, newStationaryMaxDistance, inactivityTimer))
                    {
                        SerialPrint("i", "LD2410", "Параметры установлены. Перезапустите датчик");
                    }
                    else
                    {
                        SerialPrint("E", "LD2410", "Ошибка записи параметров.");
                    }
                }
                else
                {
                    SerialPrint("E", "LD2410", "Заданы не корректные параметры. Диапазон от 1 до 8 ");
                }
            }
        }
        if (command == "setsensitivity")
        {
            if (param[0].isDecimal && param[1].isDecimal && param[2].isDecimal)
            {
                uint8_t gate = param[0].valD;
                uint8_t motionSensitivity = param[1].valD;
                uint16_t stationarySensitivity = param[2].valD;
                if (motionSensitivity >= 0 && stationarySensitivity >= 0 && motionSensitivity <= 100 && stationarySensitivity <= 100)
                {
                    if (instanceradar(_rx, _tx)->setGateSensitivityThreshold(gate, motionSensitivity, stationarySensitivity))
                    {
                        SerialPrint("i", "LD2410", "Параметры установлены. Перезапустите датчик");
                    }
                    else
                    {
                        SerialPrint("E", "LD2410", "Ошибка записи параметров.");
                    }
                }
                else
                {
                    SerialPrint("E", "LD2410", "Заданы не корректные параметры. Диапазон от 1 до 100 ");
                }
            }
        }
        if (command == "enableengineeringmode")
        {
            if (instanceradar(_rx, _tx)->requestStartEngineeringMode())
            {
                SerialPrint("i", "LD2410", "Параметры установлены. Перезапустите датчик");
            }
            else
            {
                SerialPrint("E", "LD2410", "Ошибка записи параметров.");
            }
        }
        if (command == "disableengineeringmode")
        {
            if (instanceradar(_rx, _tx)->requestEndEngineeringMode())
            {
                SerialPrint("i", "LD2410", "Параметры установлены. Перезапустите датчик");
            }
            else
            {
                SerialPrint("E", "LD2410", "Ошибка записи параметров.");
            }
        }
        if (command == "restart")
        {
            if (instanceradar(_rx, _tx)->requestRestart())
            {
                SerialPrint("i", "LD2410", "Датчик перезапущен");
            }
            else
            {
                SerialPrint("E", "LD2410", "Ошибка перезапуска датчика.");
            }
        }
        if (command == "readversion")
        {
            if (instanceradar(_rx, _tx)->requestFirmwareVersion())
            {
                SerialPrint("i", "LD2410", (String)instanceradar(_rx, _tx)->firmware_major_version);
                SerialPrint("i", "LD2410", (String)instanceradar(_rx, _tx)->firmware_minor_version);
            }
            else
            {
                SerialPrint("E", "LD2410", "Ошибка получения версии.");
            }
        }
        if (command == "factoryreset")
        {
            if (instanceradar(_rx, _tx)->requestFactoryReset())
            {
                SerialPrint("i", "LD2410", "Параметры установлены. Перезапустите датчик");
            }
            else
            {
                SerialPrint("E", "LD2410", "Ошибка сброса параметров.");
            }
        }
    }
    ~ld2410m()
    {
        delete radar;
        radar = nullptr;
    };
};
//---------------------------------------------------------------------------

// движение / стационарное - присутствие
class ld2410t : public IoTItem
{
private:
    uint8_t _rx, _tx;

public:
    //=======================================================================================================
    // setup()

    ld2410t(String parameters) : IoTItem(parameters)
    {
        String tmp;
        jsonRead(parameters, "RX", tmp);
        _rx = tmp.toInt();
        jsonRead(parameters, "TX", tmp);
        _tx = tmp.toInt();
    }

    void loop()
    {
        instanceradar(_rx, _tx)->read();
        if (instanceradar(_rx, _tx)->isConnected() && millis() - lastReadingM > 1000) // Report every 1000ms
        {
            lastReadingM = millis();
            err = 0;
            presence = instanceradar(_rx, _tx)->presenceDetected();
            if (instanceradar(_rx, _tx)->stationaryTargetDetected())
            {
                movement = 0;
                //    distance = instanceradar(_rx, _tx)->stationaryTargetDistance();
                //    energy = instanceradar(_rx, _tx)->stationaryTargetEnergy();
            }
            if (instanceradar(_rx, _tx)->movingTargetDetected())
            {
                movement = 1;
                //    distance = instanceradar(_rx, _tx)->movingTargetDistance();
                //    energy = instanceradar(_rx, _tx)->movingTargetEnergy();
            }
        }
        if (presence == 0 && movement_last != 0)
        {
            movement = 0;
        }
        if (movement != movement_last)
        {
            value.valD = int(movement);
            regEvent(value.valD, "ld2410t");
            movement_last = movement;
        }
    }

    ~ld2410t()
    {
        delete radar;
        radar = nullptr;
    };
};
//---------------------------------------------------------------------------

// дистанция
class ld2410d : public IoTItem
{
private:
    uint8_t _rx, _tx;

public:
    //=======================================================================================================
    // setup()

    ld2410d(String parameters) : IoTItem(parameters)
    {
        String tmp;
        jsonRead(parameters, "RX", tmp);
        _rx = tmp.toInt();
        jsonRead(parameters, "TX", tmp);
        _tx = tmp.toInt();
    }

    void loop()
    {
        instanceradar(_rx, _tx)->read();
        if (instanceradar(_rx, _tx)->isConnected() && millis() - lastReadingD > 1000) // Report every 1000ms
        {
            lastReadingD = millis();
            err = 0;
            presence = instanceradar(_rx, _tx)->presenceDetected();
            if (instanceradar(_rx, _tx)->stationaryTargetDetected())
            {
                //    movement = 0;
                distance = instanceradar(_rx, _tx)->stationaryTargetDistance();
                //    energy = instanceradar(_rx, _tx)->stationaryTargetEnergy();
            }
            if (instanceradar(_rx, _tx)->movingTargetDetected())
            {
                //    movement = 1;
                distance = instanceradar(_rx, _tx)->movingTargetDistance();
                //    energy = instanceradar(_rx, _tx)->movingTargetEnergy();
            }
            if (presence == 0 && distance_last != 0)
            {
                distance = 0;
            }
            if (distance != distance_last)
            {
                value.valD = int(distance);
                regEvent(value.valD, "ld2410d");
                distance_last = distance;
            }
        }
    }
    ~ld2410d()
    {
        delete radar;
        radar = nullptr;
    };
};
//---------------------------------------------------------------------------

// энергия
class ld2410e : public IoTItem
{
private:
    uint8_t _rx, _tx;

public:
    //=======================================================================================================
    // setup()

    ld2410e(String parameters) : IoTItem(parameters)
    {
        String tmp;
        jsonRead(parameters, "RX", tmp);
        _rx = tmp.toInt();
        jsonRead(parameters, "TX", tmp);
        _tx = tmp.toInt();
    }

    void loop()
    {
        instanceradar(_rx, _tx)->read();
        if (instanceradar(_rx, _tx)->isConnected() && millis() - lastReadingE > 1000) // Report every 1000ms
        {
            lastReadingE = millis();
            err = 0;
            presence = instanceradar(_rx, _tx)->presenceDetected();
            if (instanceradar(_rx, _tx)->stationaryTargetDetected())
            {
                //    movement = 0;
                // distance = instanceradar(_rx, _tx)->stationaryTargetDistance();
                energy = instanceradar(_rx, _tx)->stationaryTargetEnergy();
            }
            if (instanceradar(_rx, _tx)->movingTargetDetected())
            {
                //    movement = 1;
                // distance = instanceradar(_rx, _tx)->movingTargetDistance();
                energy = instanceradar(_rx, _tx)->movingTargetEnergy();
            }
            if (presence == 0 && energy_last != 0)
            {
                energy = 0;
            }
            if (energy != energy_last)
            {
                value.valD = int(energy);
                regEvent(value.valD, "ld2410e");
                // value = Value;
                // regEvent((String)(int)value.valD, F("ld2410e"), false, true);
                // SerialPrint("i", "LD2410", "Radar sensor энергия: " + String(energy));
                energy_last = energy;
            }
        }
    }
    ~ld2410e()
    {
        delete radar;
        radar = nullptr;
    };
};
//---------------------------------------------------------------------------
void *getAPI_ld2410(String subtype, String param)
{
    if (subtype == F("ld2410m"))
    {
        return new ld2410m(param);
    }
    else if (subtype == F("ld2410t"))
    {
        return new ld2410t(param);
    }
    else if (subtype == F("ld2410d"))
    {
        return new ld2410d(param);
    }
    else if (subtype == F("ld2410e"))
    {
        return new ld2410e(param);
    }
    else
    {
        return nullptr;
    }
}
