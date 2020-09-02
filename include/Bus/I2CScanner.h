#pragma once

#include "Bus/BusScanner.h"

class I2CScanner : public BusScanner {
   public:
    I2CScanner(String& out);

   protected:
    virtual void init() override;
    virtual boolean syncScan() override;
};