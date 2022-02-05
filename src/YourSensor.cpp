#include "YourSensor.h"

#include "Global.h"
#include "Utils/JsonUtils.h"
#include "Utils/StringUtils.h"

#include "Display/DisplayPlugin.h"

//подключаем необходимые файлы библиотеки
//#include "Adafruit_ADS1X15.h"
#include <Adafruit_AHTX0.h>
#include <BH1750.h>
#include <ClosedCube_HDC1080.h>
#include <LiquidCrystal_I2C.h>
//-------------------------------------------------------MCP 23017-----------------------------------------------------------------------
#include <Wire.h>
#include "Adafruit_MCP23017.h"

#define MCP23017_ADDR 0x20
Adafruit_MCP23017 mcp;
String status;

#define MCP23017_GPA0 0
#define MCP23017_GPA1 1
#define MCP23017_GPA2 2
#define MCP23017_GPA3 3
#define MCP23017_GPA4 4
#define MCP23017_GPA5 5
#define MCP23017_GPA6 6
#define MCP23017_GPA7 7
#define MCP23017_GPB0 8
#define MCP23017_GPB1 9
#define MCP23017_GPB2 10
#define MCP23017_GPB3 11
#define MCP23017_GPB4 12
#define MCP23017_GPB5 13
#define MCP23017_GPB6 14
#define MCP23017_GPB7 15

//-------------------------------------------------------GyverOled 1306-------------------------------------------------------------------
// #include <GyverOLED.h>
///GyverOLED<SSH1106_128x64> oled;
// GyverOLED<SSD1306_128x32, OLED_BUFFER> oled;
// GyverOLED<SSD1306_128x32, OLED_NO_BUFFER> oled;
// GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;
// GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
// GyverOLED<SSD1306_128x64, OLED_BUFFER, OLED_SPI, 8, 7, 6> oled;
//-------------------------------------------------------U8g2 Displey 1106 1306 ST7565 и многие другие--------------------------------------------------------------
#include <Arduino.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
// #ifdef U8X8_HAVE_HW_I2C
// #include <Wire.h>
// #endif
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//-------------------------------------------------------------------------------------------------------------------------------------

//создаем объект HDC1080
ClosedCube_HDC1080 hdc1080;

//создаем объект AHTX0
Adafruit_AHTX0 aht;
Adafruit_Sensor *aht_humidity, *aht_temp;
sensors_event_t tmpEvent_t;

//создаем объект LCD
LiquidCrystal_I2C LCD(0x27, 16, 2);

//создаем объект BH1750
BH1750 lightMeter;

//создаем объект ADS1015
// Adafruit_ADS1015 ads;

