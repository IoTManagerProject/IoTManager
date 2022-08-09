/******************************************************************
  Used  Energy Monitoring Library

  https://github.com/openenergymonitor/EmonLib

  adapted for version 4 @Serghei63
 ******************************************************************/

#include "Global.h"
#include "classes/IoTItem.h"
#include <map>

extern IoTGpio IoTgpio;

#include "EmonLib.h"                   // Include Emon Library

EnergyMonitor emon1;        // Create an instance 

 
class Energy : public IoTItem {  
  
  private:
    unsigned int _pinI;                           // ток
    unsigned int _calibI;                         // ток

   public:
    Energy(String parameters): IoTItem(parameters) {
        _pinI = jsonReadInt(parameters, "pin_I");
        _calibI  = jsonReadInt(parameters, "calib_I");
        emon1.current(_pinI, _calibI);  // Current: input pin, calibration.   
    }

    void doByInterval() {
      value.valD = emon1.calcIrms(1480);  // Calculate Irms only       
      regEvent(value.valD, "current");
    }

    ~Energy(){};
};


class Power : public IoTItem {  
  
  private:
    unsigned int _pinU;                           // напряжение
    unsigned int _calibU;                         // напряжение

   public:
    Power(String parameters): IoTItem(parameters) {
      _pinU = jsonReadInt(parameters, "pin_U");
      _calibU  = jsonReadInt(parameters, "calib_U");
      emon1.voltage(_pinU, _calibU, 1.7);  // Voltage: input pin, calibration, phase_shift 
    }

    void doByInterval() {
      if (emon1.Irms > 0) emon1.calcVI(10, 1000);         // Calculate all. No.of half wavelengths (crossings), time-out при условии, что ток не нулевой
      value.valD = emon1.Vrms;             //extract Vrms into Variable;        
      regEvent(value.valD, "voltage");
    }

    ~Power(){};
};

void* getAPI_Emon(String subtype, String param) {
  if (subtype == F("I")) {
    return new Energy(param);
  } else if  (subtype == F("U")) {
      return new Power(param); 
  } else {
      return nullptr;
  }
}
