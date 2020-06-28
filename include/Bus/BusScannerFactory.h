#pragma once

#include "Bus/BusScanner.h"
#include "Bus/I2CScanner.h"
#include "Bus/OneWireScanner.h"
#include "Consts.h"
#include "Utils/JsonUtils.h"

class BusScannerFactory {
   public:
    static BusScanner* get(String& config, BusScanner_t type, String& str) {
        switch (type) {
            case BS_I2C:
                return new I2CScanner(str);
            case BS_ONE_WIRE: {
                uint8_t pin = jsonReadInt(config, TAG_ONE_WIRE_PIN);
                return new OneWireScanner(str, pin);
            }
            default:
                return nullptr;
        }
    }
};
