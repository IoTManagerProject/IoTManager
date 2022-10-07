#include "Global.h"
#include "classes/IoTItem.h"
//
#include "arduinoFFT.h"

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

uint16_t samples = 512;           // This value MUST ALWAYS be a power of 2
const uint16_t maxSamples = 1024; // This value MUST ALWAYS be a power of 2

double samplingFrequency = 10000; // Hz, must be less than 10000 due to ADC

unsigned int sampling_period_us;

double vReal[maxSamples];
double vImag[maxSamples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

double Sampling();
void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType);

unsigned int freqMeterPin; // Esp32 Analog pins: 32, 33, 34, 35, 36, 39

int FreqMeterFirstSensor = 0;

IoTItem *item_FreqMeterF = nullptr; // pointer
IoTItem *item_FreqMeterPcFl = nullptr;
IoTItem *item_FreqMeterFlIn = nullptr;

void FreqMeterRequest(int sensorID);

double PercentFlickerMax;
double FlickerIndex;

extern IoTGpio IoTgpio;

//Это файл сенсора, в нем осуществляется чтение сенсора.
class FreqMeterF : public IoTItem
{
private:
  //=======================================================================================================
  // Секция переменных.
  //Это секция где Вы можете объявлять переменные и объекты arduino библиотек, что бы
  //впоследствии использовать их в loop и setup

public:
  //=======================================================================================================
  //это аналог setup из arduino. Здесь вы можете выполнять методы инициализации сенсора.

  FreqMeterF(String parameters) : IoTItem(parameters)
  {
    item_FreqMeterF = this;

#ifdef ESP8266
    freqMeterPin = 0;
#endif
#ifdef ESP32
    freqMeterPin = jsonReadInt(parameters, "pin-Esp32");
#endif

    pinMode(freqMeterPin, INPUT);

    samples = jsonReadInt(parameters, "samples");
    if (samples != 64 && samples != 128 && samples != 256 && samples != 512 && samples != 1024)
    {
      samples = 512;
    }
    SerialPrint("i", "Sensor FreqMeterF", "samples = " + String(samples));

    samplingFrequency = jsonReadInt(parameters, "samplingFrequency");
    SerialPrint("i", "Sensor FreqMeterF", "samplingFrequency = " + String(samplingFrequency));

    sampling_period_us = round(1000000 * (1.0 / samplingFrequency));
    // FreqMeterF_On = 1;
    FreqMeterFirstSensor = 0;
  }

  //=======================================================================================================
  // doByInterval()
  //это аналог loop из arduino, но вызываемый каждые int секунд, заданные в настройках. Здесь вы должны выполнить чтение вашего сенсора
  //а затем выполнить regEvent - это регистрация произошедшего события чтения
  //здесь так же доступны все переменные из секции переменных, и полученные в setup
  //если у сенсора несколько величин то делайте несколько regEvent
  void doByInterval()
  {
    FreqMeterRequest(1);
    // double frequency = Sampling();
    // value.valD = frequency;

    // regEvent(value.valD, "FreqMeterF"); //обязательный вызов хотяб один
  }
  //=======================================================================================================

  ~FreqMeterF(){};
};

//Это файл сенсора, в нем осуществляется чтение сенсора. Percent Flicker
class FreqMeterPcFl : public IoTItem
{
private:
  //=======================================================================================================
  // Секция переменных.
  //Это секция где Вы можете объявлять переменные и объекты arduino библиотек, что бы
  //впоследствии использовать их в loop и setup

public:
  //=======================================================================================================
  //это аналог setup из arduino. Здесь вы можете выполнять методы инициализации сенсора.

  FreqMeterPcFl(String parameters) : IoTItem(parameters)
  {
    item_FreqMeterPcFl = this;

#ifdef ESP8266
    freqMeterPin = 0;
#endif
#ifdef ESP32
    freqMeterPin = jsonReadInt(parameters, "pin-Esp32");
#endif

    pinMode(freqMeterPin, INPUT);

    samples = jsonReadInt(parameters, "samples");
    if (samples != 64 && samples != 128 && samples != 256 && samples != 512 && samples != 1024 && samples != 2048)
    {
      samples = 512;
    }
    SerialPrint("i", "Sensor FreqMeterF", "samples = " + String(samples));

    samplingFrequency = jsonReadInt(parameters, "samplingFrequency");
    SerialPrint("i", "Sensor FreqMeterF", "samplingFrequency = " + String(samplingFrequency));

    sampling_period_us = round(1000000 * (1.0 / samplingFrequency));
    FreqMeterFirstSensor = 0;
  }

