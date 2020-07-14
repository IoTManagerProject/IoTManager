
#pragma once

#include <Arduino.h>

// Size of the input buffer in bytes (maximum length of one command plus arguments)
#define COMMAND_BUFFER_SIZE 255
// Maximum length of a command excluding the terminating null
#define COMMAND_MAX_LENGHT 15

class StringCommand {
   private:
    struct StringCommandCallback {
        char command[COMMAND_MAX_LENGHT + 1];
        void (*callbk)();

        StringCommandCallback(const char *str, void (*func)()) {
            strncpy(command, str, COMMAND_MAX_LENGHT + 1);
            callbk = func;
        };
    };

    std::vector<StringCommandCallback> _list;

    void (*_defaultHandler)(const char *);
    // null-terminated list of character to be used as delimeters for tokenizing (default " ")
    char _delim[2];
    // Buffer of stored characters while waiting for terminator character
    char _buffer[COMMAND_BUFFER_SIZE + 1];
    // State variable used by strtok_r during processing
    char *_last;

   public:
    StringCommand(const char *delim = " ") : _defaultHandler(NULL), _last(NULL) {
        strcpy(_delim, delim);
        clearBuffer();
    }

    void addCommand(const char *str, void (*func)()) {
        _list.push_back(StringCommandCallback{str, func});
    }

    void setDefaultHandler(void (*func)(const char *)) {
        _defaultHandler = func;
    }

    void clearBuffer() {
        _buffer[0] = '\0';
    }

    void readStr(const String str) {
        str.toCharArray(_buffer, COMMAND_BUFFER_SIZE);
        // Search for command at start of buffer
        char *cmdStr = strtok_r(_buffer, _delim, &_last);
        if (!cmdStr) {
            return;
        }
        bool matched = false;
        for (auto item : _list) {
            if (strncmp(cmdStr, item.command, COMMAND_MAX_LENGHT) == 0) {
                matched = true;
                (*item.callbk)();
                break;
            }
        }
        if (!matched) {
            if (_defaultHandler) {
                (*_defaultHandler)(cmdStr);
            }
            clearBuffer();
        }
    }

    char *next() {
        return strtok_r(NULL, _delim, &_last);
    }
};
