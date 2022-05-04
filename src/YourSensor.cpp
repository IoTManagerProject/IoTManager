#include "YourSensor.h"

#include "Global.h"
#include "Utils/JsonUtils.h"
#include "Utils/StringUtils.h"

//подключаем необходимые файлы библиотеки
//#include "Adafruit_ADS1X15.h"
#include "Adafruit_AHTX0.h"
#include "BH1750.h"
#include "ClosedCube_HDC1080.h"


//создаем объект HDC1080
ClosedCube_HDC1080 hdc1080;

//создаем объект AHTX0
Adafruit_AHTX0 aht;
Adafruit_Sensor *aht_humidity, *aht_temp;
sensors_event_t tmpEvent_t;


//создаем объект BH1750
BH1750 lightMeter;

//создаем объект ADS1015
// Adafruit_ADS1015 ads;

// GND --- термистор --- A0 --- 10к --- 5V
#include <GyverNTC.h>
GyverNTC therm(0, 10000, 3435);	// пин, R термистора при 25 градусах, B термистора. (R резистора по умолч. 10000)
// GyverNTC therm(0, 10000, 3435, 25, 10000);	// пин, R термистора, B термистора, базовая температура, R резистора
// серый 4300
// проводной 3950

#include "SdsDustSensor.h"


//встроена в ядро для 8266, для 32 по этому же имени обращаемся к другой библиотеке plerup/EspSoftwareSerial
#include <SoftwareSerial.h>

// to do убрать глобальный экземпляр
#ifdef ESP8266
int rxPinSDS = 13;  // D7 – подключаем к Tx сенсора
int txPinSDS = 12;  // D6 – подключаем к Rx сенсора

SdsDustSensor sds(rxPinSDS, txPinSDS);
#endif
#ifdef ESP32
#include <HardwareSerial.h>
HardwareSerial sdsSerial(2);
SdsDustSensor sds(sdsSerial);
#endif

//int rxPinSDS = 12; // D6 – подключаем к Tx сенсора
//int txPinSDS = 14; // D5 – подключаем к Rx сенсора





// for CO2 MH-Z19С
#define pwmPin 15 // желтый провод сенсора к D8    (14-D5 тоже ok)
int rxPinCO2 = 4;// зеленый провод сенсора к D2
int txPinCO2 = 5;// синий провод сенсора к D1
SoftwareSerial swSerialCO2(rxPinCO2, txPinCO2); // 
int MHZ19_PREHEATING_TIME = 2 * 60 * 1000;// покажет реальные данные после прогрева, через 2 мин.

#include "max6675.h"

#ifdef ESP8266
int thermoDO = 12;     // MISO
int thermoCS = 13;
int thermoCLK = 14;
#endif

#ifdef ESP32
int thermoDO = 12;     // MISO
int thermoCS = 13;
int thermoCLK = 14;
#endif

//создаем объект MAX6675
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
#include "Wire.h"
#include <WEMOS_SHT3X.h>

SHT3X sht30(0x45);

//===================================================================HC4067===============================================
    /* 0- General */
#ifdef ESP8266
    #define S0 D0   /* Assign Multiplexer pin S0 connect to pin D0 of NodeMCU */
    #define S1 D1   /* Assign Multiplexer pin S1 connect to pin D1 of NodeMCU */
    #define S2 D2   /* Assign Multiplexer pin S2 connect to pin D2 of NodeMCU */
    #define S3 D3   /* Assign Multiplexer pin S3 connect to pin D3 of NodeMCU */
    #define SIG A0  /* Assign SIG pin as Analog output for all 16 channels of Multiplexer to pin A0 of NodeMCU */
#endif

#ifdef ESP32
    #define S0 12   /* Assign Multiplexer pin S0 connect to pin D0 of NodeMCU */
    #define S1 13   /* Assign Multiplexer pin S1 connect to pin D1 of NodeMCU */
    #define S2 14   /* Assign Multiplexer pin S2 connect to pin D2 of NodeMCU */
    #define S3 15   /* Assign Multiplexer pin S3 connect to pin D3 of NodeMCU */
    #define SIG A0  /* Assign SIG pin as Analog output for all 16 channels of Multiplexer to pin A0 of NodeMCU */
