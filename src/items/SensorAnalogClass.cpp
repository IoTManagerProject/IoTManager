#include "items/itemsGlobal.h"
#include "items/SensorAnalogClass.h"
#ifdef ANALOG_ENABLED
//==============================================Модуль аналогового сенсора===========================================================================================
//analog-adc;id;anydata;Сенсоры;Аналоговый;order;pin-adc;map[1,1024,1,100];c[1]
//===================================================================================================================================================================
SensorAnalogClass mySensorAnalog;
void analogAdc() {
    mySensorAnalog.update();
    String key = mySensorAnalog.gkey();
    String pin = mySensorAnalog.gpin();
    sCmd.addCommand(key.c_str(), analogReading);
    sensorReadingMap += key + ",";
    jsonWriteStr(configOptionJson, key + "_pin", pin);
    jsonWriteStr(configOptionJson, key + "_map", mySensorAnalog.gmap());
    jsonWriteStr(configOptionJson, key + "_с", mySensorAnalog.gc());
    mySensorAnalog.clear();
}

void analogReading() {
    String key = sCmd.order();
    String pin = jsonReadStr(configOptionJson, key + "_pin");
    mySensorAnalog.SensorAnalogRead(key, pin);
}
#endif