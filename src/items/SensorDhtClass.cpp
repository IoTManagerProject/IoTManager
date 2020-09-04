#include "items/SensorDhtClass.h"

#include "items/ItemsGlobal.h"
//#ifdef SensorDhtEnabled
//=========================================DHT Sensor==================================================================
//dht-temp;id;anydata;Сенсоры;Температура;order;pin;type[dht11];c[1]
//dht-hum;id;anydata;Сенсоры;Влажность;order;pin;type[dht11];c[1]
//=========================================================================================================================================
SensorDhtClass mySensorDht;
void dhtTemp() {
    mySensorDht.update();
    String key = mySensorDht.gkey();
    sCmd.addCommand(key.c_str(), dhtReadingTemp);
    mySensorDht.SensorDhtInit();
    mySensorDht.clear();
}
void dhtReadingTemp() {
    String key = sCmd.order();
    mySensorDht.SensorDhtReadTemp(key);
}

void dhtHum() {
    mySensorDht.update();
    String key = mySensorDht.gkey();
    sCmd.addCommand(key.c_str(), dhtReadingHum);
    mySensorDht.SensorDhtInit();
    mySensorDht.clear();
}
void dhtReadingHum() {
    String key = sCmd.order();
    mySensorDht.SensorDhtReadHum(key);
}
//#endif