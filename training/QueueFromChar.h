#pragma once
#include "Global.h"
#ifdef QUEUE_FROM_CHAR

#define MAX_COMMAND_LENGTH 16
#define BUFFER 128

class QueueFromChar;

class QueueFromChar {
   public:
    QueueFromChar();
    ~QueueFromChar();

    void addCommand(const char* command);

    void printCommands();

    String getLastCommand();

   private:
    struct CharBufferStruct {
        char command[MAX_COMMAND_LENGTH + 1];
    };
    CharBufferStruct* commandList;
    int commandCount = 0;
};

// extern QueueFromChar* myBuf;

//========проверка очереди=====================
// myBuf = new QueueFromChar;
// myBuf->addCommand("zero");
// myBuf->addCommand("one");
// myBuf->addCommand("two");
// myBuf->printCommands();
// myBuf->getLastCommand();
// myBuf->getLastCommand();
// myBuf->getLastCommand();
// myBuf->printCommands();

#endif
