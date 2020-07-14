#pragma once

#include "BusScanner.h"

class OneWireScanner : public BusScanner {
   public:
    OneWireScanner();
    ~OneWireScanner();

   protected:
    bool onInit() override;
    boolean onScan() override;

   private:
    uint8_t _addr[8];
};
