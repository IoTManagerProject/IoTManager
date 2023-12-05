#include "Global.h"
#include "classes/IoTItem.h"
#include "NTP.h"

extern IoTGpio IoTgpio;

#ifdef ESP8266
#define ADC_BIT 10
#endif
#ifdef ESP32
#define ADC_BIT 12
// #define analogWrite ledcWrite
#endif

#define ADC_VALUE_MAX pow(2, ADC_BIT)

// Это файл сенсора, в нем осуществляется чтение сенсора.
// для добавления сенсора вам нужно скопировать этот файл и заменить в нем текст AnalogAdc на название вашего сенсора
// Название должно быть уникальным, коротким и отражать суть сенсора.

// ребенок       -       родитель
class MQgas : public IoTItem
{
private:
    //=======================================================================================================
    // Секция переменных.
    // Это секция где Вы можете объявлять переменные и объекты arduino библиотек, что бы
    // впоследствии использовать их в loop и setup
    byte _pin;
    int currentSensor;
    float _ro = 0; // расчетное сопротивления датчика. переменная для текущих расчетов
    float ro_TH = 0;
    float rlBoard = 0;
    float R0CleanAir; // расчетное сопротивления датчика, полученное из настроек элемента
    float R0CleanAirDefault;
    float RlR0CleanAir;
    float RlR0CleanAirDefault;
    float ppmCleanAir;
    float ppmCleanAirDefault;
    float aLimit;
    float bLimit;
    byte sampleTimes = 10;
    byte sampleInterval = 20;
    byte intensity = 5;
    bool autoCalibrationEnable = true;
    unsigned int autoCalibPeriod = 24; // часы
    unsigned long autoCalibTimer = 0;  // мс
    unsigned int warmUpTime = 60;      // сек
    bool _stateCalibrate = false;
    bool _stateCalibrateTH = false;
    String lastCalibration;
    unsigned long lastCalibrationMillis;
    float highRs = 0;
    bool enableTempHumCorrection = false;
    float k1;
    float k2;
    float b1;
    float b2;
    float Hum;
    float Temp;
    double tempHumCorrection;
    String _idTempSensor;
    String _idHumSensor;
    float operatingVoltage = 3.3;
    double ppmResult = 0;
    bool debug = true;

public:
    //=======================================================================================================
    // setup()
    // это аналог setup из arduino. Здесь вы можете выполнять методы инициализации сенсора.
    // Такие как ...begin и подставлять в них параметры полученные из web интерфейса.
    // Все параметры хранятся в перемененной parameters, вы можете прочитать любой параметр используя jsonRead функции:
    // jsonReadStr, jsonReadBool, jsonReadInt
    MQgas(String parameters) : IoTItem(parameters)
    {

#ifdef ESP8266
        _pin = 0;
#endif
#ifdef ESP32
        _pin = jsonReadInt(parameters, "pin-Esp32");
        //  adcAttachPin(_pin);
#endif

        currentSensor = jsonReadInt(parameters, "Series"); // пока не используется
        jsonRead(parameters, F("Rl on board"), rlBoard, false);
        jsonRead(parameters, F("aLimit"), aLimit, false);
        jsonRead(parameters, F("bLimit"), bLimit, false);
        jsonRead(parameters, F("Ro in clean air"), R0CleanAir, false); //
        R0CleanAirDefault = R0CleanAir;
        jsonRead(parameters, F("Rl/Ro in clean air"), RlR0CleanAir, false); // соотношение
        RlR0CleanAirDefault = RlR0CleanAir;
        jsonRead(parameters, F("PPM in clean air"), ppmCleanAir, false); //
        ppmCleanAirDefault = ppmCleanAir;
        warmUpTime = jsonReadInt(parameters, "Warm up time");
        sampleInterval = jsonReadInt(parameters, "Sample interval");
        sampleTimes = jsonReadInt(parameters, "Sample times");
        intensity = jsonReadInt(parameters, "Calibtation intensity");
        autoCalibrationEnable = jsonReadBool(parameters, "autoCalibration");
        autoCalibPeriod = jsonReadInt(parameters, "autoCalib.Period");

        enableTempHumCorrection = jsonReadBool(parameters, "TempHum correction");
        jsonRead(parameters, F("k1"), k1, false);
        jsonRead(parameters, F("k2"), k2, false);
        jsonRead(parameters, F("b1"), b1, false);
        jsonRead(parameters, F("b2"), b2, false);
        jsonRead(parameters, F("temperature"), Temp, false);
        jsonRead(parameters, F("humidity"), Hum, false);
        jsonRead(parameters, "idTempSensor", _idTempSensor);
        jsonRead(parameters, "idHumSensor", _idHumSensor);

        jsonRead(parameters, F("operating voltage"), operatingVoltage, false);

        debug = jsonReadBool(parameters, "Debug");
        if (debug)
        {
            Serial.print("ADC_VALUE_MAX =");
            Serial.println(ADC_VALUE_MAX);
            Serial.print("R0CleanAir =");
            Serial.println(R0CleanAir);
            Serial.print("Rl/R0CleanAir =");
            Serial.println(RlR0CleanAir);
            Serial.print("ppmCleanAir =");
            Serial.println(ppmCleanAir);
        }

        calibrate(); // быстрая калибровка, по холодному
        warmUpTime = millis() / 1000 + warmUpTime;
        _stateCalibrate = false; // чтобы сделать еще одну калибровку через warmUpTime
        lastCalibration = NAN;
    }

