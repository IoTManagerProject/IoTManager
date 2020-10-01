#pragma once
#include <Arduino.h>
#include "Class/LineParsing.h"
#include "Global.h"

class OutputTextClass : public LineParsing {
   public:
    OutputTextClass() : LineParsing(){};

    void OutputModuleStateSetDefault() {
        if (_state != "") { 
            OutputModuleChange(_key, _state);
        }
    }

    void OutputModuleChange(String key, String state) {
        state.replace("#", " ");
        eventGen(key, "");
        jsonWriteStr(configLiveJson, key, state);
         publishStatus(key, state);
    }
};
extern OutputTextClass myOutputText;