#endif
    int decimal = 2; // Decimal places of the sensor value outputs 
    int sensor0;     /* Assign the name "sensor0" as analog output value from Channel C0 */
    int sensor1;     /* Assign the name "sensor1" as analog output value from Channel C1 */
    int sensor2;     /* Assign the name "sensor2" as analog output value from Channel C2 */
    int sensor3;     /* Assign the name "sensor3" as analog output value from Channel C3 */
    int sensor4;     /* Assign the name "sensor4" as analog output value from Channel C4 */
    int sensor5;     /* Assign the name "sensor5" as analog output value from Channel C5 */
    int sensor6;     /* Assign the name "sensor6" as analog output value from Channel C6 */
    int sensor7;     /* Assign the name "sensor7" as analog output value from Channel C7 */
    int sensor8;     /* Assign the name "sensor8" as analog output value from Channel C8 */
    int sensor9;     /* Assign the name "sensor9" as analog output value from Channel C9 */
    int sensor10;    /* Assign the name "sensor10" as analog output value from Channel C10 */
    int sensor11;    /* Assign the name "sensor11" as analog output value from Channel C11 */
    int sensor12;    /* Assign the name "sensor12" as analog output value from Channel C12 */
    int sensor13;    /* Assign the name "sensor13" as analog output value from Channel C13 */
    int sensor14;    /* Assign the name "sensor14" as analog output value from Channel C14 */
    int sensor15;    /* Assign the name "sensor15" as analog output value from Channel C15 */

  #include "Adafruit_INA219.h"
  //создаем объект INA219
  Adafruit_INA219 ina219;

  #include <Adafruit_ADS1X15.h> // Библиотека для работы с модулями ADS1115 и ADS1015
Adafruit_ADS1115 ads; // Создание объекта ads для общения через него с модулем
int16_t adc0, adc1, adc2, adc3;

//==========================================HX711=======================================
  #include "HX711.h"

  // HX711 circuit wiring
  const int LOADCELL_DOUT_PIN = 13; // D7
  const int LOADCELL_SCK_PIN = 14;  // D5

  HX711 scale;


//-------------------------------------------------------MCP 23017-----------------------------------------------------------------------
#include <Wire.h>
#include "Adafruit_MCP23017.h"

#define   MCP23017_ADDR 0x20
Adafruit_MCP23017 mcp;
String status;

#define   MCP23017_GPA0 0
#define   MCP23017_GPA1 1
#define   MCP23017_GPA2 2
#define   MCP23017_GPA3 3
#define   MCP23017_GPA4 4
#define   MCP23017_GPA5 5
#define   MCP23017_GPA6 6
#define   MCP23017_GPA7 7
#define   MCP23017_GPB0 8
#define   MCP23017_GPB1 9
#define   MCP23017_GPB2 10
#define   MCP23017_GPB3 11
#define   MCP23017_GPB4 12
#define   MCP23017_GPB5 13
#define   MCP23017_GPB6 14
#define   MCP23017_GPB7 15



