#include "Consts.h"
#ifdef SensorBme280Enabled
#include "items/SensorBme280Class.h"
#include "BufferExecute.h"
//=========================================Модуль ультрозвукового дальномера==================================================================
//bme280-temp;id;anydata;Сенсоры;Температура;order;c[1]
//bme280-hum;id;anydata;Сенсоры;Температура;order;c[1]
//bme280-press;id;anydata;Сенсоры;Температура;order;c[1]
//=========================================================================================================================================
SensorBme280Class mySensorBme280;

void bme280Temp() {
    mySensorBme280.update();
    String key = mySensorBme280.gkey();
    sCmd.addCommand(key.c_str(), bme280ReadingTemp);
    mySensorBme280.SensorBme280Init();
    mySensorBme280.clear();
}
void bme280ReadingTemp() {
    String key = sCmd.order();
    mySensorBme280.SensorBme280ReadTmp(key);
}

void bme280Hum() {
    mySensorBme280.update();
    String key = mySensorBme280.gkey();
    sCmd.addCommand(key.c_str(), bme280ReadingHum);
    mySensorBme280.SensorBme280Init();
    mySensorBme280.clear();
}
void bme280ReadingHum() {
    String key = sCmd.order();
    mySensorBme280.SensorBme280ReadHum(key);
}

void bme280Press() {
    mySensorBme280.update();
    String key = mySensorBme280.gkey();
    sCmd.addCommand(key.c_str(), bme280ReadingPress);
    mySensorBme280.SensorBme280Init();
    mySensorBme280.clear();
}
void bme280ReadingPress() {
    String key = sCmd.order();
    mySensorBme280.SensorBme280ReadPress(key);
}
#endif