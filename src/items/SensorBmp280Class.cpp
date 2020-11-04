#include "items/SensorBmp280Class.h"

#include "BufferExecute.h"
//#ifdef SensorBmp280Enabled
//=========================================Модуль ультрозвукового дальномера==================================================================
//bmp280-temp;id;anydata;Сенсоры;Температура;order;c[1]
//bmp280-hum;id;anydata;Сенсоры;Температура;order;c[1]
//bmp280-press;id;anydata;Сенсоры;Температура;order;c[1]
//=========================================================================================================================================
SensorBmp280Class mySensorBmp280;

void bmp280Temp() {
    mySensorBmp280.update();
    String key = mySensorBmp280.gkey();
    sCmd.addCommand(key.c_str(), bmp280ReadingTemp);
    mySensorBmp280.SensorBmp280Init();
    mySensorBmp280.clear();
}
void bmp280ReadingTemp() {
    String key = sCmd.order();
    mySensorBmp280.SensorBmp280ReadTmp(key);
}

void bmp280Press() {
    mySensorBmp280.update();
    String key = mySensorBmp280.gkey();
    sCmd.addCommand(key.c_str(), bmp280ReadingPress);
    mySensorBmp280.SensorBmp280Init();
    mySensorBmp280.clear();
}
void bmp280ReadingPress() {
    String key = sCmd.order();
    mySensorBmp280.SensorBmp280ReadPress(key);
}
//#endif