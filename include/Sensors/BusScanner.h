#pragma once

#include <Arduino.h>

#include "Utils/JsonUtils.h"

class BusScanner {
   public:
    BusScanner(const char* tag) : _intialized{false} {
        strcpy(_tag, tag);
    }
    virtual ~BusScanner(){};

    const String results() {
        return _results;
    }

    const char* tag() {
        return _tag;
    }

    bool scan() {
        _results = "";
        if (!_intialized) {
            bool res = onInit();
            if (!res) {
                _results = "ошибка инициализации";
                return true;
            }
            _intialized = true;
        }
        return onScan();
    }

   protected:
    virtual bool onInit() {
        return true;
    };

    virtual bool onScan() {
        return true;
    };

   protected:
    String _results;

   private:
    bool _intialized;
    char _tag[16];
};
