/******************************************************************
  Support for MHZ19

  adapted for version 4 @cmche
 ******************************************************************/
#include "Global.h"
#include "classes/IoTItem.h"

//встроена в ядро для 8266, для 32 по этому же имени обращаемся к другой библиотеке plerup/EspSoftwareSerial
#include <SoftwareSerial.h>

extern IoTGpio IoTgpio;

int rxPinCO2 = 14;                               // D5                          // зеленый провод сенсора к D2 / 4
int txPinCO2 = 16;                               // D0                          // синий провод сенсора к D1/5
SoftwareSerial swSerialCO2(rxPinCO2, txPinCO2);  // RX, TX// ESP8266  D7 / D6 Blue/Green

int MHZ19_request(int request);

void MHZ19uart_init();

bool MHZ19uart_flag = true;
// int MHZ19C_PREHEATING_TIME = 2 * 60 * 1000;// покажет реальные данные после прогрева, через 2 мин.
int MHZ19C_PREHEATING_TIME = 2 * 30 * 1000;  // покажет реальные данные после прогрева, через 2 мин.

// int prevTemperature = 0;
int temperature = 0;
bool temperatureUpdated = false;
int prevRange = 5000;
int range = 5000;  // по умолнчанию стоит шкала 5000 (не 2000 как в мануале)
bool rangeChaged = false;
int prevABC = 1;
int ABC = 1;
bool ABCchanged = false;

//Это файл сенсора, в нем осуществляется чтение сенсора.

class Mhz19uart : public IoTItem {
   private:
    //=======================================================================================================
    // Секция переменных.

   public:
    //=======================================================================================================

    Mhz19uart(String parameters) : IoTItem(parameters) {
        rxPinCO2 = jsonReadInt(parameters, "rxPin");
        txPinCO2 = jsonReadInt(parameters, "txPin");
        range = jsonReadInt(parameters, "range");
        ABC = jsonReadInt(parameters, "ABC");
    }
    //=======================================================================================================
    // doByInterval()

    void doByInterval() {
        MHZ19uart_init();
        if (millis() > MHZ19C_PREHEATING_TIME) {
            Serial.println("Start checkUARTCO2");
            value.valD = MHZ19_request(1);
        }
        regEvent(value.valD, "Mhz19uart");  //обязательный вызов хотяб один
    }
    //=======================================================================================================

    ~Mhz19uart(){};
};

//после замены названия сенсора, на функцию можно не обращать внимания
//если сенсор предполагает использование общего объекта библиотеки для нескольких экземпляров сенсора, то в данной функции необходимо предусмотреть
//создание и контроль соответствующих глобальных переменных

class Mhz19pwm : public IoTItem {
   private:
    //=======================================================================================================
    // Секция переменных.
    int pwmPin;  ///// желтый провод сенсора к D8    (14-D5 ok)

   public:
    //=======================================================================================================

    Mhz19pwm(String parameters) : IoTItem(parameters) {
        pwmPin = jsonReadInt(parameters, "pin");
    }
    //=======================================================================================================

    void doByInterval() {
        MHZ19pwm_init();
        if (millis() > MHZ19C_PREHEATING_TIME)  //
        {
            Serial.println("Start checkPWM_CO2");
            value.valD = MHZ19pwm_request();
        }
        regEvent(value.valD, "Mhz19pwm");  //обязательный вызов хотяб один
    }
    //=======================================================================================================

    void MHZ19pwm_init() {
        static bool MHZ19pwm_flag = true;
        if (MHZ19pwm_flag) {
            pinMode(pwmPin, INPUT);
            MHZ19pwm_flag = false;
        }
    }

    int MHZ19pwm_request() {
        int reply;
        Serial.println("Запрос замера по PWM запущен");
        unsigned long th, tl, ppm = 0, ppm2 = 0, ppm3 = 0;
        do {
            th = pulseIn(pwmPin, HIGH, 1004000) / 1000;
            tl = 1004 - th;
            ppm2 = 2000 * (th - 2) / (th + tl - 4);  // расчёт для диапазона от 0 до 2000ppm
            ppm3 = 5000 * (th - 2) / (th + tl - 4);  // расчёт для диапазона от 0 до 5000ppm
        } while (th == 0);

        // Serial.print(th);
        // Serial.println(" <- Milliseconds PWM is HIGH");

        if (range == 2000) {
            reply = ppm2;
            Serial.print(ppm2);
            Serial.println(" <- ppm2 (PWM) with 2000ppm as limit");
        } else {
            reply = ppm3;
            Serial.print(ppm3);
            Serial.println(" <- ppm3 (PWM) with 5000ppm as limit");
        }
        Serial.println("Completed checkPwmCO2");
        return reply;
    }