float yourSensorReading(String type, String paramsAny) {
    float value;



//======================================================== MCP23017================================================================
    if (type == "MCP23_0")
    {
          MCP23017_init();
        bool value_D0 = mcp.digitalRead(  MCP23017_GPA0);
        value = value_D0;
    }
    if (type == "MCP23_1")
    {
          MCP23017_init();
        bool value_D1 = mcp.digitalRead(  MCP23017_GPA1);
        value = value_D1;
    }
    if (type == "MCP23_2")
    {
          MCP23017_init();
        bool value_D2 = mcp.digitalRead(  MCP23017_GPA2);
        value = value_D2;
    }
    if (type == "MCP23_3")
    {
          MCP23017_init();
        bool value_D3 = mcp.digitalRead(  MCP23017_GPA3);
        value = value_D3;
    }
    if (type == "MCP23_4")
    {
          MCP23017_init();
        bool value_D4 = mcp.digitalRead(  MCP23017_GPA4);
        value = value_D4;
    }
    if (type == "MCP23_5")
    {
          MCP23017_init();
        bool value_D5 = mcp.digitalRead(  MCP23017_GPA5);
        value = value_D5;
    }
    if (type == "MCP23_6")
    {
          MCP23017_init();
        // orderBuf =+ "btn884"
        bool value_D6 = mcp.digitalRead(  MCP23017_GPA6);
        value = value_D6;
    }
    if (type == "MCP23_7")
    {
          MCP23017_init();
        bool value_D7 = mcp.digitalRead(  MCP23017_GPA7);
        value = value_D7;
    }
    //-------------------------------------------------------MCP_23017 OUT------------------------------------------------------
    if (type == "MCP23_8")
    {
        String BNTN_1 = jsonReadStr(configStoreJson, "btn1111");
        if (BNTN_1 == "1")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB0, HIGH);
            bool value_D8 = mcp.digitalRead(  MCP23017_GPB0);
            value = value_D8;
        }
    }
    if (type == "MCP23_8_1")
    {
        String BNTN_1 = jsonReadStr(configStoreJson, "btn1111");
        if (BNTN_1 == "0")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB0, LOW);
            bool value_D8 = mcp.digitalRead(  MCP23017_GPB0);
            value = value_D8;
        }
    }
    if (type == "MCP23_9")
    {
        String BNTN_2 = jsonReadStr(configStoreJson, "btn2222");
        if (BNTN_2 == "1")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB1, HIGH);
            bool value_D9 = mcp.digitalRead(  MCP23017_GPB1);
            value = value_D9;
        }
    }
    if (type == "MCP23_9_1")
    {
        String BNTN_2 = jsonReadStr(configStoreJson, "btn2222");
        if (BNTN_2 == "0")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB1, LOW);
            bool value_D9 = mcp.digitalRead(  MCP23017_GPB1);
            value = value_D9;
        }
    }

    if (type == "MCP23_10")
    {
        String BNTN_3 = jsonReadStr(configStoreJson, "btn3333");
        if (BNTN_3 == "1")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB2, HIGH);
            bool value_D10 = mcp.digitalRead(  MCP23017_GPB2);
            value = value_D10;
        }
    }

    if (type == "MCP23_10_1")
    {
        String BNTN_3 = jsonReadStr(configStoreJson, "btn3333");
        if (BNTN_3 == "0")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB2, LOW);
            bool value_D10 = mcp.digitalRead(  MCP23017_GPB2);
            value = value_D10;
        }
    }

    if (type == "MCP23_11")
    {
        String BNTN_4 = jsonReadStr(configStoreJson, "btn4444");
        if (BNTN_4 == "1")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB3, HIGH);
            bool value_D11 = mcp.digitalRead(  MCP23017_GPB3);
            value = value_D11;
        }
    }
    if (type == "MCP23_11_1")
    {
        String BNTN_4 = jsonReadStr(configStoreJson, "btn4444");
        if (BNTN_4 == "0")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB3, LOW);
            bool value_D11 = mcp.digitalRead(  MCP23017_GPB3);
            value = value_D11;
        }
    }
    if (type == "MCP23_12")
    {
        String BNTN_5 = jsonReadStr(configStoreJson, "btn5555");
        if (BNTN_5 == "1")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB4, HIGH);
            bool value_D12 = mcp.digitalRead(  MCP23017_GPB4);
            value = value_D12;
        }
    }
    if (type == "MCP23_12_1")
    {
        String BNTN_5 = jsonReadStr(configStoreJson, "btn5555");
        if (BNTN_5 == "0")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB4, LOW);
            bool value_D12 = mcp.digitalRead(  MCP23017_GPB4);
            value = value_D12;
        }
    }
    if (type == "MCP23_13")
    {
        String BNTN_6 = jsonReadStr(configStoreJson, "btn6666");
        if (BNTN_6 == "1")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB5, HIGH);
            bool value_D13 = mcp.digitalRead(  MCP23017_GPB5);
            value = value_D13;
        }
    }
    if (type == "MCP23_13_1")
    {
        String BNTN_6 = jsonReadStr(configStoreJson, "btn666");
        if (BNTN_6 == "0")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB5, LOW);
            bool value_D13 = mcp.digitalRead(  MCP23017_GPB5);
            value = value_D13;
        }
    }
    if (type == "MCP23_14")
    {
        String BNTN_7 = jsonReadStr(configStoreJson, "btn7777");
        if (BNTN_7 == "1")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB6, HIGH);
            bool value_D14 = mcp.digitalRead(  MCP23017_GPB6);
            value = value_D14;
        }
    }
    if (type == "MCP23_14_1")
    {
        String BNTN_7 = jsonReadStr(configStoreJson, "btn7777");
        if (BNTN_7 == "0")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB6, LOW);
            bool value_D14 = mcp.digitalRead(  MCP23017_GPB6);
            value = value_D14;
        }
    }
    if (type == "MCP23_15")
    {
        String BNTN_8 = jsonReadStr(configStoreJson, "btn8888");
        if (BNTN_8 == "1")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB7, HIGH);
            bool value_D15 = mcp.digitalRead(  MCP23017_GPB7);
            value = value_D15;
        }
    }
    if (type == "MCP23_15_1")
    {
        String BNTN_8 = jsonReadStr(configStoreJson, "btn8888");
        if (BNTN_8 == "0")
        {
              MCP23017_init();
            mcp.digitalWrite(  MCP23017_GPB7, LOW);
            bool value_D15 = mcp.digitalRead(  MCP23017_GPB7);
            value = value_D15;
        }
    }   

