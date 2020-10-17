#include "ItemsCmd.h"
#include "items/SensorAnalogClass.h"
#ifdef ANALOG_ENABLED
//==============================================Модуль аналогового сенсора===========================================================================================
//analog-adc;id;anydata;Сенсоры;Аналоговый;order;pin-adc;map[1,1024,1,100];c[1]
//===================================================================================================================================================================
SensorAnalogClass mySensorAnalog;
void analogAdc() {
    mySensorAnalog.update();
    String key = mySensorAnalog.gkey();
    sCmd.addCommand(key.c_str(), analogReading);
    sensorReadingMap += key + ",";
    mySensorAnalog.SensorAnalogInit();
    mySensorAnalog.clear();
}

void analogReading() {
    String key = sCmd.order();
    String pin = jsonReadStr(configOptionJson, key + "_pin");
    mySensorAnalog.SensorAnalogRead(key, pin);
}
#endif