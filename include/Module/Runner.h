#pragma once

#include <Print.h>

class Runner {
   public:
    virtual void run(const char*, Print*);
};

class CmdRunner : public Runner {
   public:
    void run(const char* cmd, Print* out) override {
        String cmdStr{cmd};
        stringExecute(cmdStr);
    }
};