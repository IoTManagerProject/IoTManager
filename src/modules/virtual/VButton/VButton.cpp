#include "Global.h"
#include "classes/IoTItem.h"


class VButton : public IoTItem {
   public:
    VButton(String parameters): IoTItem(parameters) { }

    void setValue(IoTValue Value) {
        value = Value;
        regEvent((String)(int)value.valD, "VButton");
    }

    void doByInterval() { }
};

void* getAPI_VButton(String subtype, String param) {
    if (subtype == F("VButton")) {
        return new VButton(param);
    } else {
        return nullptr;
    }
}
