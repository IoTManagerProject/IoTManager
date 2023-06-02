/******************************************************************
  Used Adafruit BME280 Driver (Barometric Pressure Sensor)
  Support for BME280
  https://github.com/adafruit/Adafruit_BME280_Library
 ******************************************************************/

#include "Global.h"
#include "classes/IoTItem.h"

#include <Adafruit_BME280.h>
#include <map>

std::map<String, Adafruit_BME280 *> bmes;

class Bme280t : public IoTItem
{
private:
    Adafruit_BME280 *_bme;

public:
    Bme280t(Adafruit_BME280 *bme, String parameters) : IoTItem(parameters)
    {
        _bme = bme;
    }

    void doByInterval()
    {
        value.valD = _bme->readTemperature();
        if (value.valD != NAN && value.valD < 145)
            regEvent(value.valD, "Bme280t");
        else
            SerialPrint("E", "Sensor Bme280t", "Error", _id);
    }

    ~Bme280t(){};
};

class Bme280h : public IoTItem
{
private:
    Adafruit_BME280 *_bme;

public:
    Bme280h(Adafruit_BME280 *bme, String parameters) : IoTItem(parameters)
    {
        _bme = bme;
    }

    void doByInterval()
    {
        value.valD = _bme->readHumidity();
        if (value.valD != NAN && value.valD < 100)
            regEvent(value.valD, "Bme280h");
        else
            SerialPrint("E", "Sensor Bme280h", "Error", _id);
    }

    ~Bme280h(){};
};

class Bme280p : public IoTItem
{
private:
    Adafruit_BME280 *_bme;

public:
    Bme280p(Adafruit_BME280 *bme, String parameters) : IoTItem(parameters)
    {
        _bme = bme;
    }

    void doByInterval()
    {
        value.valD = _bme->readPressure();
        if (value.valD != NAN && value.valD > 0)
        {
            value.valD = value.valD / 1.333224 / 100;
            regEvent(value.valD, "Bme280p");
        }
        else
            SerialPrint("E", "Sensor Bme280p", "Error", _id);
    }

    ~Bme280p(){};
};

class Bme280dp : public IoTItem
{
private:
    Adafruit_BME280 *_bme;

public:
    Bme280dp(Adafruit_BME280 *bme, String parameters) : IoTItem(parameters)
    {
        _bme = bme;
    }

    void doByInterval()
    {

        float humi = _bme->readHumidity();
        float temp = _bme->readTemperature();

        if (temp != NAN && temp < 145 && humi != NAN && humi > 0 && humi < 100)
        {
            value.valD = (temp - (14.55 + 0.114 * temp) * (1 - (0.01 * humi)) - pow(((2.5 + 0.007 * temp) * (1 - (0.01 * humi))), 3) - (15.9 + 0.117 * temp) * pow((1 - (0.01 * humi)), 14));
            regEvent(value.valD, "Bme280dp");
        }
        else
            SerialPrint("E", "Sensor Bme280dp", "Error", _id);
    }

    ~Bme280dp(){};
};

void *getAPI_Bme280(String subtype, String param)
{
    if (subtype == F("Bme280t") || subtype == F("Bme280h") || subtype == F("Bme280p") || subtype == F("Bme280dp"))
    {
        String addr;
        jsonRead(param, "addr", addr);
        if (addr == "")
        {
            scanI2C();
            return nullptr;
        }

        if (bmes.find(addr) == bmes.end())
        {
            bmes[addr] = new Adafruit_BME280();
            bmes[addr]->begin(hexStringToUint8(addr));
        }

        if (subtype == F("Bme280t"))
        {
            return new Bme280t(bmes[addr], param);
        }
        else if (subtype == F("Bme280h"))
        {
            return new Bme280h(bmes[addr], param);
        }
        else if (subtype == F("Bme280p"))
        {
            return new Bme280p(bmes[addr], param);
        }
        else if (subtype == F("Bme280dp"))
        {
            return new Bme280dp(bmes[addr], param);
        }
    }

    return nullptr;
}
