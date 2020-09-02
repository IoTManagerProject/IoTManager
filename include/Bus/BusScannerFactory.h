#pragma once

#include "Bus/BusScanner.h"
#include "Bus/I2CScanner.h"
#include "Consts.h"
#include "Utils/JsonUtils.h"

class BusScannerFactory {
   public:
    static BusScanner* get(String& config, BusScanner_t type, String& str) {
        switch (type) {
            case BS_I2C:
                return new I2CScanner(str);
            default:
                return nullptr;
        }
    }
};
