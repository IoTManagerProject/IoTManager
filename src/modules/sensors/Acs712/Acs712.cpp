#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;

#ifdef ESP32
    #define SC_ADC 4095. //Scale ADC
#else
    #define SC_ADC 1023. //Scale ADC
#endif

#define DEF_NAN 50 //количество отсчетов АПЦ которые мы принимаем за отсутствие датчика (в идеале 0, но все равно Acs712 выдает минимум 0,5В)

class Acs712 : public IoTItem
{
private:
    unsigned int _pin;
    const unsigned long _sampleTime = 100000UL;                      // sample over 100ms, it is an exact number of cycles for both 50Hz and 60Hz mains
    const unsigned long _numSamples = 250UL;                         // choose the number of samples to divide sampleTime exactly, but low enough for the ADC to keep up
    const unsigned long _sampleInterval = _sampleTime / _numSamples; // the sampling interval, must be longer than then ADC conversion time
    int _adc_zero1;                                                  // Переменная автоматической калибровки
    int _fl_rms;            // 1 - подсчет средне-квадратического тока (переменный), 0 - подсчет средне-арифмитического тока (постоянный)
    int _sens = 100;        //Чувствительность датчика тока: 5A = 185mВ/A, 20A = 100mВ/A, 30A = 66mВ/A     
    int _vref;              //"Vref (мВ) - Опороное наряжение питания Acs712, по умолчанию = 5000мВ",          
    float k ;               //Чувствительность(разрешение) Acs712 по току, сколько тока в одном отсчете АЦП, k=VACP/sens  (30А->74mА, 20A->49mA, 5A->26mA для esp8266)                                                                                           
    bool f_nan = false;     //Флаг отсутствия входа на АЦП
    float vacp;             //Напряжение в мВ для смещения одного разряда АЦП esp8266 = 4.887, esp32 = 1.221;  vacp = vcc*1000/1023

public:
    Acs712(String parameters) : IoTItem(parameters)
    {
        String tmp;
        jsonRead(parameters, "pin", tmp);
        _pin = tmp.toInt();
        jsonRead(parameters, "adczero", tmp);
        _adc_zero1 = tmp.toInt(); // determineVQ(_pin);
        jsonRead(parameters, "rms", tmp);
        _fl_rms = tmp.toInt();
        jsonRead(parameters, "sens", tmp);
        _sens = tmp.toInt();
        jsonRead(parameters, "vref", tmp);
        _vref = tmp.toInt();
        vacp = _vref/SC_ADC;
        k = vacp / (float)_sens; //коэффециент для домножения измерений АЦП   
    }

    void doByInterval()
    {
        f_nan = false;
        unsigned long currentAcc = 0;
        unsigned int count = 0;
        unsigned long prevMicros = micros() - _sampleInterval;
        while (count < _numSamples)
        {
            if (micros() - prevMicros >= _sampleInterval)
            {
                int adc_raw = IoTgpio.analogRead(_pin);
                if (adc_raw > DEF_NAN) f_nan = true; //Если за цикл измерений не было АЦП больше 50, то считаем что нет датчика
                adc_raw -= _adc_zero1;
                if (_fl_rms == 0)
                    currentAcc += (unsigned long)abs(adc_raw);
                else
                    currentAcc += (unsigned long)(adc_raw * adc_raw);
                ++count;
                prevMicros += _sampleInterval;
            }
        }

        if (_fl_rms == 0)
        {
#ifdef ESP32
            value.valD = ((float)currentAcc / (float)_numSamples) * k;     
#else
            value.valD = ((float)currentAcc / (float)_numSamples) * k;
#endif
        }
        else
        {
#ifdef ESP32
            value.valD = (sqrt((float)currentAcc / (float)_numSamples) * k);
#else
            value.valD = (sqrt((float)currentAcc / (float)_numSamples) * k);
#endif
        }
        if (f_nan)
            regEvent(value.valD, "Acs712");
        else
            regEvent(NAN, "Acs712");
    }

    void onModuleOrder(String &key, String &value)
    {
        if (key == "setZero")
        {
            _adc_zero1 = determineVQ(_pin);
            SerialPrint("i", F("Acs712"), "User run calibration ADC zero: " + String(_adc_zero1));
            // TODO wtitejson to config.json?????
        }
    }

    int determineVQ(int PIN)
    {
        long VQ = 0;
        for (int i = 0; i < 100; i++)
        {
            VQ += IoTgpio.analogRead(PIN);
        }
        VQ /= 100;
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