    //=======================================================================================================
    // это аналог loop из arduino, но вызываемый каждые int секунд, заданные в настройках. Здесь вы должны выполнить чтение вашего сенсора
    void doByInterval()
    {

        if (!_stateCalibrate && millis() > warmUpTime * 1000) // повторная калибровка после минимального прогрева
        {
            calibrate();
        }

        if (((millis() - autoCalibTimer) > autoCalibPeriod * 3600 * 1000) && autoCalibrationEnable) // автомастическа калибровка
        {
            autoCalibration();
        }

        ppmResult = readSensor();

        if (_stateCalibrate) // без калибровки не выводим ничего
        {
            value.valD = ppmResult;

            regEvent(value.valD, "MQgas");
        }
        else
        {
            value.valD = NAN;
            regEvent(value.valD, "MQgas");
        }
    }

    // получаем Temp и Hum из других элементов
    void onRegEvent(IoTItem *eventItem)
    {
        if (eventItem)
        {
            if (_idTempSensor != "")
            {
                if (_idTempSensor == eventItem->getID())
                {
                    String _idTempSensorString = eventItem->getValue();
                    Temp = _idTempSensorString.toFloat();
                    if (debug)
                    {
                        String output = " got via eventItem: Temp = " + String(Temp);
                        SerialPrint("I", "MQgas", output, _id);
                    }
                }
            }

            if (_idHumSensor != "")
            {
                if (_idHumSensor == eventItem->getID())
                {
                    String _idHumSensorSting = eventItem->getValue();
                    Hum = _idHumSensorSting.toFloat();
                    if (debug)
                    {
                        String output = " got via eventItem: Hum = " + String(Hum);
                        SerialPrint("I", "MQgas", output, _id);
                    }
                }
            }
        }
    }

    // получаем вызовы из сценария
    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (command == "calibrate") // калибровка значениями по умолчанию (из настроек)
        {
            R0CleanAir = R0CleanAirDefault;
            RlR0CleanAir = RlR0CleanAirDefault;
            ppmCleanAir = ppmCleanAirDefault;
            calibrate();
            SerialPrint("I", "MQgas", "calibrate() with default values", _id);
        }
        else if (command == "calibrateR0")
        {
            R0CleanAir = param[0].valD;
            calibrate();
            String output = "calibrateR0(), R0CleanAir = " + String(R0CleanAir);
            SerialPrint("I", "MQgas", output, _id);
        }
        else if (command == "calibrateRlRo")
        {
            R0CleanAir = 0;
            ppmCleanAir = 0;
            RlR0CleanAir = param[0].valD;
            calibrate();
            String output = "calibrateRlRo(), RlR0CleanAir = " + String(RlR0CleanAir);
            SerialPrint("I", "MQgas", output, _id);
        }
        else if (command == "calibratePPM")
        {
            R0CleanAir = 0;
            RlR0CleanAir = 0;
            ppmCleanAir = param[0].valD;
            calibrate();
            String output = "calibratePPM(), ppmCleanAir = " + String(ppmCleanAir);
            SerialPrint("I", "MQgas", output, _id);
        }
        else if (command == "setAutoCalibration")
        {
            if (param[0].isDecimal)
            {
                autoCalibrationEnable = param[0].valD;
            }
            String output = "setAutoCalibration = " + String(autoCalibrationEnable);
            SerialPrint("I", "MQgas", output, _id);
        }
        else if (command == "runAutoCalibration")
        {
            autoCalibration();
        }
        else if (command == "lastCalibration") // оправляем время послежней калибровки в сценарий
        {
            IoTValue valTmp;
            valTmp.isDecimal = false;

            if (lastCalibration == "00.00.00 00:00:00")
            {
                unsigned long miliSeconds = millis() - lastCalibrationMillis;
                unsigned long seconds = miliSeconds / 1000;
                unsigned long minutes = seconds / 60;
                unsigned long hours = minutes / 60;
                unsigned long days = hours / 24;
               // miliSeconds %= 1000;
                seconds %= 60;
                minutes %= 60;
                hours %= 24;
                lastCalibration = String(days) + "d " + String(hours) + ":" + String(minutes) + ":" + String(seconds);
            }

            valTmp.valS = lastCalibration;
            String output = "By request: lastCalibration = " + String(valTmp.valS);
            SerialPrint("I", "MQgas", output, _id);
            return valTmp;
        }
        /*
         else if (command == "enabledAutoCalibration") // оправляем время послежней калибровки в сценарий
        {
            IoTValue valTmp;
            valTmp.isDecimal = true;
            valTmp.valD = autoCalibrationEnable;
            String output = "By request: enabledAutoCalibration = " + String(valTmp.valD);
            SerialPrint("I", "MQgas", output, _id);
            return valTmp;
        }
        */
        else if (command == "TempHumCorrection") // получаем Temp и Hum из сценария
        {
            if (param[0].isDecimal)
                Temp = param[0].valD;
            if (param[1].isDecimal)
                Hum = param[1].valD;
            String output = "TempHumCorrection()  temperature = " + String(Temp) + "  humidity = " + String(Hum);
            SerialPrint("I", "MQgas", output, _id);
        }
        
