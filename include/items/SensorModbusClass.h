#pragma once
#include <Arduino.h>
#include <ModbusMaster.h>
#include <SoftwareSerial.h>

#include "Class/LineParsing.h"
#include "Global.h"
#include "items/SensorConvertingClass.h"

ModbusMaster modbus1;
SoftwareSerial uart(13, 12);  // RX, TX

class SensorModbusClass : public SensorConvertingClass {
   public:
    SensorModbusClass() : SensorConvertingClass(){};

    void SensorModbusInit() {
        uart.begin(9600);
        jsonWriteStr(configOptionJson, _key + "_map", _map);
        jsonWriteStr(configOptionJson, _key + "_с", _c);
        sensorReadingMap += _key + " " + _addr + " " + _reg + ","; 
        Serial.println(sensorReadingMap);
    }

    void SensorModbusRead(String key, uint8_t slaveAddress, uint16_t regAddress) {
        int value;

        modbus1.begin(slaveAddress, uart);
        uint16_t reqisterValue = modbus1.readHoldingRegisters(regAddress, 1);
        if (getResultMsg(&modbus1, reqisterValue)) {
            reqisterValue = modbus1.getResponseBuffer(0);
            value = reqisterValue;
        } else {
           value = NULL;
        }

        int valueFl = this->correction(key, value);
        eventGen(key, "");
        jsonWriteStr(configLiveJson, key, String(valueFl));
        MqttClient::publishStatus(key, String(valueFl));
        Serial.println("I sensor '" + key + "' data: " + String(valueFl) + ", Slave dev addr: " + String(slaveAddress) + ", Register: " + String(regAddress));
    }

    bool getResultMsg(ModbusMaster* modbus1, uint16_t result) {
        String tmpstr;
        switch (result) {
            case modbus1->ku8MBSuccess:
                return true;
                tmpstr += "Ok";
                break;
            case modbus1->ku8MBIllegalFunction:
                tmpstr += "Illegal Function";
                break;
            case modbus1->ku8MBIllegalDataAddress:
                tmpstr += "Illegal Data Address";
                break;
            case modbus1->ku8MBIllegalDataValue:
                tmpstr += "Illegal Data Value";
                break;
            case modbus1->ku8MBSlaveDeviceFailure:
                tmpstr += "Slave Device Failure";
                break;
            case modbus1->ku8MBInvalidSlaveID:
                tmpstr += "Invalid Slave ID";
                break;
            case modbus1->ku8MBInvalidFunction:
                tmpstr += "Invalid Function";
                break;
            case modbus1->ku8MBResponseTimedOut:
                tmpstr += "Response Timed Out";
                break;
            case modbus1->ku8MBInvalidCRC:
                tmpstr += "Invalid CRC";
                break;
            default:
                tmpstr += "Unknown error: " + String(result);
                break;
        }
        SerialPrint("I", "Modbus", tmpstr);
        return false;
    }
};
extern SensorModbusClass mySensorModbus;