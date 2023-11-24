#include "Global.h"
#include "classes/IoTItem.h"

#include <SensirionI2CScd4x.h>
#include <SensirionCore.h>

#include <Wire.h>

// SensirionI2CScd4x scd4x;

SensirionI2CScd4x *scd4x = nullptr; // create an object of the CSD40 class
char errorMessageScd4x[256];
uint16_t errorCodeScd4x;

void printUint16Hex(uint16_t value)
{
    Serial.print(value < 4096 ? "0" : "");
    Serial.print(value < 256 ? "0" : "");
    Serial.print(value < 16 ? "0" : "");
    Serial.print(value, HEX);
}

void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2)
{
    Serial.print("Serial: 0x");
    printUint16Hex(serial0);
    printUint16Hex(serial1);
    printUint16Hex(serial2);
    Serial.println();
}

// Функция инициализации библиотечного класса, возвращает Единстрвенный указать на библиотеку
SensirionI2CScd4x *instanceScd4x()
{
    if (!scd4x)
    { // Если библиотека ранее инициализировалась, т о просто вернем указатель
        // Инициализируем библиотеку
        scd4x = new SensirionI2CScd4x();
        Wire.begin();
        scd4x->begin(Wire);

        //Останавливаем периодический опрос датчика вбиблиотеке для запроса Сер.номера (на всякий случай)
        // stop potentially previously started measurement
        errorCodeScd4x = instanceScd4x()->stopPeriodicMeasurement();
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
            Serial.println(errorMessageScd4x);
        }

        //Запрашиваем и выводим серийный номер датчика
        uint16_t serial0;
        uint16_t serial1;
        uint16_t serial2;
        errorCodeScd4x = instanceScd4x()->getSerialNumber(serial0, serial1, serial2);
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute getSerialNumber(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
        }
        else
        {
            printSerialNumber(serial0, serial1, serial2);
        }

        //Обратно стартуем периодический опрос датчика библиотекой (по описанию библиотеки каждые 5сек)
        // Start Measurement
        errorCodeScd4x = instanceScd4x()->startPeriodicMeasurement();
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute startPeriodicMeasurement(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
        }

        Serial.println("Waiting for first measurement... ");
    }
    return scd4x;
}

class Scd40co2 : public IoTItem
{
private:
bool lowPeriodic = true;
bool autoCalibration = true;
//int targetCo2 = 0;
public:
    Scd40co2(String parameters) : IoTItem(parameters)
    {
        jsonRead(parameters, F("lowPeriodic"), lowPeriodic);
        jsonRead(parameters, F("autoCalibration"), autoCalibration);  
 //       jsonRead(parameters, F("targetCo2"), targetCo2);        
        settingParameters();   
    }

    void doByInterval()
    {
        // Read Measurement
        uint16_t co2 = 0;
        float temperature = 0.0f;
        float humidity = 0.0f;
        bool isDataReady = false;
        //Запрашиваем библиотеку о готовности отправить запрос 
        errorCodeScd4x = instanceScd4x()->getDataReadyFlag(isDataReady);
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute getDataReadyFlag(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
            return;
        }
        if (!isDataReady)
        {
            return;
        }
        //Если все нормально забираем у библиотеки данные
        errorCodeScd4x = instanceScd4x()->readMeasurement(co2, temperature, humidity);
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute readMeasurement(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
        }
        else if (co2 == 0)
        {
            Serial.println("Invalid sample detected, skipping.");
        }
        else
        {
            Serial.print("Co2:");
            Serial.print(co2);
            Serial.print("\t");
            // Serial.print("Temperature:");
            // Serial.print(temperature);
            // Serial.print("\t");
            // Serial.print("Humidity:");
            // Serial.println(humidity);
        }
        // value.valD = scd4x.readMeasurement(temperature);
        value.valD = co2;
        if (value.valD < 5000)
            regEvent(value.valD, "Scd40co2");
        else
            SerialPrint("E", "Sensor Scd40co2", "Error", _id);
    }

    //Хук для обработки кнопки
    void onModuleOrder(String &key, String &value)
    {
        if (key == "Recalibration")//название кнопки btn-Recalibration
        {
            SerialPrint("i", F("Sensor Scd40co2"), "User run calibration, targetCo2: " + value);
            Recalibration(value.toInt());
        }
    }
    void Recalibration(int targetCo2)
    {
        //Останавливаем периодический опрос датчика вбиблиотеке для запроса Сер.номера (на всякий случай)
        // stop potentially previously started measurement
        errorCodeScd4x = instanceScd4x()->stopPeriodicMeasurement();
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
            Serial.println(errorMessageScd4x);
        }
        delay(500); // Из описания performForcedRecalibration  2. Stop periodic measurement. Wait 500 ms.
        uint16_t frcCorrection;
        errorCodeScd4x = instanceScd4x()->performForcedRecalibration(targetCo2, frcCorrection);

        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute performForcedRecalibration(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
        }
        else
        {
            Serial.println("performForcedRecalibration(): OK!");
            SerialPrint("i", F("Sensor Scd40co2"), "Calibration is OK, frcCorrection: " + String(frcCorrection));
        }

        //Обратно стартуем периодический опрос датчика библиотекой (по описанию библиотеки каждые 5сек)
        // Start Measurement
        errorCodeScd4x = instanceScd4x()->startPeriodicMeasurement();
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute startPeriodicMeasurement(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
        }

