#include "Global.h"
#include "classes/IoTItem.h"

//дочь         -        родитель
class Variable : public IoTItem {
   private:
   public:
    Variable(String parameters) : IoTItem(parameters) {
    }

    void doByInterval() {
    }
};

void* getAPI_Variable(String subtype, String param) {
    if (subtype == F("Variable")) {
        return new Variable(param);
    } else {
        return nullptr;
    }
}
