#include "MySensorsGate.h"

#ifdef MYSENSORS

MyMessage msg(CHILD_ID, V_TEXT);

void receive(const MyMessage &message) {
    String msg = String(message.getSender()) + "-" +  //node-id
                 String(message.getSensor()) + "," +  //child-sensor-id
                 String(message.getFloat()) + ";";    //value

    mysensorBuf += msg;
}
#endif