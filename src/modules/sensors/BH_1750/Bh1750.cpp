#include "Global.h"
#include "classes/IoTItem.h"

#include "BH1750.h"


class Bh1750 : public IoTItem {       
    BH1750 _lightMeter;
      
  public:
    Bh1750(String parameters): IoTItem(parameters){
        _lightMeter.begin();
    }
      
    void doByInterval() {
        value.valD = _lightMeter.readLightLevel();
        if (value.valD < 70000)
            regEvent(value.valD, "BH1750");
        else
            SerialPrint("E", "Sensor BH1750", "Error");  
    }

    ~Bh1750() {};   
};

void* getAPI_Bh1750(String subtype, String param) {
    if (subtype == F("Bh1750")) {
        return new Bh1750(param);
    } else {
        return nullptr;
    }
}