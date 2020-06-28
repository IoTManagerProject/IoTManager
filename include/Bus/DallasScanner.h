#pragma once

#include "BusScanner.h"

class DallasScanner : public BusScanner {
   public:
    DallasScanner();

   protected:
    virtual boolean syncScan() override;
};
