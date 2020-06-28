#pragma once

#include "BusScanner.h"

class DallasScanner : public BusScanner {
   public:
    DallasScanner(String& out);

   protected:
    virtual boolean syncScan() override;
};
