#include "ItemsCmd.h"
#include "items/SensorAnalogClass.h"
#ifdef ANALOG_ENABLED
//==============================================Модуль аналогового сенсора===========================================================================================
//===================================================================================================================================================================
SensorAnalogClass mySensorAnalog;
void analogAdc() {
    mySensorAnalog.update();
    String key = mySensorAnalog.gkey();
    sCmd.addCommand(key.c_str(), analogReading);
    sensorReadingMap10sec += key + ",";
    mySensorAnalog.SensorAnalogInit();
    mySensorAnalog.clear();
}

void analogReading() {
    String key = sCmd.order();
    String pin = jsonReadStr(configOptionJson, key + "_pin");
    mySensorAnalog.SensorAnalogRead(key, pin);
}
#endif