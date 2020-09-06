//#include "ItemsCmd.h"
//#include "items/SensorBme280Class.h"
////#ifdef SensorBme280Enabled
////=========================================Модуль ультрозвукового дальномера==================================================================
////Bme280-temp;id;anydata;Сенсоры;Температура;order;pin;c[1]
////=========================================================================================================================================
//SensorBme280Class mySensorBme280;
//void bme280Temp() {
//    mySensorBme280.update();
//    String key = mySensorBme280.gkey();
//    sCmd.addCommand(key.c_str(), bme280Reading);
//    mySensorBme280.SensorBme280Init();
//    mySensorBme280.clear();
//}
//
//void bme280Reading() {
//    String key = sCmd.order();
//    mySensorBme280.SensorBme280Read(key);
//}
////#endif