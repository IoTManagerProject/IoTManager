#include "Cmd.h"

void cmd_serialLog() {
    bool enabled = atoi(sCmd.next());
    String out = sCmd.next();

    PrintMessage::setOutput(out.equalsIgnoreCase("serial1") ? &Serial1 : &Serial);
    PrintMessage::enablePrint(enabled);
}
