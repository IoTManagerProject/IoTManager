#include "Global.h"
#include "Classes/IoTItem.h"
#include "classes/IoTGpio.h"
#include <Adafruit_MCP23X17.h>


class Mcp23017 : public IoTItem, IoTGpio {
   private:
   
   public:
    Adafruit_MCP23X17 mcp;

    Mcp23017(String parameters, int index): IoTItem(parameters), IoTGpio(index) {
        
    }
    
    void doByInterval() {
        

        //regEvent(value.valD, "Mcp23017");  
    }

    IoTGpio* getGpioDriver() {
        return this;
    }

    void pinMode(uint8_t pin, uint8_t mode) {
        mcp.pinMode(pin, mode);
    }

    void digitalWrite(uint8_t pin, uint8_t val) {
        mcp.digitalWrite(pin, val);
    }

    int digitalRead(uint8_t pin) {
        return mcp.digitalRead(pin);
    }

    ~Mcp23017() {};
};


void* getAPI_Mcp23017(String subtype, String param) {
    if (subtype == F("Mcp23017")) {
        String addr;
        jsonRead(param, "addr", addr);
        Serial.printf("deviceAddress %s = %02x \n", addr.c_str(), hexStringToUint8(addr));

        String index_str;
        jsonRead(param, "index", index_str);
        int index = index_str.toInt();
        if (index > 4) {
            Serial.println("MCP23X17 wrong index. Must be 0 - 4");
            return nullptr;
        }

        Mcp23017* newItem = new Mcp23017(param, index);
        if (!newItem->mcp.begin_I2C(hexStringToUint8(addr))) {
            Serial.println("MCP23X17 Init Error.");
            delete newItem;
            return nullptr;
        }

        return newItem;
    } else {
        return nullptr;
    }
}
