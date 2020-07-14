#include "Cmd.h"

/*
* serialLog 1 serial1
*/

void cmd_serialLog() {
    String enabled = sCmd.next();
    String out = sCmd.next();

    PrintMessage::setOutput(out.equalsIgnoreCase("serial1") ? &Serial1 : &Serial);
    PrintMessage::enablePrint(enabled.toInt());
}