//===================================================HX711====================================

	      if (type == "HX711") {
        HX711_init();
          if (scale.is_ready()) {
    long reading = scale.read();
    value = reading;
    Serial.print("HX711 reading: ");
    Serial.println(reading);
        } else {
    Serial.println("HX711 not found.");
        }
      }
//===================================================ADS1015==================================
    if (type == "adc0") {
    ADS1115_init();
  adc0 = ads.readADC_SingleEnded(0); // Чтение АЦП нулевого канала
  value = adc0;
  } 
if (type == "adc1"){
  adc1 = ads.readADC_SingleEnded(1); // Чтение АЦП первого канала
  value = adc1;
}
if (type == "adc2"){
  adc2 = ads.readADC_SingleEnded(2); // Чтение АЦП второго канала
  value = adc2;
}
if (type == "adc3"){
  adc3 = ads.readADC_SingleEnded(3); // Чтение АЦП третьего канала
  value = adc3;
}
if (type == "u0") {
  float u0 = float(adc0) * 0.1875 / 1000.0;
  value = u0;}
if (type == "u1") {
  float u1 = float(adc1) * 0.1875 / 1000.0;
  value = u1;}
if (type == "u2") {
  float u2 = float(adc2) * 0.1875 / 1000.0;
  value = u2;}
if (type == "u3") {
  float u3 = float(adc3) * 0.1875 / 1000.0;
  value = u3;}
//=============================================INA219============================================
  uint32_t currentFrequency;
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

      if (type == "INA_shvlt") {
        INA219_init();
        value = ina219.getShuntVoltage_mV();
      }
      if (type == "INA_vlt") {
        INA219_init();
        value = ina219.getBusVoltage_V();
      }
      if (type == "INA_curr") {
        INA219_init();
        value = ina219.getCurrent_mA();
      }
      if (type == "INA_pow") {
        INA219_init();
        value = ina219.getPower_mW();
      }
      if (type == "INA_lvlt") {
        INA219_init();
        value = busvoltage + (shuntvoltage / 1000);
      }

