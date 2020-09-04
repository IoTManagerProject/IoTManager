#include "items/ItemsGlobal.h"
#include "items/SensorDhtClass.h"
//#ifdef SensorDhtEnabled
//=========================================Модуль ультрозвукового дальномера==================================================================
//dht-temp;id;anydata;Сенсоры;Температура;order;pin;c[1]
//=========================================================================================================================================
SensorDhtClass mySensorDht;
void DhtTemp() {
    mySensorDht.update();
    String key = mySensorDht.gkey();
    sCmd.addCommand(key.c_str(), dhtReading);
    mySensorDht.SensorDhtInit();
    mySensorDht.clear();
}

void dhtReading() {
    String key = sCmd.order();
    mySensorDht.SensorDhtRead(key);
}
//#endif