#include "YourSensor.h"

#include "Global.h"
#include "Utils/JsonUtils.h"
#include "Utils/StringUtils.h"

//подключаем необходимые файлы библиотеки
//#include "Adafruit_ADS1X15.h"
#include "Adafruit_AHTX0.h"
#include "BH1750.h"
#include "ClosedCube_HDC1080.h"
#include "LiquidCrystal_I2C.h"

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

// co2 sensor
SoftwareSerial K_30_Serial(13, 15);                           //Программный порт
byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Команды для запроса показаний с датчика
byte response[] = {0, 0, 0, 0, 0, 0, 0};                      //массив для ответа от датчика
unsigned long getValue(byte packet[]) {
    int high = packet[3];                    //верхний байт показания СО2
    int low = packet[4];                     //нижний байт показания СО2
    unsigned long val_1 = high * 256 + low;  //соединяем байты
    return val_1;
}
void sendRequest(byte packet[]) {
    while (!K_30_Serial.available()) {
        K_30_Serial.write(readCO2, 7);
        delay(50);
    }
}

float yourSensorReading(String type, String paramsAny) {
    float value;
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
    //==========================================================LCD=================================================================
    if (type == "LCD") {
        LCD_init();
        LCD.setCursor(jsonReadInt(paramsAny, "c"), jsonReadInt(paramsAny, "k"));
        String toPrint = jsonReadStr(paramsAny, "descr") + " " + jsonReadStr(configLiveJson, jsonReadStr(paramsAny, "val"));
        LCD.print(toPrint);
    }
    //==========================================================BH1750=================================================================
    if (type == "BH1750_lux") {
        BH1750_init();
        value = lightMeter.readLightLevel();
    }
    //==========================================================BH1750=================================================================
    if (type == "valCO2") {
        K_30_Serial.begin(9600);
        sendRequest(readCO2);
        int valCO2 = getValue(response);
        value = valCO2;
        // Serial.println(valCO2);
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

void LCD_init() {
    static bool LCD_flag = true;
    if (LCD_flag) {
        LCD.init();       //инициализация дисплея
        LCD.backlight();  //включаем подсветку
    }
    LCD_flag = false;
}

void BH1750_init() {
    static bool BH1750_flag = true;
    if (BH1750_flag) {
        lightMeter.begin();

        BH1750_flag = false;
    }
}