        Serial.println("Waiting for first measurement... ");        
    }

    void settingParameters()
    {
        //Останавливаем периодический опрос датчика вбиблиотеке для запроса Сер.номера (на всякий случай)
        // stop potentially previously started measurement
        errorCodeScd4x = instanceScd4x()->stopPeriodicMeasurement();
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
            Serial.println(errorMessageScd4x);
        }

        errorCodeScd4x = instanceScd4x()->startLowPowerPeriodicMeasurement();
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute startLowPowerPeriodicMeasurement(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
        }
        else
        {
            Serial.println("startLowPowerPeriodicMeasurement(): OK!");
        }

        errorCodeScd4x = instanceScd4x()->setAutomaticSelfCalibration((uint16_t)autoCalibration);
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute setAutomaticSelfCalibration(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
        }
        else
        {
            Serial.println("setAutomaticSelfCalibration(): OK!");
        }

        //Обратно стартуем периодический опрос датчика библиотекой (по описанию библиотеки каждые 5сек)
        // Start Measurement
        errorCodeScd4x = instanceScd4x()->startPeriodicMeasurement();
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute startPeriodicMeasurement(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
        }

        Serial.println("Waiting for first measurement... ");        
    }

    ~Scd40co2(){};
};

class Scd40t : public IoTItem
{
private:
    int offsetT; 
public:
    Scd40t(String parameters) : IoTItem(parameters)
    {
        jsonRead(parameters, F("offset"), offsetT);
        setTemperatureOffset();
    }

    void doByInterval()
    {
        // Read Measurement
        uint16_t co2 = 0;
        float temperature = 0.0f;
        float humidity = 0.0f;
        bool isDataReady = false;
        errorCodeScd4x = instanceScd4x()->getDataReadyFlag(isDataReady);
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute getDataReadyFlag(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
            return;
        }
        if (!isDataReady)
        {
            return;
        }
        errorCodeScd4x = instanceScd4x()->readMeasurement(co2, temperature, humidity);
        if (errorCodeScd4x)
        {
            Serial.print("errorCodeScd4x trying to execute readMeasurement(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
        }
        else if (co2 == 0)
        {
            Serial.println("Invalid sample detected, skipping.");
        }
        else
        {
            // Serial.print("Co2:");
            //  Serial.print(co2);
            // Serial.print("\t");
            Serial.print("Temperature:");
            Serial.print(temperature);
            Serial.print("\t");
            //  Serial.print("Humidity:");
            // Serial.println(humidity);
        }
        // value.valD = scd4x.readMeasurement(temperature);
        value.valD = temperature;
        if (value.valD < 124)
            regEvent(value.valD, "Scd40t");
        else
            SerialPrint("E", "Sensor Scd40t", "Error", _id);
    }

    void setTemperatureOffset()
    {
        //Останавливаем периодический опрос датчика вбиблиотеке для запроса Сер.номера (на всякий случай)
        // stop potentially previously started measurement
        errorCodeScd4x = instanceScd4x()->stopPeriodicMeasurement();
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
            Serial.println(errorMessageScd4x);
        }

        errorCodeScd4x = instanceScd4x()->setTemperatureOffset((uint16_t)offsetT);
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute setTemperatureOffset(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
        }
        else
        {
            Serial.println("setTemperatureOffset(): OK!");
        }

        //Обратно стартуем периодический опрос датчика библиотекой (по описанию библиотеки каждые 5сек)
        // Start Measurement
        errorCodeScd4x = instanceScd4x()->startPeriodicMeasurement();
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute startPeriodicMeasurement(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
        }

        Serial.println("Waiting for first measurement... ");        
    }

    ~Scd40t(){};
};

class Scd40h : public IoTItem
{
public:
    Scd40h(String parameters) : IoTItem(parameters)
    {
    }

    void doByInterval()
    {
         // Read Measurement
        uint16_t co2 = 0;
        float temperature = 0.0f;
        float humidity = 0.0f;
        bool isDataReady = false;
        errorCodeScd4x = instanceScd4x()->getDataReadyFlag(isDataReady);
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute getDataReadyFlag(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
            return;
        }
        if (!isDataReady)
        {
            return;
        }
        errorCodeScd4x = instanceScd4x()->readMeasurement(co2, temperature, humidity);
        if (errorCodeScd4x)
        {
            Serial.print("Error trying to execute readMeasurement(): ");
            errorToString(errorCodeScd4x, errorMessageScd4x, 256);
            Serial.println(errorMessageScd4x);
        }
        else if (co2 == 0)
        {
            Serial.println("Invalid sample detected, skipping.");
        }
        else
        {
            // Serial.print("Co2:");
            // Serial.print(co2);
            // Serial.print("\t");
            // Serial.print("Temperature:");
            // Serial.print(temperature);
            // Serial.print("\t");
            Serial.print("Humidity:");
            Serial.println(humidity);
            Serial.print("\t");
        }
        // value.valD = scd4x.readMeasurement(temperature);
        value.valD = humidity;
        if (value.valD < 100)
            regEvent(value.valD, "Scd40h");
        else
            SerialPrint("E", "Sensor Scd40h", "Error", _id);
    }

    ~Scd40h(){};
};

void *getAPI_Scd40(String subtype, String param)
{

    if (subtype == F("Scd40co2"))
    {
        // hdc1080.begin(0x40);
        // scd4x->begin(Wire);
        //      scd4x.begin(Wire);
        return new Scd40co2(param);
    }
    else if (subtype == F("Scd40t"))
    {
        // hdc1080.begin(0x40);
        // scd4x->begin(Wire);
        return new Scd40t(param);
    }
    else if (subtype == F("Scd40h"))
    {
        // hdc1080.begin(0x40);
        // scd4x->begin(Wire);
        return new Scd40h(param);
        // } else {
        //     return nullptr;
    }

    return nullptr;
}