  //=======================================================================================================
  // doByInterval()
  //это аналог loop из arduino, но вызываемый каждые int секунд, заданные в настройках. Здесь вы должны выполнить чтение вашего сенсора
  //а затем выполнить regEvent - это регистрация произошедшего события чтения
  //здесь так же доступны все переменные из секции переменных, и полученные в setup
  //если у сенсора несколько величин то делайте несколько regEvent
  void doByInterval()
  {
    FreqMeterRequest(2);
    // double frequency = Sampling();
    // value.valD = frequency;

    // regEvent(value.valD, "FreqMeterPFl"); //обязательный вызов хотяб один
  }
  //=======================================================================================================

  ~FreqMeterPcFl(){};
};

//Это файл сенсора, в нем осуществляется чтение сенсора. Flicker Index
class FreqMeterFlIn : public IoTItem
{
private:
  //=======================================================================================================
  // Секция переменных.
  //Это секция где Вы можете объявлять переменные и объекты arduino библиотек, что бы
  //впоследствии использовать их в loop и setup

public:
  //=======================================================================================================
  //это аналог setup из arduino. Здесь вы можете выполнять методы инициализации сенсора.

  FreqMeterFlIn(String parameters) : IoTItem(parameters)
  {
    item_FreqMeterFlIn = this;

#ifdef ESP8266
    freqMeterPin = 0;
#endif
#ifdef ESP32
    freqMeterPin = jsonReadInt(parameters, "pin-Esp32");
#endif

    pinMode(freqMeterPin, INPUT);

    samples = jsonReadInt(parameters, "samples");
    if (samples != 64 && samples != 128 && samples != 256 && samples != 512 && samples != 1024 && samples != 2048)
    {
      samples = 512;
    }
    SerialPrint("i", "Sensor FreqMeterF", "samples = " + String(samples));

    samplingFrequency = jsonReadInt(parameters, "samplingFrequency");
    SerialPrint("i", "Sensor FreqMeterF", "samplingFrequency = " + String(samplingFrequency));

    sampling_period_us = round(1000000 * (1.0 / samplingFrequency));
    FreqMeterFirstSensor = 0;
  }

  //=======================================================================================================
  // doByInterval()
  //это аналог loop из arduino, но вызываемый каждые int секунд, заданные в настройках. Здесь вы должны выполнить чтение вашего сенсора
  //а затем выполнить regEvent - это регистрация произошедшего события чтения
  //здесь так же доступны все переменные из секции переменных, и полученные в setup
  //если у сенсора несколько величин то делайте несколько regEvent
  void doByInterval()
  {
    FreqMeterRequest(3);
    // double frequency = Sampling();
    // value.valD = frequency;
    // regEvent(value.valD, "FreqMeterFlIn"); //обязательный вызов хотяб один
  }
  //=======================================================================================================

  ~FreqMeterFlIn(){};
};

//после замены названия сенсора, на функцию можно не обращать внимания
//если сенсор предполагает использование общего объекта библиотеки для нескольких экземпляров сенсора, то в данной функции необходимо предусмотреть
//создание и контроль соответствующих глобальных переменных
void *getAPI_FreqMeter(String subtype, String param)
{
  if (subtype == F("FreqMeterF"))
  {
    return new FreqMeterF(param);
  }
  else if (subtype == F("FreqMeterPcFl"))
  {
    return new FreqMeterPcFl(param);
  }
  else if (subtype == F("FreqMeterFlIn"))
  {
    return new FreqMeterFlIn(param);
  }
  else
  {
    return nullptr;
  }
}

