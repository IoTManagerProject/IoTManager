#include "Global.h"
#include "classes/IoTItem.h"
#include "Arduino.h"
#include "MySensorsData.h"

#ifdef MYSENSORS

void receive(const MyMessage& message) {
    String inMsg = String(message.getSender()) + "," +   // node-id
                   String(message.getSensor()) + "," +   // child-sensor-id
                   String(message.getType()) + "," +     // type of var
                   String(message.getCommand()) + "," +  // command
                   parseToString(message) + ";";         // value

    SerialPrint("i", F("MySensorsGate"), inMsg);

    mysensorBuf += inMsg;
}

String parseToString(const MyMessage& message) {
    String value = "error";
    switch (message.getPayloadType()) {
        case 0:  // Payload type is string
            value = message.getString();
            return value;
        case 1:  // Payload type is byte
            value = String(message.getByte());
            return value;
        case 2:  // Payload type is INT16
            value = String(message.getInt());
            return value;
        case 3:  // Payload type is UINT16
            value = String(message.getUInt());
            return value;
        case 4:  // Payload type is INT32
            value = String(message.getInt());
            return value;
        case 5:  // Payload type is UINT32
            value = String(message.getUInt());
            return value;
        case 6:  // Payload type is binary
            value = String(message.getBool());
            return value;
        case 7:  // Payload type is float32
            value = String(message.getFloat());
            return value;
        default:
            return value;
    }
}

#endif