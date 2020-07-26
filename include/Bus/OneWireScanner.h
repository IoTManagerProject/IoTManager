#pragma once

#include "BusScanner.h"
#include "OneWireBus.h"

class OneWireScanner : public BusScanner {
   public:
    OneWireScanner(String& out, uint8_t pin);

   protected:
    virtual boolean syncScan() override;

   private:
    OneWire* _bus;
};
