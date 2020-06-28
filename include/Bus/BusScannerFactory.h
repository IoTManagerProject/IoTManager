#pragma once

#include "Bus/BusScanner.h"
#include "Bus/I2CScanner.h"
#include "Bus/DallasScanner.h"
#include "Consts.h"
#include "Utils/JsonUtils.h"

class BusScannerFactory {
   public:
    static BusScanner* get(String& config, BusScanner_t type, String& str) {
        switch (type) {
            case BS_I2C:
                return new I2CScanner(str);
            case BS_ONE_WIRE: {
                return new DallasScanner(str);
            }
            default:
                return nullptr;
        }
    }
};
