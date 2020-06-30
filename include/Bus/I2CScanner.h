#pragma once

#include "Bus/BusScanner.h"

#include <Wire.h>
class I2CScanner : public BusScanner {
   public:
    I2CScanner();

   protected:
    virtual void init() override;
    virtual boolean syncScan() override;
};