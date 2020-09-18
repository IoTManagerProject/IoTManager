#include "items/SensorModbusClass.h"

#include "ItemsCmd.h"
//#ifdef SensorModbusEnabled
//=========================================Модуль modbus===================================================================================
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
    String addr = sCmd.next();
    String regaddr = sCmd.next();
    mySensorModbus.SensorModbusRead(key, addr.toInt(), regaddr.toInt());
}
//#endif