//====================================================SHT30===================================
    if (type == "SHT30t") {
        if(sht30.get()==0){
        SHT30_init();
        value = sht30.cTemp;

        }          else
  {
    Serial.println("Error!"); // Ошибка, если датчик не ответил
  }
    }
    if (type == "SHT30h") {
        if(sht30.get()==0){
        SHT30_init();
        value = sht30.humidity;
        }
                  else
  {
    Serial.println("Error!"); // Ошибка, если датчик не ответил
  }
    }

    //========================================================MAX6675=========================================
    if (type == "MAX6675_temp") {
        MAX6675_init();
        value = thermocouple.readCelsius();
    }

    //===============================================================MHZ19========================

     if (type == "CO2uart"){
        MHZ19_init();
        if ( millis() > MHZ19_PREHEATING_TIME) {

    Serial.println("Start checkUartCO2");

    byte measure_cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    unsigned char measure_response[9];
    unsigned long th, tl, ppm = 0, ppm2 = 0, ppm3 = 0;

    // ***** узнаём концентрацию CO2 через UART: *****
    swSerialCO2.write(measure_cmd, 9);
    swSerialCO2.readBytes(measure_response, 9);
    int i;
    byte crc = 0;
    for (i = 1; i < 8; i++) crc += measure_response[i];
    crc = 255 - crc;
    crc += 1;
    if ( !(measure_response[0] == 0xFF && measure_response[1] == 0x86 && measure_response[8] == crc) ) {
      //Serial.println("CRC error: " + String(crc) + " / " + String(measure_response[8]));
    }
    unsigned int responseHigh = (unsigned int) measure_response[2];
    unsigned int responseLow = (unsigned int) measure_response[3];
    // unsigned int ppm = (256*responseHigh) + responseLow;
    ppm = (256 * responseHigh) + responseLow;

    int temperature = measure_response[4] - 44;  // - 40;

   // uartTempToLCD = temperature;
    value = ppm;
    Serial.print(ppm);
    Serial.print(" <- ppm (UART) ");
   // Serial.print((ppm / 5) * 2);
    //Serial.println(" <- two fifths of it"); // for scale [0-2000]


    Serial.print(temperature);
    Serial.println(" <- temperature");
        }

    }
    

    if (type == "CO2pwm"){
          // *****  узнаём концентрацию CO2 через PWM: *****
          // может зависать на пару минут
          // можно использовать только для проверки CO2uart
        MHZ19_init();
        if (millis() > MHZ19_PREHEATING_TIME) {
    Serial.println("Start checkPwmCO2");
    unsigned long th, tl, ppm = 0, ppm2 = 0, ppm3 = 0;
    do {
      th = pulseIn(pwmPin, HIGH, 1004000) / 1000;
      tl = 1004 - th;
      //ppm2 =  2000 * (th - 2) / (th + tl - 4); // расчёт для диапазона от 0 до 2000ppm
      ppm3 =  5000 * (th - 2) / (th + tl - 4); // расчёт для диапазона от 0 до 5000ppm
    } while (th == 0);

    Serial.print(ppm3);
    Serial.println(" <- ppm3 (PWM) with 5000ppm as limit");
    Serial.println("Completed checkPwmCO2");
   
    value = ppm3;
         }
    }


    //===========================================SDS011==============================

          PmResult pm = sds.readPm();

    if (type == "PM2.5") {
        SDS011_init();
        if (pm.isOk()) {
        value = pm.pm25;
       }
    }

        if (type == "PM10") {
        SDS011_init();
        if (pm.isOk()) {
        value = pm.pm10;
       }
    }

    //========================================================NTC=============================
    if (type == "NTC") {
        NTC_init();
        value = therm.getTempAverage();
    }
    //========================================================HDC1080================================================================
    if (type == "HDC1080_temp") {
        HDC1080_init(jsonReadStr(paramsAny, "addr"));
        value = hdc1080.readTemperature();
    }
    if (type == "HDC1080_hum") {
        HDC1080_init(jsonReadStr(paramsAny, "addr"));
        value = hdc1080.readHumidity();
    }
    //==========================================================AHTX0=================================================================
    if (type == "AHTX0_temp") {
        AHTX0_init();
        aht_temp->getEvent(&tmpEvent_t);
        value = tmpEvent_t.temperature;
    }
    if (type == "AHTX0_hum") {
        AHTX0_init();
        aht_humidity->getEvent(&tmpEvent_t);
        value = tmpEvent_t.relative_humidity;
    }
    //==========================================================BH1750=================================================================
    if (type == "BH1750_lux") {
        BH1750_init();
        value = lightMeter.readLightLevel();
    }
 
    //==========================================================timer=================================================================
    if (type == "timer") {
        value = jsonReadFloat(configLiveJson, jsonReadStr(paramsAny, "val"));
    }

    //=====================================================HC4067===================================================================
    if (type == "A_0") {
        HC4067_init();
    // Channel 0 (C0 pin - binary output 0,0,0,0)
    digitalWrite(S0,LOW); digitalWrite(S1,LOW); digitalWrite(S2,LOW); digitalWrite(S3,LOW);
    sensor0 = analogRead(SIG);
        value =  sensor0;
    }
        if (type == "A_1") {
        HC4067_init();
    // Channel 1 (C1 pin - binary output 1,0,0,0)
    digitalWrite(S0,HIGH); digitalWrite(S1,LOW); digitalWrite(S2,LOW); digitalWrite(S3,LOW);
    sensor1 = analogRead(SIG);
        value =  sensor1;
    }
        if (type == "A_2") {
        HC4067_init();
    // Channel 2 (C2 pin - binary output 0,1,0,0)
    digitalWrite(S0,LOW); digitalWrite(S1,HIGH); digitalWrite(S2,LOW); digitalWrite(S3,LOW);
    sensor2 = analogRead(SIG);
        value =  sensor2;
    }
        if (type == "A_3") {
        HC4067_init();
    // Channel 3 (C3 pin - binary output 1,1,0,0)
    digitalWrite(S0,HIGH); digitalWrite(S1,HIGH); digitalWrite(S2,LOW); digitalWrite(S3,LOW);
    sensor3 = analogRead(SIG);
        value =  sensor3;
    }
        if (type == "A_4") {
        HC4067_init();
    // Channel 4 (C4 pin - binary output 0,0,1,0)
    digitalWrite(S0,LOW); digitalWrite(S1,LOW); digitalWrite(S2,HIGH); digitalWrite(S3,LOW);
    sensor4 = analogRead(SIG);
        value =  sensor4;
    }
        if (type == "A_5") {
        HC4067_init();
    // Channel 5 (C5 pin - binary output 1,0,1,0)
    digitalWrite(S0,HIGH); digitalWrite(S1,LOW); digitalWrite(S2,HIGH); digitalWrite(S3,LOW);
    sensor5 = analogRead(SIG);
        value =  sensor5;
    }
        if (type == "A_6") {
        HC4067_init();
    // Channel 6 (C6 pin - binary output 0,1,1,0)
    digitalWrite(S0,LOW); digitalWrite(S1,HIGH); digitalWrite(S2,HIGH); digitalWrite(S3,LOW);
    sensor6 = analogRead(SIG);
        value =  sensor6;
    }
        if (type == "A_7") {
        HC4067_init();
    // Channel 7 (C7 pin - binary output 1,1,1,0)
    digitalWrite(S0,HIGH); digitalWrite(S1,HIGH); digitalWrite(S2,HIGH); digitalWrite(S3,LOW);
    sensor7 = analogRead(SIG);
        value =  sensor7;
    }
        if (type == "A_8") {
        HC4067_init();
    // Channel 8 (C8 pin - binary output 0,0,0,1)
    digitalWrite(S0,LOW); digitalWrite(S1,LOW); digitalWrite(S2,LOW); digitalWrite(S3,HIGH);
    sensor8 = analogRead(SIG);
        value =  sensor8;
    }
         if (type == "A_9") {
        HC4067_init();
    // Channel 9 (C9 pin - binary output 1,0,0,1)
    digitalWrite(S0,HIGH); digitalWrite(S1,LOW); digitalWrite(S2,LOW); digitalWrite(S3,HIGH);
    sensor9 = analogRead(SIG);
        value =  sensor9;
    }
        if (type == "A_10") {
        HC4067_init();
    // Channel 10 (C10 pin - binary output 0,1,0,1)
    digitalWrite(S0,LOW); digitalWrite(S1,HIGH); digitalWrite(S2,LOW); digitalWrite(S3,HIGH);
    sensor10 = analogRead(SIG);
        value =  sensor10;
    }
        if (type == "A_11") {
        HC4067_init();
    // Channel 11 (C11 pin - binary output 1,1,0,1)
    digitalWrite(S0,HIGH); digitalWrite(S1,HIGH); digitalWrite(S2,LOW); digitalWrite(S3,HIGH);
    sensor11 = analogRead(SIG);
        value =  sensor11;
    }
         if (type == "A_12") {
        HC4067_init();
    // Channel 12 (C12 pin - binary output 0,0,1,1)
    digitalWrite(S0,LOW); digitalWrite(S1,LOW); digitalWrite(S2,HIGH); digitalWrite(S3,HIGH);
    sensor12 = analogRead(SIG);
        value =  sensor12;
    }
        if (type == "A_13") {
        HC4067_init();
    // Channel 13 (C13 pin - binary output 1,0,1,1)
    digitalWrite(S0,HIGH); digitalWrite(S1,LOW); digitalWrite(S2,HIGH); digitalWrite(S3,HIGH);
    sensor13 = analogRead(SIG);
        value =  sensor13;
    }
        if (type == "A_14") {
        HC4067_init();
    // Channel 14 (C14 pin - binary output 0,1,1,1)
    digitalWrite(S0,LOW); digitalWrite(S1,HIGH); digitalWrite(S2,HIGH); digitalWrite(S3,HIGH);
    sensor14 = analogRead(SIG);
        value =  sensor14;
    }
        if (type == "A_15") {
        HC4067_init();
    // Channel 15 (C15 pin - binary output 1,1,1,1)
    digitalWrite(S0,HIGH); digitalWrite(S1,HIGH); digitalWrite(S2,HIGH); digitalWrite(S3,HIGH);
    sensor15 = analogRead(SIG);
        value =  sensor15;
    }

    return value;
}

