#include "Global.h"
#include "classes/IoTItem.h"
#include "classes/IoTGpio.h"
#include <Adafruit_MCP23X08.h>

class Mcp23008Driver : public IoTGpio {
   private:
    Adafruit_MCP23X08 _mcp;

   public:
    Mcp23008Driver(int index, String addr) : IoTGpio(index) {
        if (!_mcp.begin_I2C(hexStringToUint8(addr))) {
            Serial.println("MCP23X08 Init Error.");
        }
    }

    void pinMode(int pin, uint8_t mode) {
        _mcp.pinMode(pin, mode);
    }

    void digitalWrite(int pin, uint8_t val) {
        _mcp.digitalWrite(pin, val);
    }

    int digitalRead(int pin) {
        return _mcp.digitalRead(pin);
    }

    void digitalInvert(int pin) {
        _mcp.digitalWrite(pin, 1 - _mcp.digitalRead(pin));
    }

    ~Mcp23008Driver() {};
};


class Mcp23008 : public IoTItem {
   private:
    Mcp23008Driver* _driver;
    String _addr;

   public:
    Mcp23008(String parameters) : IoTItem(parameters) {
        _driver = nullptr;
        
        jsonRead(parameters, "addr", _addr);
        if (_addr == "") {
            scanI2C();
            return;
        }

        int index;
        jsonRead(parameters, "index", index);
        if (index > 4) {
            Serial.println("MCP23X08 wrong index. Must be 0 - 4");
            return;
        }
        
        _driver = new Mcp23008Driver(index, _addr);        
    }

    void doByInterval() {
        if (_addr == "") {
            scanI2C();
            return;
        }
    }

    //возвращает ссылку на экземпляр класса Mcp23008Driver
    IoTGpio* getGpioDriver() {
        return _driver;
    }

    ~Mcp23008() {
        delete _driver;
    };
};

void* getAPI_Mcp23008(String subtype, String param) {
    if (subtype == F("Mcp23008")) {
        return new Mcp23008(param);
    } else {
        return nullptr;
    }
}
