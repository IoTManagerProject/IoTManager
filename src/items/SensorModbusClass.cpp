#include "ItemsCmd.h"
#include "items/SensorModbusClass.h"
//#ifdef SensorModbusEnabled
//=========================================Модуль ультрозвукового дальномера==================================================================
//modbus;id;anydata;Сенсоры;Температура;order;addr[1];regaddr[0];c[1]
//=========================================================================================================================================
SensorModbusClass mySensorModbus;

void modbus() {
    mySensorModbus.update();
    String key = mySensorModbus.gkey();
    sCmd.addCommand(key.c_str(), modbusReading);
    mySensorModbus.SensorModbusInit();
    mySensorModbus.clear();
}
void modbusReading() {
    String key = sCmd.order();
    mySensorModbus.SensorModbusRead(key, 1, 0);
}
//#endif