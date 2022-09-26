
#include "Global.h"
#include "classes/IoTItem.h"

#include "PZEMSensor.h"
#include "modules/sensors/SoftUart/SoftUart.h"

PZEMSensor* pzem;

class Pzem004v : public IoTItem {
   private:
    String addr;

   public:
    Pzem004v(String parameters) : IoTItem(parameters) {
        addr = jsonReadStr(parameters, "addr");
        pzem = new PZEMSensor(myUART, hexStringToUint8(addr));
    }

    void doByInterval() {
        value.valD = pzem->values()->voltage;
        regEvent(value.valD, "Pzem Voltage");
    }

    ~Pzem004v(){};
};

void* getAPI_Pzem004(String subtype, String param) {
    if (subtype == F("Pzem004v")) {
        return new Pzem004v(param);
    } else {
        return nullptr;
    }
}