//------------------------------------------- co2 sensor S8-----------------------------------------------------------------------------
SoftwareSerial K_30_Serial(13, 15);                          //Программный порт
byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25}; //Команды для запроса показаний с датчика
byte response[] = {0, 0, 0, 0, 0, 0, 0};                     //массив для ответа от датчика
unsigned long getValue(byte packet[])
{
    int high = packet[3];                   //верхний байт показания СО2
    int low = packet[4];                    //нижний байт показания СО2
    unsigned long val_1 = high * 256 + low; //соединяем байты
    return val_1;
}
void sendRequest(byte packet[])
{
    while (!K_30_Serial.available())
    {
        K_30_Serial.write(readCO2, 7);
        delay(50);
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------

float yourSensorReading(String type, String paramsAny)
{
    // SerialPrint("I", "Sensorrrrrrrrrrrrrrr", type); Выводит все type
    // SerialPrint("I", "Sensoreeeeeeeeeeeee ", configLiveJson); //Выводит все сеноры которые используются
    // SerialPrint("I", "Sensoreeeeeeeeeeeee ", configStoreJson); //Выводит все сеноры которые используются и даже кнопки!!!
    float value{0};
    
    //==========================================================SSD 1306=================================================================
    if (type == "SSD1306")
    {
        // SSD_1306_init();
        // oled.setScale(1);
        // oled.setCursorXY(jsonReadInt(paramsAny, "c"), jsonReadInt(paramsAny, "k"));
        // String TeSt = jsonReadStr(paramsAny, "descr") + " " + jsonReadStr(configLiveJson, jsonReadStr(paramsAny, "val"));
        // oled.print(TeSt);
        // oled.update();
        // String BNTN = jsonReadStr(configStoreJson, "btn662");
        // if(BNTN == "1"){
        //    oled.setScale(2);
        //    oled.setCursorXY(10,40);
        //    oled.print(BNTN);
        //    oled.setScale(2);
        //    oled.setCursorXY(35,40);
        //    oled.print("ECCC!!!");}
        //    if(BNTN == "0"){
        //      oled.setScale(2);
        //      oled.setCursorXY(0,40);
        //      oled.print("                       ");
        //    }
        //   SerialPrint("I", "Sensorrr5566774433r", type);
        //   SerialPrint(TeSt,"","Это TeSt");
        //    SerialPrint(BNTN,"","Это BNTN");
        //    SerialPrint("I", "Sensoreeeeeeeeeeeee ", configLiveJson);
    }
    // oled.clear();
    //----------------------------------------------------------------ST 7565---------------------------------------------------------------
    if (type == "ST7565")
    {
        DisplayPlugin::show(configLiveJson, paramsAny); 
    }
    // if (type == "ST7565_1")
    // {
    //     ST7565_init();
    //     u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    //     String TeSt = jsonReadStr(paramsAny, "descr") + " " + jsonReadStr(configLiveJson, jsonReadStr(paramsAny, "val"));
    //     u8g2.setCursor(jsonReadInt(paramsAny, "c"), jsonReadInt(paramsAny, "k"));
    //     u8g2.print(TeSt);
    //     u8g2.sendBuffer();
    // }
    // if (type == "ST7565_2")
    // {
    //     ST7565_init();
    //     u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    //     String TeSt = jsonReadStr(paramsAny, "descr") + " " + jsonReadStr(configLiveJson, jsonReadStr(paramsAny, "val"));
    //     u8g2.setCursor(jsonReadInt(paramsAny, "c"), jsonReadInt(paramsAny, "k"));
    //     u8g2.print(TeSt);
    //     u8g2.sendBuffer();
    // }
    // if (type == "ST7565_3")
    // {
    //     ST7565_init();
    //     u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    //     String TeSt = jsonReadStr(paramsAny, "descr") + " " + jsonReadStr(configLiveJson, jsonReadStr(paramsAny, "val"));
    //     u8g2.setCursor(jsonReadInt(paramsAny, "c"), jsonReadInt(paramsAny, "k"));
    //     u8g2.print(TeSt);
    //     u8g2.sendBuffer();
    // }
    // if (type == "ST7565_4")
    // {
    //     ST7565_init();
    //     u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    //     String TeSt = jsonReadStr(paramsAny, "descr") + " " + jsonReadStr(configLiveJson, jsonReadStr(paramsAny, "val"));
    //     u8g2.setCursor(jsonReadInt(paramsAny, "c"), jsonReadInt(paramsAny, "k"));
    //     u8g2.print(TeSt);
    //     u8g2.sendBuffer();
    // }
    //========================================================HDC1080================================================================
    if (type == "HDC1080_temp")
    {
        HDC1080_init(jsonReadStr(paramsAny, "addr"));
        value = hdc1080.readTemperature();
    }
    if (type == "HDC1080_hum")
    {
        HDC1080_init(jsonReadStr(paramsAny, "addr"));
        value = hdc1080.readHumidity();
    }
    //==========================================================AHTX0=================================================================
    if (type == "AHTX0_temp")
    {
        AHTX0_init();
        aht_temp->getEvent(&tmpEvent_t);
        value = tmpEvent_t.temperature;
    }
    if (type == "AHTX0_hum")
    {
        AHTX0_init();
        aht_humidity->getEvent(&tmpEvent_t);
        value = tmpEvent_t.relative_humidity;
    }
    //==========================================================LCD=================================================================
    if (type == "LCD")
    {
        LCD_init();
        LCD.setCursor(jsonReadInt(paramsAny, "c"), jsonReadInt(paramsAny, "k"));
        String toPrint = jsonReadStr(paramsAny, "descr") + " " + jsonReadStr(configLiveJson, jsonReadStr(paramsAny, "val"));
        LCD.print(toPrint);
    }
    //==========================================================BH1750=================================================================
    if (type == "BH1750_lux")
    {
        BH1750_init();
        value = lightMeter.readLightLevel();
    }
    //==========================================================co2=================================================================
    if (type == "valCO2")
    {
        K_30_Serial.begin(9600);
        sendRequest(readCO2);
        int valCO2 = getValue(response);
        value = valCO2;
        // Serial.println(valCO2);
    }

    //========================================================MCP23x17================================================================
    if (type == "MCP23_0")
    {
        MCP23017_init();
        bool value_D0 = mcp.digitalRead(MCP23017_GPA0);
        value = value_D0;
    }
    if (type == "MCP23_1")
    {
        MCP23017_init();
        bool value_D1 = mcp.digitalRead(MCP23017_GPA1);
        value = value_D1;
    }
    if (type == "MCP23_2")
    {
        MCP23017_init();
        bool value_D2 = mcp.digitalRead(MCP23017_GPA2);
        value = value_D2;
    }
    if (type == "MCP23_3")
    {
        MCP23017_init();
        bool value_D3 = mcp.digitalRead(MCP23017_GPA3);
        value = value_D3;
    }
    if (type == "MCP23_4")
    {
        MCP23017_init();
        bool value_D4 = mcp.digitalRead(MCP23017_GPA4);
        value = value_D4;
    }
    if (type == "MCP23_5")
    {
        MCP23017_init();
        bool value_D5 = mcp.digitalRead(MCP23017_GPA5);
        value = value_D5;
    }
    if (type == "MCP23_6")
    {
        MCP23017_init();
        // orderBuf =+ "btn884"
        bool value_D6 = mcp.digitalRead(MCP23017_GPA6);
        value = value_D6;
    }
    if (type == "MCP23_7")
    {
        MCP23017_init();
        bool value_D7 = mcp.digitalRead(MCP23017_GPA7);
        value = value_D7;
    }
    //-------------------------------------------------------MCP_23017 OUT------------------------------------------------------
    if (type == "MCP23_8")
    {
        String BNTN_1 = jsonReadStr(configStoreJson, "btn1111");
        if (BNTN_1 == "1")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB0, HIGH);
            bool value_D8 = mcp.digitalRead(MCP23017_GPB0);
            value = value_D8;
        }
    }
    if (type == "MCP23_8_1")
    {
        String BNTN_1 = jsonReadStr(configStoreJson, "btn1111");
        if (BNTN_1 == "0")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB0, LOW);
            bool value_D8 = mcp.digitalRead(MCP23017_GPB0);
            value = value_D8;
        }
    }
    if (type == "MCP23_9")
    {
        String BNTN_2 = jsonReadStr(configStoreJson, "btn2222");
        if (BNTN_2 == "1")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB1, HIGH);
            bool value_D9 = mcp.digitalRead(MCP23017_GPB1);
            value = value_D9;
        }
    }
    if (type == "MCP23_9_1")
    {
        String BNTN_2 = jsonReadStr(configStoreJson, "btn2222");
        if (BNTN_2 == "0")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB1, LOW);
            bool value_D9 = mcp.digitalRead(MCP23017_GPB1);
            value = value_D9;
        }
    }

    if (type == "MCP23_10")
    {
        String BNTN_3 = jsonReadStr(configStoreJson, "btn3333");
        if (BNTN_3 == "1")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB2, HIGH);
            bool value_D10 = mcp.digitalRead(MCP23017_GPB2);
            value = value_D10;
        }
    }

    if (type == "MCP23_10_1")
    {
        String BNTN_3 = jsonReadStr(configStoreJson, "btn3333");
        if (BNTN_3 == "0")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB2, LOW);
            bool value_D10 = mcp.digitalRead(MCP23017_GPB2);
            value = value_D10;
        }
    }

    if (type == "MCP23_11")
    {
        String BNTN_4 = jsonReadStr(configStoreJson, "btn4444");
        if (BNTN_4 == "1")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB3, HIGH);
            bool value_D11 = mcp.digitalRead(MCP23017_GPB3);
            value = value_D11;
        }
    }
    if (type == "MCP23_11_1")
    {
        String BNTN_4 = jsonReadStr(configStoreJson, "btn4444");
        if (BNTN_4 == "0")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB3, LOW);
            bool value_D11 = mcp.digitalRead(MCP23017_GPB3);
            value = value_D11;
        }
    }
    if (type == "MCP23_12")
    {
        String BNTN_5 = jsonReadStr(configStoreJson, "btn5555");
        if (BNTN_5 == "1")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB4, HIGH);
            bool value_D12 = mcp.digitalRead(MCP23017_GPB4);
            value = value_D12;
        }
    }
    if (type == "MCP23_12_1")
    {
        String BNTN_5 = jsonReadStr(configStoreJson, "btn5555");
        if (BNTN_5 == "0")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB4, LOW);
            bool value_D12 = mcp.digitalRead(MCP23017_GPB4);
            value = value_D12;
        }
    }
    if (type == "MCP23_13")
    {
        String BNTN_6 = jsonReadStr(configStoreJson, "btn6666");
        if (BNTN_6 == "1")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB5, HIGH);
            bool value_D13 = mcp.digitalRead(MCP23017_GPB5);
            value = value_D13;
        }
    }
    if (type == "MCP23_13_1")
    {
        String BNTN_6 = jsonReadStr(configStoreJson, "btn666");
        if (BNTN_6 == "0")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB5, LOW);
            bool value_D13 = mcp.digitalRead(MCP23017_GPB5);
            value = value_D13;
        }
    }
    if (type == "MCP23_14")
    {
        String BNTN_7 = jsonReadStr(configStoreJson, "btn7777");
        if (BNTN_7 == "1")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB6, HIGH);
            bool value_D14 = mcp.digitalRead(MCP23017_GPB6);
            value = value_D14;
        }
    }
    if (type == "MCP23_14_1")
    {
        String BNTN_7 = jsonReadStr(configStoreJson, "btn7777");
        if (BNTN_7 == "0")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB6, LOW);
            bool value_D14 = mcp.digitalRead(MCP23017_GPB6);
            value = value_D14;
        }
    }
    if (type == "MCP23_15")
    {
        String BNTN_8 = jsonReadStr(configStoreJson, "btn8888");
        if (BNTN_8 == "1")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB7, HIGH);
            bool value_D15 = mcp.digitalRead(MCP23017_GPB7);
            value = value_D15;
        }
    }
    if (type == "MCP23_15_1")
    {
        String BNTN_8 = jsonReadStr(configStoreJson, "btn8888");
        if (BNTN_8 == "0")
        {
            MCP23017_init();
            mcp.digitalWrite(MCP23017_GPB7, LOW);
            bool value_D15 = mcp.digitalRead(MCP23017_GPB7);
            value = value_D15;
        }
    }

    return value;
}