void FreqMeterRequest(int sensorID)
{

  if (FreqMeterFirstSensor == 0)
    FreqMeterFirstSensor = sensorID;

  if (FreqMeterFirstSensor == sensorID)
  {
    double frequency = Sampling();
    if (item_FreqMeterF)
    {
      item_FreqMeterF->value.valD = frequency;
      item_FreqMeterF->regEvent(item_FreqMeterF->value.valD, "FreqMeterF");
    }

    if (item_FreqMeterPcFl)
    {
      item_FreqMeterPcFl->value.valD = PercentFlickerMax;
      item_FreqMeterPcFl->regEvent(item_FreqMeterPcFl->value.valD, "FreqMeterPcFl");
    }

    if (item_FreqMeterFlIn)
    {
      item_FreqMeterFlIn->value.valD = FlickerIndex;
      item_FreqMeterFlIn->regEvent(item_FreqMeterFlIn->value.valD, "FreqMeterFlIn");
    }
  }
  else
  {
    //пропускаем вызов от остальных сенсоров
  }
}

double Sampling()
{
 // double ADCRange = 0;
  double MaxADCValue = 0;

#ifdef ESP8266
  double MinADCValue = 1023;
#endif

#ifdef ESP32
  double MinADCValue = 4095;
#endif

  double analogReadSum = 0;
  //double PercentFlickerSum = 0;

  unsigned long microseconds = micros();

  for (int i = 0; i < samples; i++)
  {

    vReal[i] = analogRead(freqMeterPin);
    analogReadSum += vReal[i];
    vImag[i] = 0;
    while (micros() - microseconds < sampling_period_us)
    {
      // empty loop
    }
    microseconds += sampling_period_us;
    double A1Value = vReal[i];

    if (A1Value > MaxADCValue)
    {
      MaxADCValue = A1Value;
    }

    if (A1Value < MinADCValue)
    {
      MinADCValue = A1Value;
    }
    // заготовка под другие сенсоры
    /*
        if (i > 3)
        {
          double PercentFlicker = 100 * (MaxADCValue - MinADCValue) / (MaxADCValue + MinADCValue);
          PercentFlickerSum += PercentFlicker;
        }
        */
  }
  double analogReadAv = (MaxADCValue + MinADCValue) / 2;
   // double analogReadAv = analogReadSum / samples;

  double area1 = 0;
  double area2 = 0;

  for (int i = 0; i < samples; i++)
  {
    if (vReal[i] > analogReadAv)
    {
      area1 += (vReal[i] - analogReadAv)* sampling_period_us;
      area2 += analogReadAv * sampling_period_us;
    }
    else
    {
      area2 += vReal[i] * sampling_period_us;
    }
  }

  FlickerIndex = area1 / (area1 + area2);

  // double PercentFlickerAv = PercentFlickerSum / (samples - 4);

  PercentFlickerMax = 100 * (MaxADCValue - MinADCValue) / (MaxADCValue + MinADCValue);

 // ADCRange = MaxADCValue - MinADCValue;

  /* Print the results of the sampling according to time */
  // Serial.println("Data:");
  PrintVector(vReal, samples, SCL_TIME);
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD); /* Weigh data */
  // Serial.println("Weighed data:");
  PrintVector(vReal, samples, SCL_TIME);
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
  // Serial.println("Computed Real values:");
  PrintVector(vReal, samples, SCL_INDEX);
  // Serial.println("Computed Imaginary values:");
  PrintVector(vImag, samples, SCL_INDEX);
  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
  //  Serial.println("Computed magnitudes:");
  PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);
  double x = FFT.MajorPeak(vReal, samples, samplingFrequency);
  // Serial.print("dominant frequency = ");
  // Serial.println(x, 6); // Print out what frequency is the most dominant.
  //  while(1); /* Run Once */
  double frequency = x;
  return frequency;
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
    case SCL_INDEX:
      abscissa = (i * 1.0);
      break;
    case SCL_TIME:
      abscissa = ((i * 1.0) / samplingFrequency);
      break;
    case SCL_FREQUENCY:
      abscissa = ((i * 1.0 * samplingFrequency) / samples);
      break;
    }
    //  Serial.print(abscissa, 6);
    if (scaleType == SCL_FREQUENCY)
      ;
    //   Serial.print("Hz");
    //   Serial.print(" ");
    //   Serial.println(vData[i], 4);
  }
  // Serial.println();
}