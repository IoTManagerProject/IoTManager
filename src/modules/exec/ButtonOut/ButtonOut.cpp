#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;


class ButtonOut : public IoTItem {
   private:
    int _pin, _inv;

   public:
    ButtonOut(String parameters): IoTItem(parameters) {
        jsonRead(parameters, "pin", _pin);
        jsonRead(parameters, "inv", _inv);

        IoTgpio.pinMode(_pin, OUTPUT);
        IoTgpio.digitalWrite(_pin, value.valD?HIGH:LOW);
    }

    void doByInterval() {
        //value.valD = IoTgpio.analogRead(_pin);

        //regEvent(value.valD, "ButtonOut");  //обязательный вызов хотяб один
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        // реакция на вызов команды модуля из сценария
        // String command - имя команды после ID. (ID.Команда())
        // param - вектор ("массив") значений параметров переданных вместе с командой: ID.Команда("пар1", 22, 33) -> param[0].ValS = "пар1", param[1].ValD = 22
    
        if (command == "change") { 
            value.valD = 1 - IoTgpio.digitalRead(_pin);
            IoTgpio.digitalWrite(_pin, value.valD);
            regEvent(value.valD, "ButtonOut");
        }

        return {};  // команда поддерживает возвращаемое значения. Т.е. по итогу выполнения команды или общения с внешней системой, можно вернуть значение в сценарий для дальнейшей обработки
    }

    void setValue(IoTValue Value, bool generateEvent = true) {
        value = Value;
        IoTgpio.digitalWrite(_pin, _inv?!value.valD:value.valD);
        regEvent((String)(int)value.valD, "ButtonOut");
    }

    String getValue() {
        return (String)(int)value.valD;
    }
    //=======================================================================================================

    ~ButtonOut() {};
};

void* getAPI_ButtonOut(String subtype, String param) {
    if (subtype == F("ButtonOut")) {
        return new ButtonOut(param);
    } else {
        return nullptr;
    }
}