        return {}; // команда поддерживает возвращаемое значения. Т.е. по итогу выполнения команды или общения с внешней системой, можно вернуть значение в сценарий для дальнейшей обработки
    }

    // получиение соотношения Rl/Ro на чистом воздухе
    float getRlRoInCleanAir()
    {
        float RlR0CleanAirCalc;

        if (ppmCleanAir) // или расчитываем по ppm
        {
            RlR0CleanAirCalc = exp((log(ppmCleanAir) * aLimit) + bLimit);
        }
        else // или берем готовое их настроек
        {
            RlR0CleanAirCalc = RlR0CleanAir;
        }

        return RlR0CleanAirCalc;
    }

    // калибровка датчика
    void calibrate()
    {
        float ro = 0;
        if (R0CleanAir) // при знании сопративления датчика на чистом воздухe
        {               // фиксированая калибровка датчика
            ro = R0CleanAir;
            RlR0CleanAir = rlBoard / R0CleanAir; // просто пересчитываем для вывода в дебаг
        }
        else // расчет ro через RlRoInCleanAir
        {
            float rs = readRs(sampleTimes * intensity); // считывания показаний сопративление датчика на чистом воздухе!!
            ro = rs / getRlRoInCleanAir();
        }

        if (ro && ro == ro) // проверка что не NAN
        {
            _ro = ro; // значение сопративления сенсора на воздухе
            _stateCalibrate = true;
            lastCalibration = getDateTimeDotFormated();
            lastCalibrationMillis = millis();
            String output = "Calibration successful!  Ro in Clean Air = " + String(_ro);
            Serial.println();
            SerialPrint("I", "MQgas", output, _id);
            calibrationTH();
        }
        else
        {
            Serial.println();
            SerialPrint("E", "MQgas", " Calibration failed! Fill one of Setting 'in clean air', check wiring ", _id);
        }
    }

    // выполнение автоматической калибровки
    void autoCalibration()
    {
        _ro = highRs / getRlRoInCleanAir(); // значение сопративления сенсора на воздухе

        if (_ro && _ro == _ro)
        {
            _stateCalibrate = true;
            lastCalibration = getDateTimeDotFormated();
            lastCalibrationMillis = millis();
            String output = "autoCalibration successful!, R0 =  " + String(_ro);
            Serial.println();
            SerialPrint("I", "MQgas", output, _id);
            calibrationTH();
            autoCalibTimer = millis();
            highRs = 0;
        }
        else
        {
            Serial.println();
            SerialPrint("E", "MQgas", " autoCalibration failed! Fill one of Setting 'in clean air', check wiring ", _id);
        }
    }

    // калибровка с учетом температуры и влажности
    void calibrationTH()
    {
        if (enableTempHumCorrection && goodReadTH()) // нормализуем к текущей температуре и влажности
        {
            tempHumCorrection = TempHumCorrection();
            ro_TH = _ro * tempHumCorrection;
            _stateCalibrateTH = true;
            String output = "TH Calibration successful!  Ro_TH in Clean Air = " + String(ro_TH);
            Serial.println();
            SerialPrint("I", "MQgas", output, _id);
        }
        else
        {
            Serial.println();
            SerialPrint("E", "MQgas", "TH Calibration failed! Fill Temp and Hum at Settings or add Temp and Hum sensors!", _id);
        }
    }

    // расчет сопротивление датчика
    float CalculateResistance(int sensorADC)
    {
        float sensorVoltage = sensorADC * (operatingVoltage / ADC_VALUE_MAX);
        float sensorResistance = (operatingVoltage - sensorVoltage) / sensorVoltage * rlBoard;
        return sensorResistance;
    }

    // циклическое считывание сопративления датчика
    float readRs(byte sampleTimes)
    {
        float rs = 0;
        int sensorADC;
        byte error0 = 0;
        byte errorMax = 0;

        for (int i = 0; i < sampleTimes; i++)
        {
            sensorADC = analogRead(_pin);
            if (sensorADC >= ADC_VALUE_MAX - 1)
            {
                errorMax = 1;
                if (debug)
                {
                    String output = "Check wiring, analogRead(_pin) =  " + String(sensorADC);
                    SerialPrint("E", "MQgas", output, _id);
                }
            }
            else if (sensorADC == 0)
            {
                error0 = 1;
                if (debug)
                    SerialPrint("E", "MQgas", "Check sensor, analogRead(_pin) = 0", _id);
            }
            else
            {
                rs += CalculateResistance(sensorADC);
                delay(sampleInterval);
            }
        }

        if (!error0 && !errorMax)
        {
            rs = rs / (sampleTimes);

            if (rs > highRs && millis() > warmUpTime * 1000) // запоминаем максимальное значение сопротивления сенсора для автокалибровки
                highRs = rs;
        }
        else
        {
            rs = NAN;
            if (errorMax)
            {
                String output = "Check wiring, analogRead(_pin) =  " + String(sensorADC);
                SerialPrint("E", "MQgas", output, _id);
            }
            if (error0)
                SerialPrint("E", "MQgas", "Check sensor, analogRead(_pin) = 0", _id);
        }

        if (debug)
        {
            Serial.print("Analog: ");
            Serial.print(analogRead(_pin));
            Serial.print("\tRo: ");
            Serial.print(_ro);
            Serial.print("\treadRs: ");
            Serial.print(rs);
            Serial.print("\thighRs: ");
            Serial.print(highRs);
        }
        return rs;
    }

    // расчет соотношения Rs/Ro
    float readRatio()
    {
        float readRatio = NAN;
        if (_ro)
        {
            float rs = readRs(sampleTimes);
            if (rs)
            {
                readRatio = rs / _ro; // getRo();
            }
        }
        if (debug)
        {
            Serial.print(" \tRs/RoRatio: ");
            Serial.print(readRatio);
        }
        return readRatio;
    }

    // расчет значения концентрации газа
    double readSensor()
    {
        double readSensor = NAN;
        double ratio = readRatio();

        if (ratio != 0 && ratio == ratio) // also not NAN
        {
            readSensor = exp((log(ratio) - bLimit) / aLimit);
            if (debug)
            {
                Serial.print("\tGas: ");
                Serial.println(readSensor, 10);
            }

            if (enableTempHumCorrection && _stateCalibrateTH && goodReadTH()) // поправка на температуру и влажность
            {
                tempHumCorrection = TempHumCorrection();

                ratio = ratio * tempHumCorrection;
                readSensor = exp((log(ratio * _ro / ro_TH) - bLimit) / aLimit);
                if (debug)
                {
                    Serial.print("\tTHCor: ");
                    Serial.print(tempHumCorrection, 10);
                    Serial.print("\tGasTHCorrected: ");
                    Serial.println(readSensor, 10);
                }
            }
        }

        return readSensor;
    }

    // расчет поправки на температуру и влажность
    double TempHumCorrection()
    {
        double correction = 1;

        double k_hum = k1 * Hum / 100 + k2;
        double b_hum = b1 * Hum / 100 + b2;
        correction = k_hum * Temp + b_hum;

        if (debug)
        {
            Serial.print("\tk_hum: ");
            Serial.print(k_hum, 10);
            Serial.print("\tb_hum: ");
            Serial.print(b_hum, 10);
            Serial.print("\tcor: ");
            Serial.print(correction, 10);
        }

        return correction;
    }

    // проверка значний температуры и влажности
    bool goodReadTH()
    {
        if (Hum > 0 && Hum < 95 && Temp > 0 && Temp < 70)
        {
            return true;
        }
        else
        {
            if (debug)
            {
                Serial.println();
                SerialPrint("E", "MQgas", "Wrong data from Temperature and Humidity sensor or from Settings", _id);
            }
            return false;
        }
    }

    ~MQgas(){};
};

void *getAPI_MQgas(String subtype, String param)
{
    if (subtype == F("MQgas"))
    {
        return new MQgas(param);
    }
    else
    {
        return nullptr;
    }
}
