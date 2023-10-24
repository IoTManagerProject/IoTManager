#include "Global.h"
#include "classes/IoTItem.h"

class IoTMath : public IoTItem {
private:
public:
    IoTMath(String parameters) : IoTItem(parameters) {}

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if(command == "map" & param.size() == 5) {
            IoTValue valTmp;
            valTmp.isDecimal = true;
            valTmp.valD = map(param[0].valD, param[1].valD, param[2].valD, param[3].valD, param[4].valD);
            return valTmp;
        }
        return {};
    }
};

void *getAPI_IoTMath(String subtype, String param) {
    if (subtype == F("IoTMath")) {
        return new IoTMath(param);
    }
    return nullptr;
}
