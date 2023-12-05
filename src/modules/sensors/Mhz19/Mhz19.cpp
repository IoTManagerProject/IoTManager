/******************************************************************
  Support for MHZ19

  adapted for version 4 @cmche
 ******************************************************************/
#include "Global.h"
#include "classes/IoTItem.h"



extern IoTGpio IoTgpio;

// SoftwareSerial* swSerialCO2 = nullptr;
int rxPinCO2; // зеленый провод сенсора к прописаному по умолчанию D7 (GPIO13)
int txPinCO2; // синий провод сенсора к прописаному по умолчанию D6 (GPIO12)
#ifdef ESP8266
//встроена в ядро для 8266, для 32 по этому же имени обращаемся к другой библиотеке plerup/EspSoftwareSerial
#include <SoftwareSerial.h>
SoftwareSerial *swSerialCO2 = nullptr;
#endif
#ifdef ESP32
#include <HardwareSerial.h>
HardwareSerial swSerialCO2(1);
#endif

int MHZ19_request(int request);

void MHZ19uart_init();

bool MHZ19uartInit_flag = true;
bool MHZ19uartUpdateInputs_flag = true;
unsigned int preheating = 2 * 60; // покажет реальные данные после прогрева, через 2 мин.

int temperature = 0;
bool temperatureUpdated = false;
int prevRange = 5000;
int range = 5000; // по умолнчанию стоит шкала 5000 (не 2000 как в мануале)
bool rangeChaged = false;
int prevABC = 1;
int ABC = 1;
bool ABCchanged = false;

//Это файл сенсора, в нем осуществляется чтение сенсора.

class Mhz19uart : public IoTItem
{
private:
    //=======================================================================================================
    // Секция переменных.

public:
    //=======================================================================================================

    Mhz19uart(String parameters) : IoTItem(parameters)
    {
        rxPinCO2 = jsonReadInt(parameters, "rxPin");
        txPinCO2 = jsonReadInt(parameters, "txPin");
        preheating = jsonReadInt(parameters, "warmUp");
        range = jsonReadInt(parameters, "range");
        ABC = jsonReadInt(parameters, "ABC");
        MHZ19uartInit_flag = true;
        MHZ19uartUpdateInputs_flag = true;
#ifdef ESP8266
        if (!swSerialCO2)
            swSerialCO2 = new SoftwareSerial(rxPinCO2, txPinCO2);
#endif
#ifdef ESP32
// HardwareSerial swSerialCO2(1);
// HardwareSerial swSerialCO2(2);
// swSerialCO2.begin(9600);
#endif
    }

    //=======================================================================================================
    // doByInterval()

    void doByInterval()
    {
        MHZ19uart_init();

        if (millis() > preheating * 1000)
        {
            // Serial.println("Start checkUARTCO2");
            int reply = MHZ19_request(1);
            if (reply)
            {
                value.valD = reply;
                regEvent(value.valD, "Mhz19uart");
            }
        }
    }
    //=======================================================================================================

    ~Mhz19uart(){};
};

//после замены названия сенсора, на функцию можно не обращать внимания
//если сенсор предполагает использование общего объекта библиотеки для нескольких экземпляров сенсора, то в данной функции необходимо предусмотреть
//создание и контроль соответствующих глобальных переменных

//замер по ШИМ создает задержку. вызываем его нечасто, по умолчанию раз в 5 минут
class Mhz19pwm : public IoTItem
{
private:
    //=======================================================================================================
    // Секция переменных.
    int pwmPin = 15; // желтый провод сенсора к прописаному по умолчанию D8 (GPIO15)
    int maxRetriesNotAvailable = 10;

public:
    //=======================================================================================================

    Mhz19pwm(String parameters) : IoTItem(parameters)
    {
        pwmPin = jsonReadInt(parameters, "pin");
        preheating = jsonReadInt(parameters, "warmUp");
        maxRetriesNotAvailable = jsonReadInt(parameters, "maxRetriesNotAvailable");
    }
    //=======================================================================================================

