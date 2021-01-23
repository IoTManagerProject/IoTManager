#include "MySensorsGate.h"

#ifdef MYSENSORS

MyMessage msg(CHILD_ID, V_TEXT);

void receive(const MyMessage &message) {
    String value;
    switch (message.getPayloadType()) {
        case 0:
            value = message.getString();
        case 1:
            value = String(message.getByte());
        case 2:
            value = String(message.getInt());
        case 3:
            value = String(message.getUInt());
        case 4:
            value = String(message.getInt());
        case 5:
            value = String(message.getUInt());
        case 6:
            value = String(message.getBool());
        case 7:
            value = String(message.getFloat());
    }
    String msg = String(message.getPayloadType()) + "," +  //type
                 String(message.getSender()) + "," +       //node-id
                 String(message.getSensor()) + "," +       //child-sensor-id
                 value + ";";                              //value

    mysensorBuf += msg;
}
#endif

//0  Payload type is string
//1  Payload type is byte
//2  Payload type is INT16
//3  Payload type is UINT16
//4  Payload type is INT32
//5  Payload type is UINT32
//6  Payload type is binary
//7  Payload type is float32