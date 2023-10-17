#include "Global.h"
#include "classes/IoTItem.h"

#include "Wire.h"
#include <Adafruit_PCF8591.h>

// Make sure that this is set to the value in volts of VCC
#define ADC_REFERENCE_VOLTAGE 3.3

class Pcf8591 : public IoTItem {
    int _pin;
    bool _isRaw;
    bool _isInited = false;
    Adafruit_PCF8591 pcf = Adafruit_PCF8591();

   public:
    Pcf8591(String parameters) : IoTItem(parameters) {
        String tmp;
        jsonRead(parameters, "pin", tmp);
        _pin = tmp.toInt();

        jsonRead(parameters, "mode", tmp);
        _isRaw = tmp == "raw";

        if (!pcf.begin()) {
             Serial.println("# Adafruit PCF8591 not found!");
            _isInited = false;
        } else

            _isInited = true;

            Serial.println("# Adafruit PCF8591 found");

             pcf.enableDAC(true);

             Serial.println("AIN0, AIN1, AIN2, AIN3");

    }

    uint8_t dac_counter = 0;

    void doByInterval() {

          // Make a triangle wave on the DAC output
  pcf.analogWrite(dac_counter++);

        if (_isInited) {
            if (_isRaw)
               value.valD = pcf.analogRead(_pin);  // Чтение АЦП нулевого канала (Вольты)
            else
                value.valD = (int_to_volts(pcf.analogRead(_pin), 8, ADC_REFERENCE_VOLTAGE));
            regEvent(value.valD, "PCF8591");
       }

}

        float int_to_volts(uint16_t dac_value, uint8_t bits, float logic_level) {
  return (((float)dac_value / ((1 << bits) - 1)) * logic_level);
  
    }

    ~Pcf8591(){};
};

void *getAPI_Pcf8591(String subtype, String param) {
    if (subtype == F("Pcf8591")) {
        return new Pcf8591(param);
    } else {
        return nullptr;
    }
}