    void doByInterval()
    {
        MHZ19pwm_init();

        if (millis() > preheating * 1000) //
        {
            // Serial.println("Start checkPWM_CO2");
            int reply = MHZ19pwm_request();
            if (reply)
            {
                SerialPrint("E", "Sensor Mhz19pwm", "MHZ19pwm_init reply = " + String(reply));
                value.valD = reply;
                regEvent(value.valD, "Mhz19pwm");
            }
        }
    }
    //=======================================================================================================

    void MHZ19pwm_init()
    {
        static bool MHZ19pwm_flag = true;
        if (MHZ19pwm_flag)
        {
            pinMode(pwmPin, INPUT);
            MHZ19pwm_flag = false;
        }
    }

    int MHZ19pwm_request()
    {
        int reply;
        // Serial.println("Запрос замера по PWM запущен");
        unsigned long th = 0, tl, ppm = 0, ppm2 = 0, ppm3 = 0;
        // int pwmPinReading1 = digitalRead(pwmPin);
        // SerialPrint("E", "Sensor Mhz19uart", "pwmPinReading1 = " + String(pwmPinReading1));
        /*
        do
        {
            th = pulseIn(pwmPin, HIGH, 1004000) / 1000;
            tl = 1004 - th;
            ppm2 = 2000 * (th - 2) / (th + tl - 4); // расчёт для диапазона от 0 до 2000ppm
            ppm3 = 5000 * (th - 2) / (th + tl - 4); // расчёт для диапазона от 0 до 5000ppm
        } while (th == 0);
        */
        for (int i = 0; i < maxRetriesNotAvailable; i++)
        {
            th = pulseIn(pwmPin, HIGH, 1004000) / 1000;

            if (th != 0)
            {
                tl = 1004 - th;
                ppm2 = 2000 * (th - 2) / (th + tl - 4); // расчёт для диапазона от 0 до 2000ppm
                ppm3 = 5000 * (th - 2) / (th + tl - 4); // расчёт для диапазона от 0 до 5000ppm
                if (i > 1)
                SerialPrint("E", "Sensor Mhz19pwm", "Got reading from PWM pin after " + String(i) + " tries");
                break;
            }
            // Serial.println(i);
        }
        if (!th)
        {
            SerialPrint("E", "Sensor Mhz19pwm", "No reading from PWM pin. Check wiring.");
            reply = 0;
        }
        else
        {

            // Serial.print(th);
            // Serial.println(" <- Milliseconds PWM is HIGH");
            if (range == 2000)
            {
                reply = ppm2;
                // Serial.print(ppm2);
                // Serial.println(" <- ppm2 (PWM) with 2000ppm as limit");
            }
            else
            {
                reply = ppm3;
                // Serial.print(ppm3);
                // Serial.println(" <- ppm3 (PWM) with 5000ppm as limit");
            }
            // Serial.println("Completed checkPwmCO2");
        }
        // int pwmPinReading2 = digitalRead(pwmPin);
        // SerialPrint("E", "Sensor Mhz19uart", "pwmPinReading2 = " + String(pwmPinReading2));
        return reply;
    }

    ~Mhz19pwm(){};
};

//====================TEMP===================================================================================

class Mhz19temp : public IoTItem
{
private:
    //=======================================================================================================
    // Секция переменных.
public:
    //====================TEMP===================================================================================

    Mhz19temp(String parameters) : IoTItem(parameters)
    {
        rxPinCO2 = jsonReadInt(parameters, "rxPin");
        txPinCO2 = jsonReadInt(parameters, "txPin");

#ifdef ESP8266
        if (!swSerialCO2)
            swSerialCO2 = new SoftwareSerial(rxPinCO2, txPinCO2);
#endif
#ifdef ESP32
// HardwareSerial swSerialCO2(1);
#endif
    }
    //=======================================================================================================

