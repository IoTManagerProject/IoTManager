#include "Global.h"
#include "classes/IoTItem.h"
#include "classes/IoTGpio.h"
#include <Adafruit_PCF8574.h>

/* Example for 8 input buttons that are connected from the GPIO expander pins to ground.
 * Note the buttons must be connected with the other side of the switch to GROUND. There is
 * a built in pull-up 'resistor' on each input, but no pull-down resistor capability.
 */

void scanI2C();

class Pcf8574Driver : public IoTGpio {
   private:
    Adafruit_PCF8574 _pcf;

   public:
    Pcf8574Driver(int index, String addr) : IoTGpio(index) {
        if (!_pcf.begin(hexStringToUint8(addr))) {
            Serial.println("PCF8574 Init Error.");
        }
    }

    void pinMode(uint8_t pin, uint8_t mode) {
        _pcf.pinMode(pin, mode);
    }

    void digitalWrite(uint8_t pin, uint8_t val) {
        _pcf.digitalWrite(pin, val);
    }

    int digitalRead(uint8_t pin) {
        return _pcf.digitalRead(pin);
    }

    void digitalInvert(uint8_t pin) {
        _pcf.digitalWrite(pin, 1 - _pcf.digitalRead(pin));
    }

    ~Pcf8574Driver() {};
};


class Pcf8574 : public IoTItem {
   private:
    Pcf8574Driver* _driver;

   public:
    Pcf8574(String parameters) : IoTItem(parameters) {
        _driver = nullptr;
        
        String addr;
        jsonRead(parameters, "addr", addr);
        if (addr == "") {
            scanI2C();
            return;
        }

        int index;
        jsonRead(parameters, "index", index);
        if (index > 4) {
            Serial.println("Pcf8574 wrong index. Must be 0 - 4");
            return;
        }
        
        _driver = new Pcf8574Driver(index, addr);        
    }

    void doByInterval() {}

    //возвращает ссылку на экземпляр класса Pcf8574Driver
    IoTGpio* getGpioDriver() {
        return _driver;
    }

    ~Pcf8574() {
        delete _driver;
    };
};

void* getAPI_Pcf8574(String subtype, String param) {
    if (subtype == F("Pcf8574")) {
        return new Pcf8574(param);
    } else {
        return nullptr;
    }
}
