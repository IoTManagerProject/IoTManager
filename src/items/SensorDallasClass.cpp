#include "items/ItemsGlobal.h"
#include "items/SensorDallasClass.h"
//#ifdef SensorDallasEnabled
//=========================================Модуль ультрозвукового дальномера==================================================================
//dallas-temp;id;anydata;Сенсоры;Температура;order;pin;c[1]
//=========================================================================================================================================
SensorDallasClass mySensorDallas;
void dallasTemp() {
    mySensorDallas.update();
    String key = mySensorDallas.gkey();
    sCmd.addCommand(key.c_str(), dallasReading);
    mySensorDallas.SensorDallasInit();
    mySensorDallas.clear();
}

void dallasReading() {
    String key = sCmd.order();
    mySensorDallas.SensorDallasRead(key);
}
//#endif