void HDC1080_init(String addr)
{
    static bool HDC1080_flag = true;
    if (HDC1080_flag)
    {
        hdc1080.begin(hexStringToUint8(addr));
        HDC1080_flag = false;
    }
}

void AHTX0_init()
{
    static bool AHTX0_flag = true;
    if (AHTX0_flag)
    {
        if (!aht.begin())
        {
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

void LCD_init()
{
    static bool LCD_flag = true;
    if (LCD_flag)
    {
        LCD.init();      //инициализация дисплея
        LCD.backlight(); //включаем подсветку
    }
    LCD_flag = false;
}

void BH1750_init()
{
    static bool BH1750_flag = true;
    if (BH1750_flag)
    {
        lightMeter.begin();

        BH1750_flag = false;
    }
}
//---------------------------------------------------------------SSD 1306--------------------------------------------------------------------------
void SSD_1306_init()
{
    static bool SSD_1306_flag = true;
    if (SSD_1306_flag)
    {
       // oled.init();
        // oled.invertText(true);    // инвертируй текст!
        // oled.autoPrintln(true);   // автоматически переносить текст
        //  СЕРВИС
        // oled.setContrast(10);   // яркость 0..15
        // oled.setPower(true);    // true/false - включить/выключить дисплей
        // oled.flipH(true);       // true/false - отзеркалить по горизонтали
        // oled.flipV(true);       // true/false - отзеркалить по вертикали
        // oled.isEnd();           // возвращает true, если дисплей "кончился" - при побуквенном выводе
        // oled.clear();
        //  oled.dot(0, 0);     // точка на x,y
        //  oled.dot(0, 1, 1);  // третий аргумент: 0 выкл пиксель, 1 вкл пиксель (по умолч)
        //  oled.line(5, 5, 10, 10);        // линия x0,y0,x1,y1
        //  //oled.line(5, 5, 10, 10, 0);   // пятый аргумент: 0 стереть, 1 нарисовать (по умолч)
        //  oled.fastLineH(0, 5, 10);       // горизонтальная линия (y, x1, x2)
        //  //oled.fastLineH(0, 5, 10, 0);  // четвёртый аргумент: 0 стереть, 1 нарисовать (по умолч)
        //  oled.fastLineV(0, 5, 10);       // аналогично верт. линия (x, y1, y2)
        //  oled.rect(20, 20, 30, 25);      // прямоугольник (x0,y0,x1,y1)
        //  oled.rect(5, 35, 35, 60, OLED_STROKE);      // прямоугольник (x0,y0,x1,y1)
        //  // параметры фигуры:
        //  // OLED_CLEAR - очистить
        //  // OLED_FILL - залить
        //  // OLED_STROKE - нарисовать рамку
        //  oled.roundRect(50, 5, 80, 25, OLED_STROKE);  // аналогично скруглённый прямоугольник
        //  oled.circle(60, 45, 15, OLED_STROKE);        // окружность с центром в (x,y, с радиусом)
        //  oled.circle(60, 45, 5, OLED_FILL);           // четвёртый аргумент: параметр фигуры
        //  битмап
        // oled.drawBitmap(90, 16, bitmap_32x32, 32, 32, BITMAP_NORMAL, BUF_ADD);
        // oled.drawBitmap(90, 16, bitmap_32x32, 32, 32);  // по умолч. нормал и BUF_ADD
        //  x, y, имя, ширина, высота, BITMAP_NORMAL(0)/BITMAP_INVERT(1), BUF_ADD/BUF_SUBTRACT/BUF_REPLACE

        SSD_1306_flag = false;
    }
}
//----------------------------------------------------------MCP 23017--------------------------------------------------------------------------
void MCP23017_init()
{
    static bool MCP23017_flag = true;
    if (MCP23017_flag)
    {
        mcp.begin();
        mcp.pinMode(MCP23017_GPA0, INPUT);
        mcp.pinMode(MCP23017_GPA1, INPUT);
        mcp.pinMode(MCP23017_GPA2, INPUT);
        mcp.pinMode(MCP23017_GPA3, INPUT);
        mcp.pinMode(MCP23017_GPA4, INPUT);
        mcp.pinMode(MCP23017_GPA5, INPUT);
        mcp.pinMode(MCP23017_GPA6, INPUT);
        mcp.pinMode(MCP23017_GPA7, INPUT);

        mcp.pinMode(MCP23017_GPB0, OUTPUT);
        mcp.pinMode(MCP23017_GPB1, OUTPUT);
        mcp.pinMode(MCP23017_GPB2, OUTPUT);
        mcp.pinMode(MCP23017_GPB3, OUTPUT);
        mcp.pinMode(MCP23017_GPB4, OUTPUT);
        mcp.pinMode(MCP23017_GPB5, OUTPUT);
        mcp.pinMode(MCP23017_GPB6, OUTPUT);
        mcp.pinMode(MCP23017_GPB7, OUTPUT);

        MCP23017_flag = false;
    }
}