    ~Mhz19pwm(){};
};

//====================TEMP===================================================================================

class Mhz19temp : public IoTItem {
   private:
    //=======================================================================================================
    // Секция переменных.
   public:
    //====================TEMP===================================================================================

    Mhz19temp(String parameters) : IoTItem(parameters) {
        rxPinCO2 = jsonReadInt(parameters, "rxPin");
        txPinCO2 = jsonReadInt(parameters, "txPin");
        range = jsonReadInt(parameters, "range");
        ABC = jsonReadInt(parameters, "ABC");
    }
    //=======================================================================================================

    void doByInterval() {
        // Serial.println("Start Mhz19temp doByInterval");
        if (temperatureUpdated) {
            value.valD = temperature;
            temperatureUpdated = false;
        } else {
            MHZ19uart_init();
            Serial.println("Start temperature request");
            if (MHZ19_request(13)) {
                value.valD = temperature;
            };  // change
        }
        regEvent(value.valD, "Mhz19temp");  //обязательный вызов хотяб один
    }
    //=======================================================================================================

    //=======================================================================================================

    ~Mhz19temp(){};
};

//=======================Range================
class Mhz19range : public IoTItem {
   private:
   public:
    Mhz19range(String parameters) : IoTItem(parameters) {
        rxPinCO2 = jsonReadInt(parameters, "rxPin");
        txPinCO2 = jsonReadInt(parameters, "txPin");
        range = jsonReadInt(parameters, "range");
        ABC = jsonReadInt(parameters, "ABC");
    }

    void doByInterval() {
        if (range != prevRange) {
            MHZ19uart_init();
            Serial.println("Start change range");

            if (range == 2000) {
                if (MHZ19_request(9)) {
                    prevRange = 2000;
                    value.valD = 2000;
                }  // change range to 2000
            } else {
                if (MHZ19_request(10)) {
                    prevRange = 5000;
                    value.valD = 5000;
                }  // change range to 5000
            }
        } else {
            value.valD = prevRange;
        }
        regEvent(value.valD, "Mhz19range");  //обязательный вызов хотяб один
    }

    ~Mhz19range(){};
};

//===================ABC=================

class Mhz19ABC : public IoTItem {
   private:
   public:
    Mhz19ABC(String parameters) : IoTItem(parameters) {
        rxPinCO2 = jsonReadInt(parameters, "rxPin");
        txPinCO2 = jsonReadInt(parameters, "txPin");
        range = jsonReadInt(parameters, "range");
        ABC = jsonReadInt(parameters, "ABC");
    }

    void doByInterval() {
        if (ABC != prevABC) {
            if (ABC == 1) {
                if (MHZ19_request(7)) {
                    prevABC = 1;
                    value.valD = 1;
                }  // change ABC to 1
            } else {
                if (MHZ19_request(8)) {
                    prevABC = 0;
                    value.valD = 0;
                }  // change ABC to 0
            }
        } else {
            value.valD = prevABC;
        }
        regEvent(value.valD, "Mhz19ABC");  //обязательный вызов хотяб один
    }

    ~Mhz19ABC(){};
};

///============== end of classes==========================

void *getAPI_Mhz19(String subtype, String param) {
    if (subtype == F("Mhz19uart")) {
        return new Mhz19uart(param);
    } else if (subtype == F("Mhz19pwm")) {
        return new Mhz19pwm(param);
    } else if (subtype == F("Mhz19temp")) {
        return new Mhz19temp(param);
    } else if (subtype == F("Mhz19range")) {
        return new Mhz19range(param);
    } else if (subtype == F("Mhz19ABC")) {
        return new Mhz19ABC(param);
    } else {
        return nullptr;
    }
}

