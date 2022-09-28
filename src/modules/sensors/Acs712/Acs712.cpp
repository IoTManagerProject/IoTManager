#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;

class Acs712 : public IoTItem
{
private:
    unsigned int _pin;
    const unsigned long _sampleTime = 100000UL;                      // sample over 100ms, it is an exact number of cycles for both 50Hz and 60Hz mains
    const unsigned long _numSamples = 250UL;                         // choose the number of samples to divide sampleTime exactly, but low enough for the ADC to keep up
    const unsigned long _sampleInterval = _sampleTime / _numSamples; // the sampling interval, must be longer than then ADC conversion time
    int _adc_zero1; //Переменная автоматической калибровки

public:

    Acs712(String parameters) : IoTItem(parameters)
    {
        String tmp;
        jsonRead(parameters, "pin", tmp);
        _pin = tmp.toInt();
        _adc_zero1 = determineVQ(_pin); 
    }
    
    void doByInterval()
    {
       
            unsigned long currentAcc = 0;
            unsigned int count = 0;
            unsigned long prevMicros = micros() - _sampleInterval;
            while (count < _numSamples)
            {
                if (micros() - prevMicros >= _sampleInterval)
                {
                    int adc_raw = IoTgpio.analogRead(_pin) - _adc_zero1;
                    currentAcc += (unsigned long)(adc_raw * adc_raw);
                    ++count;
                    prevMicros += _sampleInterval;
                }
            }
            #ifdef ESP32
            value.valD = int(sqrt((float)currentAcc / (float)_numSamples) * (75.7576 / 4095.0));
            #else
            value.valD = int(sqrt((float)currentAcc / (float)_numSamples) * (75.7576 / 1023.0));
            #endif
            regEvent(value.valD, "Acs712");
    }
  
    int determineVQ(int PIN)
    {
        long VQ = 0;
        // read 5000 samples to stabilise value
        for (int i = 0; i < 5000; i++)
        {
            VQ += IoTgpio.analogRead(PIN);
            //delay(1); // depends on sampling (on filter capacitor), can be 1/80000 (80kHz) max.
        }
        VQ /= 5000;
        return int(VQ);
    }

    
    ~Acs712(){};
};

void *getAPI_Acs712(String subtype, String param)
{
    if (subtype == F("Acs712"))
    {
        return new Acs712(param);
    }
    else
    {
        return nullptr;
    }
}
