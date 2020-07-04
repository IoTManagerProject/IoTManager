#pragma once

#include <Wire.h>

#include "Objects/BusScanner.h"

class I2CScanner : public BusScanner {
   public:
    I2CScanner();

   protected:
    virtual void init() override;
    virtual boolean syncScan() override;
};