void MHZ19uart_init() {
    if (MHZ19uart_flag) {
        int reply;
        swSerialCO2.begin(9600);

        delay(50);

        reply = MHZ19_request(2);  // show range, for  test of uart only
        Serial.print("show range reply = ");
        Serial.println(reply);

        if (reply) {
            MHZ19uart_flag = false;
        }
    }

    if (!MHZ19uart_flag) {
        static int prevABC;
        int reply;

        if (ABC != prevABC) {
            if (ABC) {
                reply = MHZ19_request(7);  // ABC on
            } else {
                reply = MHZ19_request(8);  // ABC off
            }
            Serial.print("ABC change reply = ");
            Serial.println(reply);
        }
        if (reply) {
            prevABC = ABC;
        }
    }

    static bool MHZ19_range_flag = true;
    if (MHZ19_range_flag && !MHZ19uart_flag && (millis() > 30 * 1000)) {
        int reply;
        if (range == 2000) {
            reply = MHZ19_request(9);  // Установка шкалы 0-2000
                                       //  255 155 0 0 7 208 0 3 139
        } else {
            reply = MHZ19_request(10);  // Установка шкалы 0-5000
                                        // 255 155 0 0 19 136 0 3 199
        }
        Serial.print("Scale change reply = ");
        Serial.println(reply);
        if (reply) {
            reply = MHZ19_request(2);  // show range
            Serial.print("show range reply = ");
            Serial.println(reply);

            MHZ19_range_flag = false;
        }
    }
}

