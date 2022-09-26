
#include "Global.h"
#include "classes/IoTItem.h"

#include "PZEMSensor.h"
//#include "SoftUART.h"

PZEMSensor* pzem;

class Pzem004v : public IoTItem {
   private:
    String addr;

   public:
    Pzem004v(String parameters) : IoTItem(parameters) {
        addr = jsonReadStr(parameters, "addr");
        // pzem = new PZEMSensor(myUART, hexStringToUint8(addr));
    }

    ~Pzem004v(){};
};

void* getAPI_Pzem004(String subtype, String param) {
}
