#include "Global.h"
#include "classes/IoTItem.h"


class VButton : public IoTItem {
   public:
    VButton(String parameters): IoTItem(parameters) { }

    void setValue(const IoTValue& Value, bool generateEvent = true) {
        value = Value;
        if (generateEvent) regEvent((String)(int)value.valD, "VButton");
    }

    String getValue() {
        return (String)(int)value.valD;
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
