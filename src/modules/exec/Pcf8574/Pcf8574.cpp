#include "Global.h"
#include "classes/IoTItem.h"
#include "classes/IoTGpio.h"

#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>

#define PCF8574_I2CADDR_DEFAULT 0x20 ///< DS3502 default I2C address

class Adafruit_PCF8574_mod {
   public:
    Adafruit_PCF8574_mod() {};
    bool begin(uint8_t i2c_address = PCF8574_I2CADDR_DEFAULT, TwoWire *wire = &Wire) {
        i2c_dev = new Adafruit_I2CDevice(i2c_address, wire);

        if (!i2c_dev->begin()) {
            return false;
        }

        return true;
    }

    bool digitalWriteByte(uint8_t d) {
        _writebuf = d;
        return i2c_dev->write(&_writebuf, 1);
    }

    uint8_t digitalReadByte(void) {
        i2c_dev->read(&_readbuf, 1);
        return _readbuf;
    }

    bool digitalWrite(int pinnum, bool val) {
        if (val) {
            _writebuf |= 1 << pinnum;
        } else {
            _writebuf &= ~(1 << pinnum);
        }
        return i2c_dev->write(&_writebuf, 1);         
    }

    bool pinMode(int pinnum, uint8_t val) {
        if ((val == INPUT) || (val == INPUT_PULLUP)) {
            _writebuf |= 1 << pinnum;
        } else {
            _writebuf &= ~(1 << pinnum);
        }
        return i2c_dev->write(&_writebuf, 1);
    }

    bool digitalRead(int pinnum) {
        i2c_dev->read(&_readbuf, 1);
        return (_readbuf >> pinnum) & 0x1;
    }

   private:
    uint8_t _readbuf = 0, _writebuf = 0;

    Adafruit_I2CDevice *i2c_dev;
};

class Pcf8574Driver : public IoTGpio {
   private:
    Adafruit_PCF8574_mod _pcf;

   public:
    Pcf8574Driver(int index, String addr) : IoTGpio(index) {
        if (!_pcf.begin(hexStringToUint8(addr), &Wire)) {
            Serial.println("PCF8574 Init Error.");
        }
    }

    void pinMode(int pin, uint8_t mode) {
        _pcf.pinMode(pin, mode);
    }

    void digitalWrite(int pin, uint8_t val) {
        _pcf.digitalWrite(pin, val);
    }

    int digitalRead(int pin) {
        return _pcf.digitalRead(pin);
    }

    void digitalInvert(int pin) {
        _pcf.digitalWrite(pin, 1 - _pcf.digitalRead(pin));
    }

    ~Pcf8574Driver() {};
};


class Pcf8574 : public IoTItem {
   private:
    Pcf8574Driver* _driver;
    String _addr;

   public:
    Pcf8574(String parameters) : IoTItem(parameters) {
        _driver = nullptr;
        
        jsonRead(parameters, "addr", _addr);
        if (_addr == "") {
            scanI2C();
            return;
        }

        int index;
        jsonRead(parameters, "index", index);
        if (index > 4) {
            Serial.println("Pcf8574 wrong index. Must be 0 - 4");
            return;
        }
        
        _driver = new Pcf8574Driver(index, _addr);        
    }

    void doByInterval() {
        if (_addr == "") {
            scanI2C();
            return;
        }
    }

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
