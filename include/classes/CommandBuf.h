#pragma once
#include "Global.h"

#define MAX_COMMAND_LENGTH 16
#define BUFFER 128

class CommandBuf;

class CommandBuf {
   public:
    CommandBuf();
    ~CommandBuf();

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

extern CommandBuf* myBuf;