void HDC1080_init(String addr) {
    static bool HDC1080_flag = true;
    if (HDC1080_flag) {
        hdc1080.begin(hexStringToUint8(addr));
        HDC1080_flag = false;
    }
}

void AHTX0_init() {
    static bool AHTX0_flag = true;
    if (AHTX0_flag) {
        if (!aht.begin()) {
            Serial.println("Failed to find AHT10/AHT20 chip");
            // return -127;
        }
        aht_temp = aht.getTemperatureSensor();
        aht_temp->printSensorDetails();

        aht_humidity = aht.getHumiditySensor();
        aht_humidity->printSensorDetails();
        AHTX0_flag = false;
    }
}



void BH1750_init() {
    static bool BH1750_flag = true;
    if (BH1750_flag) {
        lightMeter.begin();

        BH1750_flag = false;
    }
}
void NTC_init() {
    static bool NTC_flag = true;
    if (NTC_flag) {
        NTC_flag = false;
    }
}

void SDS011_init() {
    static bool SDS011_flag = true;
    if (SDS011_flag) {
        sds.begin();
        SDS011_flag = false;
    }
}

void MHZ19_init() {
        static bool MHZ19_flag = true;
        if (MHZ19_flag)
        {
            swSerialCO2.begin(9600);
            pinMode(pwmPin, INPUT);   
            MHZ19_flag = false;
        }
    }

