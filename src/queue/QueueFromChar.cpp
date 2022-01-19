#ifdef QUEUE_FROM_CHAR
#include "classes/QueueFromChar.h"

QueueFromChar::QueueFromChar() {
    commandList = NULL;
    commandCount = 0;
}
QueueFromChar::~QueueFromChar() {}

//добавление команды в буфер
void QueueFromChar::addCommand(const char* command) {
    commandList = (CharBufferStruct*)realloc(commandList, (commandCount + 1) * sizeof(CharBufferStruct));
    strncpy(commandList[commandCount].command, command, MAX_COMMAND_LENGTH);
    Serial.println("command added: " + String(command) + " " + String(commandCount));
    commandCount++;
}

//распечатаем все добавленные команды
void QueueFromChar::printCommands() {
    if (commandCount > 0 && commandList != NULL) {
        for (int i = 0; i < commandCount; i++) {
            Serial.println(commandList[i].command);
        }
    }
}

//заберем последнюю из положенных в буфер команд
String QueueFromChar::getLastCommand() {
    String ret = "empty";
    if (commandList != NULL) {
        int cnt = commandCount - 1;
        ret = commandList[cnt].command;
        if (cnt > 0) {
            delete commandList[cnt].command;
        } else if (cnt == 0) {
            commandList = NULL;
        }
        Serial.println("command deleted: " + ret + " " + String(cnt));
        commandCount--;
    }
    return ret;
}

// QueueFromChar* myBuf;

#endif