    void doByInterval()
    {
        int reply;

        // Serial.println("Start Mhz19temp doByInterval");
        if (temperatureUpdated)
        {
            reply = temperature;
            temperatureUpdated = false;
        }
        else
        {
            MHZ19uart_init();
            // Serial.println("Start temperature request");
            reply = MHZ19_request(13);
        }

        if (reply)
        {
            value.valD = reply;
            regEvent(value.valD, "Mhz19temp");
        }
    }
    //=======================================================================================================

    //=======================================================================================================

    ~Mhz19temp(){};
};

void *getAPI_Mhz19(String subtype, String param)
{
    if (subtype == F("Mhz19uart"))
    {
        return new Mhz19uart(param);
    }
    else if (subtype == F("Mhz19pwm"))
    {
        return new Mhz19pwm(param);
    }
    else if (subtype == F("Mhz19temp"))
    {
        return new Mhz19temp(param);
    }
    else
    {
        return nullptr;
    }
}

void MHZ19uart_init()
{
    if (MHZ19uartInit_flag)
    {

#ifdef ESP8266
        swSerialCO2->begin(9600);
#endif
#ifdef ESP32
        // if(swSerialCO2.available()) {    Serial.println("ok");  }
        swSerialCO2.begin(9600, SERIAL_8N1, rxPinCO2, txPinCO2);
#endif
        delay(50);
        int reply;
        reply = MHZ19_request(2); // show range, for  test of uart only
        // Serial.print("show range reply = ");
        // Serial.println(reply);

        if (reply)
        {
            prevRange = reply;
            MHZ19uartInit_flag = false;
        }
    }

    if (!MHZ19uartInit_flag && ABC != prevABC)
    {
        int reply;

        if (ABC != prevABC)
        {
            if (ABC)
            {
                reply = MHZ19_request(7); // ABC on
            }
            else
            {
                reply = MHZ19_request(8); // ABC off
            }
            // Serial.print("ABC change reply = ");
            // Serial.println(reply);
        }
        if (reply)
        {
            prevABC = ABC;
        }
    }

    if (range != prevRange && !MHZ19uartInit_flag)
    {
        int reply;
        if (range == 2000)
        {
            reply = MHZ19_request(9); // Установка шкалы 0-2000 - 255 155 0 0 7 208 0 3 139
        }
        else
        {
            reply = MHZ19_request(10); // Установка шкалы 0-5000 - 255 155 0 0 19 136 0 3 199
        }
        // Serial.print("Scale change reply = ");
        // Serial.println(reply);
        if (reply)
        {
            // reply = MHZ19_request(2); // show range
            //  Serial.print("show range reply = ");
            //  Serial.println(reply);
            prevRange = range;
            // MHZ19_range_flag = false;
        }
    }
}