void MAX6675_init() {
         static bool MAX6675_flag = true;
        if (MAX6675_flag) {
         MAX6675_flag = false;
        }
    }

void SHT30_init() {
   static bool SHT30_flag = true;
   if (SHT30_flag) {
       SHT30_flag = false;
   }
}

void HC4067_init() {
    static bool HC4067_flag = true;
    if (HC4067_flag) {
        HC4067_flag = false;
    }
}

//=================================INA219========================
void INA219_init() {
    static bool INA219_flag = true;
    if (INA219_flag) {
        ina219.begin();
        INA219_flag = false;
    }
}

void ADS1115_init() {
    ads.setGain(GAIN_TWOTHIRDS);  // ВОЗМОЖНЫЕ ВАРИАНТЫ УСТАНОВКИ КУ:
  // ads.setGain(GAIN_TWOTHIRDS); //| 2/3х | +/-6.144V | 1bit = 0.1875mV    |
  // ads.setGain(GAIN_ONE);       //| 1х   | +/-4.096V | 1bit = 0.125mV     |
  // ads.setGain(GAIN_TWO);       //| 2х   | +/-2.048V | 1bit = 0.0625mV    |
  // ads.setGain(GAIN_FOUR);      //| 4х   | +/-1.024V | 1bit = 0.03125mV   |
  // ads.setGain(GAIN_EIGHT);     //| 8х   | +/-0.512V | 1bit = 0.015625mV  |
  // ads.setGain(GAIN_SIXTEEN);   //| 16х  | +/-0.256V | 1bit = 0.0078125mV |
    ads.begin();
 }
 //=================================HX711========================
void HX711_init() {
    static bool HX711_flag = true;
    if (HX711_flag) {
        scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
        HX711_flag = false;
    }
}
//----------------------------------------------------------MCP 23017--------------------------------------------------------------------------
void   MCP23017_init()
{
    static bool   MCP23017_flag = true;
    if (  MCP23017_flag)
    {
        mcp.begin();
        mcp.pinMode(  MCP23017_GPA0, INPUT);
        mcp.pinMode(  MCP23017_GPA1, INPUT);
        mcp.pinMode(  MCP23017_GPA2, INPUT);
        mcp.pinMode(  MCP23017_GPA3, INPUT);
        mcp.pinMode(  MCP23017_GPA4, INPUT);
        mcp.pinMode(  MCP23017_GPA5, INPUT);
        mcp.pinMode(  MCP23017_GPA6, INPUT);
        mcp.pinMode(  MCP23017_GPA7, INPUT);

        mcp.pinMode(  MCP23017_GPB0, OUTPUT);
        mcp.pinMode(  MCP23017_GPB1, OUTPUT);
        mcp.pinMode(  MCP23017_GPB2, OUTPUT);
        mcp.pinMode(  MCP23017_GPB3, OUTPUT);
        mcp.pinMode(  MCP23017_GPB4, OUTPUT);
        mcp.pinMode(  MCP23017_GPB5, OUTPUT);
        mcp.pinMode(  MCP23017_GPB6, OUTPUT);
        mcp.pinMode(  MCP23017_GPB7, OUTPUT);

          MCP23017_flag = false;
    }
}


