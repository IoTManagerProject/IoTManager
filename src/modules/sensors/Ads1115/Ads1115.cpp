/******************************************************************
  Used Adafruit ADS1X15 Driver (16-bit Differential or Single-Ended ADCs with PGA and Comparator)
  Support for ADS1015/1115
  https://github.com/adafruit/Adafruit_ADS1X15

  adapted for version 4 @Serghei63
 ******************************************************************/

#include "Global.h"
#include "classes/IoTItem.h"

#include "Wire.h"
#include <Adafruit_ADS1X15.h>  // Библиотека для работы с модулями ADS1115 и ADS1015


class Ads1115 : public IoTItem {
    int _pin;
    bool _isRaw;
    bool _isInited = false;
    Adafruit_ADS1115 ads;

   public:
    Ads1115(String parameters) : IoTItem(parameters) {
        String tmp;
        jsonRead(parameters, "pin", tmp);
        _pin = tmp.toInt();

        jsonRead(parameters, "mode", tmp);
        _isRaw = tmp == "raw";

        if (!ads.begin()) {
            Serial.println("Failed to initialize ADS.");
            _isInited = false;
        } else
            _isInited = true;

        String gain;
        jsonRead(parameters, "gain", gain);
        if (gain == "1x")
            ads.setGain(GAIN_ONE);
        else if (gain == "2x")
            ads.setGain(GAIN_TWO);
        else if (gain == "4x")
            ads.setGain(GAIN_FOUR);
        else if (gain == "8x")
            ads.setGain(GAIN_EIGHT);
        else if (gain == "16x")
            ads.setGain(GAIN_SIXTEEN);
        else
            ads.setGain(GAIN_TWOTHIRDS);
        // ВОЗМОЖНЫЕ ВАРИАНТЫ УСТАНОВКИ КУ:
        // ads.setGain(GAIN_TWOTHIRDS); //| 2/3х | +/-6.144V | 1bit = 0.1875mV    |
        // ads.setGain(GAIN_ONE);       //| 1х   | +/-4.096V | 1bit = 0.125mV     |
        // ads.setGain(GAIN_TWO);       //| 2х   | +/-2.048V | 1bit = 0.0625mV    |
        // ads.setGain(GAIN_FOUR);      //| 4х   | +/-1.024V | 1bit = 0.03125mV   |
        // ads.setGain(GAIN_EIGHT);     //| 8х   | +/-0.512V | 1bit = 0.015625mV  |
        // ads.setGain(GAIN_SIXTEEN);   //| 16х  | +/-0.256V | 1bit = 0.0078125mV |
    }

    void doByInterval() {
        if (_isInited) {
            if (_isRaw)
                value.valD = ads.readADC_SingleEnded(_pin);  // Чтение АЦП нулевого канала(rawdata)
            else
                value.valD = ads.computeVolts(ads.readADC_SingleEnded(_pin));  // Чтение АЦП нулевого канала (Вольты)
            regEvent(value.valD, "ADC1115");
        }
    }

    ~Ads1115(){};
};

void *getAPI_Ads1115(String subtype, String param) {
    if (subtype == F("Ads1115")) {
        return new Ads1115(param);
    } else {
        return nullptr;
    }
}

// {
//     "name": "26. Датчик напряжения ADS1115",
//     "num": 26,
//     "type": "Reading",
//     "subtype": "Ads1115",
//     "id": "Ads3",
//     "widget": "anydataVlt",
//     "page": "Сенсоры",
//     "descr": "ADS_3",

//     "pin": "0",
//     "mode": "volt",
//     "gain": "3/4x",

//     "plus": 0,
//     "multiply": 1,
//     "round": 100,
//     "int": 10
// }