int MHZ19_request(int request)
{
    int reply;
    // Serial.print("prevRange = ");
    // Serial.println(prevRange);

    byte uartReqSamplePpm[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79}; // PPM
    // 255 1 134 0 0 0 0 0 121
    // Response 255 134 1 148 67 0 0 0 162

    byte uartReqSampleABCon[9] = {0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6};     // ABC logic on
                                                                                             // 255 1 121 160 0 0 0 0 230
                                                                                             // Response  255 121 1 0 0 0 0 0 134
    byte uartReqSampleABCoff[9] = {0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86};    // ABC logic off
    byte uartReqSampleABCstatus[9] = {0xFF, 0x01, 0x7D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82}; // ABC logic status
    byte uartReqSample1000Range[9] = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x03, 0xE8, 0x7B};
    byte uartReqSample2000Range[9] = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x07, 0xD0, 0x8F};    // задаёт диапазон 0 - 2000ppm
    byte uartReqSample3000Range[9] = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x0B, 0xB8, 0xA3};    // задаёт диапазон 0 - 2000ppm
    byte uartReqSample5000Range[9] = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x13, 0x88, 0xCB};    // задаёт диапазон 0 - 5000ppm
                                                                                                // 255 1 153 0 0 0 19 136 203
                                                                                                // Response  255 153 1 0 0 0 0 0 102
    byte uartReqSampleRequestRange[9] = {0xFF, 0x01, 0x9B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64}; // запрос диапазона
                                                                                                // request    // 255 1 155 0 0 0 0 0 100
                                                                                                // reply       // 255 1 155 0 0 0 0 0 100
    byte uartReqSampleZeroPnt[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};      // !!ZERO POINT CALIBRATION
    byte uartReqSampleReset[9] = {0xFF, 0x01, 0x8D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72};        // reset
    byte uartReqSampleReset1[9] = {0xFF, 0x01, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87};       // reset - did not work out

    byte request_cmd[9];
    // byte c;
    switch (request)
    {
        // случаи 3-7 для перезапуска сенсора
    case 1:
    {
        // Serial.println("Запрос No.1 - отправлен. Запрос замера по UART");
        for (int i = 0; i < 9; i++)
        {
            request_cmd[i] = uartReqSamplePpm[i];
        }
    }
    break;

    case 2:
    {
        for (int i = 0; i < 9; i++)
        {
            request_cmd[i] = uartReqSampleRequestRange[i];
        }
        // Serial.println("Запрос No.2 - отправлен. Запрос шкалы");
    }
    break;

    case 3:
    {
        // код для запуска сенсора с работой по UART (запуск таймера)
        // Serial.println("Запрос No.3 - отправлен. Сенсор по UART запущен");
    }
    break;
    case 4:
    {
        // код для остановке сенсора с работой по UART (остановка таймера)
        // Serial.println("Запрос No.4 - отправлен. Сенсор по UART остановлен");
    }
    break;
    case 5:
    {
        // код для запуска сенсора с работой по PWM (запуск таймера)
        // Serial.println("Запрос No.5 - отправлен. Сенсор по PWM запущен");
    }
    break;
    case 6:
    {
        // код для остановки сенсора с работой по PWM (остановка таймера)
        // Serial.println("Запрос No.6 - отправлен. Сенсор по PWM остановлен");
    }
    break;
    case 7:
    {
        for (int i = 0; i < 9; i++)
        {
            request_cmd[i] = uartReqSampleABCon[i];
        }
        // Serial.println("Запрос No.7 - отправлен. Включаем функцию атокалибровки");
    }
    break;
    case 8:
    {
        for (int i = 0; i < 9; i++)
        {
            request_cmd[i] = uartReqSampleABCoff[i];
        }
        // Serial.println("Запрос No.8 - отправлен. Выключаем функцию атокалибровки");
    }
    break;
    case 9:
    {
        for (int i = 0; i < 9; i++)
        {
            request_cmd[i] = uartReqSample2000Range[i];
        }
        // Serial.println("Запрос No.9 - отправлен. Установливаем шкалу 0-2000");
    }
    break;
    case 10:
    {
        for (int i = 0; i < 9; i++)
        {
            request_cmd[i] = uartReqSample5000Range[i];
        }
        // Serial.println("Запрос No.10 - отправлен. Установливаем шкалу 0-5000");
    }
    break;
    case 11:
    {
        for (int i = 0; i < 9; i++)
        {
            request_cmd[i] = uartReqSampleZeroPnt[i];
        }
        // Serial.println("Запрос No.11 - отправлен. Калибровка. Установливаем нулевой уровень");
    }
    break;
    case 12:
    {
        for (int i = 0; i < 9; i++)
        {
            request_cmd[i] = uartReqSampleReset1[i];
        }
        // Serial.println("Запрос No.11 - отправлен. Запрос на Сброс");
    }
    break;
    case 13:
    {
        for (int i = 0; i < 9; i++)
        {
            request_cmd[i] = uartReqSamplePpm[i];
        }
        // Serial.println("Запрос No.13 - отправлен. Запрос по Температуре");
    }
    break;
    default:
        // byte c = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
        // if nothing else matches, do the default
        // default is optional
        break;
    }

#ifdef ESP8266
    swSerialCO2->write(request_cmd, 9);
#endif
#ifdef ESP32
    swSerialCO2.write(request_cmd, 9);
#endif
    /*
        Serial.print("Request : ");
        for (int i = 0; i < 9; i++)
        {
            Serial.print(" ");
            Serial.print(request_cmd[i]);
        }
        Serial.println(" ");
    */
    delay(5);
    unsigned char response[9];

