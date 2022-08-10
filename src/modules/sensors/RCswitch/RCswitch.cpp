#include "Global.h"
#include "classes/IoTItem.h"

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

class RCswitch : public IoTItem {
   private:
    int _pinRx; // Выход радио модуля
    int _pinTx; // Выход модуля передатчика

   public:
    RCswitch(String parameters): IoTItem(parameters) {
        jsonRead(parameters, "pinRx", _pinRx);
        jsonRead(parameters, "pinTx", _pinTx);
        _interval = _interval / 1000;   // корректируем величину интервала int, теперь он в миллисекундах
        if (_pinRx >= 0) {
            Serial.printf("Protocol: %d", _pinRx);
            mySwitch.enableReceive(digitalPinToInterrupt(_pinRx));
        }
        if (_pinTx >= 0)    
            mySwitch.enableTransmit(_pinTx);
    }

    void doByInterval() {
        if (_pinRx >= 0 && mySwitch.available()) {
            // Serial.print("Received ");
            //Serial.print( mySwitch.getReceivedValue() );
            // Serial.print(" / ");                                       
            // Serial.print( mySwitch.getReceivedBitlength() );
            // Serial.print("bit ");
            // Serial.print("Protocol: ");
            // Serial.println( mySwitch.getReceivedProtocol() );
            value.valD = mySwitch.getReceivedValue();
            regEvent(value.valD, "RCswitch");                         
       
            mySwitch.resetAvailable();
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (_pinTx >= 0)
        if (command == "sendBitStr") {  // отправляем строку вида "000000000001010100010001"
            if (param.size()) {
                mySwitch.send(param[0].valS.c_str());
                return {};
            }
        } else if (command == "sendTriState") { // отправляем строку вида "00000FFF0F0F"
            if (param.size()) {
                mySwitch.sendTriState(param[0].valS.c_str());
                return {};
            }
        } else if (command == "sendDecimal") { // отправляем строку вида 5393 первым параметром и количество бит чтоб заполнить нулями
            if (param.size()) {
                mySwitch.send(param[0].valD, param[1].valD);
                return {};
            }
        }  
        return {}; 
    }

    ~RCswitch() {};
};

void* getAPI_RCswitch(String subtype, String param) {
    if (subtype == F("RCswitch")) {
        return new RCswitch(param);
    } else {
        return nullptr;  
    }
}