int MHZ19_request(int request) {
    int reply;
    Serial.print("prevRange = ");
    Serial.println(prevRange);

    byte uartReqSamplePpm[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};  // PPM
    // 255 1 134 0 0 0 0 0 121
    // Response 255 134 1 148 67 0 0 0 162

    byte uartReqSampleABCon[9] = {0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6};      // ABC logic on
                                                                                              // 255 1 121 160 0 0 0 0 230
                                                                                              // Response  255 121 1 0 0 0 0 0 134
    byte uartReqSampleABCoff[9] = {0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86};     // ABC logic off
    byte uartReqSampleABCstatus[9] = {0xFF, 0x01, 0x7D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82};  // ABC logic status

    byte uartReqSample1000Range[9] = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x03, 0xE8, 0x7B};
    byte uartReqSample2000Range[9] = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x07, 0xD0, 0x8F};  // задаёт диапазон 0 - 2000ppm
    byte uartReqSample3000Range[9] = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x0B, 0xB8, 0xA3};  // задаёт диапазон 0 - 2000ppm

    byte uartReqSample5000Range[9] = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x13, 0x88, 0xCB};     // задаёт диапазон 0 - 5000ppm
                                                                                                 // 255 1 153 0 0 0 19 136 203
                                                                                                 // Response  255 153 1 0 0 0 0 0 102
    byte uartReqSampleRequestRange[9] = {0xFF, 0x01, 0x9B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64};  // запрос диапазона
                                                                                                 // request    // 255 1 155 0 0 0 0 0 100
                                                                                                 // reply       // 255 1 155 0 0 0 0 0 100
    byte uartReqSampleZeroPnt[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};       // !!ZERO POINT CALIBRATION
    byte uartReqSampleReset[9] = {0xFF, 0x01, 0x8D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72};         // reset

    byte uartReqSampleReset1[9] = {0xFF, 0x01, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87};  // reset - did not work out

    byte request_cmd[9];
    // byte c;
    switch (request) {
            // случаи 3-7 для перезапуска сенсора
        case 1: {
            Serial.println("Запрос No.1 - отправлен. Запрос замера по UART");
            for (int i = 0; i < 9; i++) {
                request_cmd[i] = uartReqSamplePpm[i];
            }
        } break;

        case 2: {
            for (int i = 0; i < 9; i++) {
                request_cmd[i] = uartReqSampleRequestRange[i];
            }
            Serial.println("Запрос No.2 - отправлен. Запрос шкалы");
        } break;

        case 3: {
            // код для запуска сенсора с работой по UART (запуск таймера)
            Serial.println("Запрос No.3 - отправлен. Сенсор по UART запущен");
        } break;
        case 4: {
            // код для остановке сенсора с работой по UART (остановка таймера)
            Serial.println("Запрос No.4 - отправлен. Сенсор по UART остановлен");
        } break;
        case 5: {
            // код для запуска сенсора с работой по PWM (запуск таймера)
            Serial.println("Запрос No.5 - отправлен. Сенсор по PWM запущен");
        } break;
        case 6: {
            // код для остановки сенсора с работой по PWM (остановка таймера)
            Serial.println("Запрос No.6 - отправлен. Сенсор по PWM остановлен");
        } break;
        case 7: {
            for (int i = 0; i < 9; i++) {
                request_cmd[i] = uartReqSampleABCon[i];
            }
            Serial.println("Запрос No.7 - отправлен. Включаем функцию атокалибровки");
        } break;
        case 8: {
            for (int i = 0; i < 9; i++) {
                request_cmd[i] = uartReqSampleABCoff[i];
            }
            Serial.println("Запрос No.8 - отправлен. Выключаем функцию атокалибровки");
        } break;
        case 9: {
            for (int i = 0; i < 9; i++) {
                request_cmd[i] = uartReqSample2000Range[i];
            }
            Serial.println("Запрос No.9 - отправлен. Установливаем шкалу 0-2000");
        } break;
        case 10: {
            for (int i = 0; i < 9; i++) {
                request_cmd[i] = uartReqSample5000Range[i];
            }
            Serial.println("Запрос No.10 - отправлен. Установливаем шкалу 0-5000");
        } break;
        case 11: {
            for (int i = 0; i < 9; i++) {
                request_cmd[i] = uartReqSampleZeroPnt[i];
            }
            Serial.println("Запрос No.11 - отправлен. Калибровка. Установливаем нулевой уровень");
        } break;
        case 12: {
            for (int i = 0; i < 9; i++) {
                request_cmd[i] = uartReqSampleReset1[i];
            }
            Serial.println("Запрос No.11 - отправлен. Запрос на Сброс");
        } break;
        case 13: {
            for (int i = 0; i < 9; i++) {
                request_cmd[i] = uartReqSamplePpm[i];
            }
            Serial.println("Запрос No.13 - отправлен. Запрос по Температуре");
        } break;
        default:
            // byte c = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
            // if nothing else matches, do the default
            // default is optional
            break;
    }

    swSerialCO2.write(request_cmd, 9);

    Serial.print("Request : ");
    for (int i = 0; i < 9; i++) {
        Serial.print(" ");
        Serial.print(request_cmd[i]);
    }
    Serial.println(" ");

    delay(50);

    unsigned char response[9];

    Serial.print("Response :");
    swSerialCO2.readBytes(response, 9);

    for (int i = 0; i < 9; i++) {
        Serial.print(" ");
        Serial.print(response[i]);
    }
    Serial.println(" ");

    byte crc = 0;
    for (int i = 1; i < 8; i++) crc += response[i];
    crc = 255 - crc;
    crc += 1;

    if (!(response[0] == 0xFF && response[8] == crc)) {
        Serial.println("Range CRC error: " + String(crc) + " / " + String(response[8]));
        reply = 0;
    } else {
        // Serial.println("No CRC errors");
        switch (request) {
            case 1: {
                unsigned int responseHigh = (unsigned int)response[2];
                unsigned int responseLow = (unsigned int)response[3];
                unsigned int ppm = (256 * responseHigh) + responseLow;

                Serial.print("CO2 UART = ");
                Serial.println(ppm);

                temperature = response[4] - 44;  // - 40;
                Serial.print("Temperature = ");
                Serial.println(temperature);
                temperatureUpdated = true;

                reply = ppm;
            } break;

            case 2: {
                reply = 1;
                Serial.println("Case 2 - OK. На запрос шкалы пришел ответ");
            } break;
            case 3: {
                // Serial.println("Case 3 - OK");
                reply = 1;
            } break;
            case 4: {
                // Serial.println("Case 4 - OK");
                reply = 1;
            } break;
            case 5: {
                // Serial.println("Case 5 - OK");
                reply = 1;
            } break;
            case 6: {
                Serial.println("Case 6 - OK");
                reply = 1;
            } break;
            case 7: {
                Serial.println("Case 7 - OK. ABC включен");
                reply = 1;
            } break;
            case 8: {
                Serial.println("Case 8 - OK. ABC выключен");
                reply = 1;
            } break;
            case 9: {
                Serial.println("Case 9 - OK. Установлена шкала 0-2000");
                reply = 1;
                prevRange = 2000;
            } break;
            case 10: {
                Serial.println("Case 9 - OK. Установлена шкала 0-5000");
                reply = 1;
                prevRange = 5000;
            } break;
            case 11: {
                reply = 1;
                Serial.println("Запрос No.11 - сработал. Калибровка. Установлен нулевой уровень");
            } break;

            case 12: {
                reply = 1;
                Serial.println("Запрос No.12 - сработал. Сброс произошел");
            } break;

            case 13: {
                reply = 1;
                temperature = response[4] - 44;  // - 40;

                Serial.println("Запрос No.12 - сработал. Температура получена");
                Serial.println(temperature);
            } break;

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