#ifdef ESP8266
    swSerialCO2->readBytes(response, 9);
#endif

#ifdef ESP32
    swSerialCO2.readBytes(response, 9);
#endif
    /*
        Serial.print("Response :");
        for (int i = 0; i < 9; i++)
        {
            Serial.print(" ");
            Serial.print(response[i]);
        }
        Serial.println(" ");
    */
    byte crc = 0;
    for (int i = 1; i < 8; i++)
        crc += response[i];
    crc = 255 - crc;
    crc += 1;

    if (!(response[0] == 0xFF && response[8] == crc))
    {
        String msg = "Range CRC error: " + String(crc) + " / " + String(response[8]) + " Check wiring";
        SerialPrint("E", "Sensor Mhz19uart", msg);
        reply = 0;
        MHZ19uartInit_flag = true; //
    }
    else
    {
        // Serial.println("No CRC errors");
        switch (request)
        {
        case 1:
        {
            unsigned int responseHigh = (unsigned int)response[2];
            unsigned int responseLow = (unsigned int)response[3];
            unsigned int ppm = (256 * responseHigh) + responseLow;

            //   Serial.print("CO2 UART = ");
            //   Serial.println(ppm);

            temperature = response[4] - 44; // - 40;
                                            //   Serial.print("Temperature = ");
                                            //   Serial.println(temperature);
            temperatureUpdated = true;

            reply = ppm;
        }
        break;

        case 2:
        {

            unsigned int responseHigh = (unsigned int)response[4];
            unsigned int responseLow = (unsigned int)response[5];
            unsigned int scale = (256 * responseHigh) + responseLow;
            reply = scale;
            SerialPrint("i", "Sensor Mhz19uart", "Запрос No.2 - сработал. Шкала получена - " + String(scale));
        }
        break;
        case 3:
        {
            // Serial.println("Case 3 - OK");
            reply = 1;
        }
        break;
        case 4:
        {
            // Serial.println("Case 4 - OK");
            reply = 1;
        }
        break;
        case 5:
        {
            // Serial.println("Case 5 - OK");
            reply = 1;
        }
        break;
        case 6:
        {
            // Serial.println("Запрос No.6 - сработал.");
            reply = 1;
        }
        break;
        case 7:
        {
            SerialPrint("i", "Sensor Mhz19uart", "Запрос No.7 - сработал. ABC включен");
            reply = 1;
        }
        break;
        case 8:
        {
            SerialPrint("i", "Sensor Mhz19uart", "Запрос No.8 - сработал. ABC выключен");
            reply = 1;
        }
        break;
        case 9:
        {
            SerialPrint("i", "Sensor Mhz19uart", "Запрос No.9 - сработал. Установлена шкала 0-2000");
            reply = 1;
            prevRange = 2000;
        }
        break;
        case 10:
        {
            SerialPrint("i", "Sensor Mhz19uart", "Запрос No.10 - сработал. Установлена шкала 0-5000");
            reply = 1;
            prevRange = 5000;
        }
        break;
        case 11:
        {
            reply = 1;
            SerialPrint("i", "Sensor Mhz19uart", "Запрос No.11 - сработал. Калибровка. Установлен нулевой уровень");
        }
        break;

        case 12:
        {
            reply = 1;
            SerialPrint("i", "Sensor Mhz19uart", "Запрос No.12 - сработал. Сброс произошел");
        }
        break;

        case 13:
        {
            temperature = response[4] - 44; // - 40;
            reply = temperature;
            // SerialPrint("i", "Sensor Mhz19uart", "Запрос No.13 - сработал. Температура получена - " + String(temperature));
            //  Serial.print("Запрос No.13 - сработал. Температура получена - ");
            //  Serial.println(temperature);
        }
        break;

        default:
            // byte c = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
            // if nothing else matches, do the default
            // default is optional
            break;
        }
    }
    // Serial.print("reply = ");
    // Serial.println(reply);
    return reply;
}
