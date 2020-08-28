#pragma once

#include <Arduino.h>

#include "Class/LineParsing.h"
#include "Global.h"

class SensorConverting : public LineParsing {
   public:
    SensorConverting() : LineParsing(){};

    void SensorConvertingInit() {
     

     
    }

    
};
extern SensorConverting* mySensorConverting;