#include "Global.h"
#include "classes/IoTItem.h"
#include "classes/IoTGpio.h"
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>

#define PCF8574_I2CADDR_DEFAULT 0x20 ///< DS3502 стандартный I2C адрес

class Adafruit_PCF8574_mod {
   public:
    Adafruit_PCF8574_mod() : _pinConfig(0xFF) {}; // По умолчанию все пины настроены как входы

    bool begin(uint8_t i2c_address = PCF8574_I2CADDR_DEFAULT, TwoWire *wire = &Wire) {
        i2c_dev = new Adafruit_I2CDevice(i2c_address, wire);
        return i2c_dev->begin();
    }

    bool digitalWriteByte(uint8_t d) {
        _writebuf = d;
        return updateRegister();
    }

    uint8_t digitalReadByte(void) {
        i2c_dev->read(&_readbuf, 1);
        return _readbuf;
    }

    bool digitalWrite(int pinnum, bool val) {
        if (val) {
            _writebuf |= (1 << pinnum);
        } else {
            _writebuf &= ~(1 << pinnum);
        }
        return updateRegister(); // Обновляем регистр после изменения состояния пина
    }

    bool pinMode(int pinnum, uint8_t val) {
        if ((val == INPUT) || (val == INPUT_PULLUP)) {
            _pinConfig |= (1 << pinnum);
        } else {
            _pinConfig &= ~(1 << pinnum);
        }
        return updateRegister(); // Обновляем регистр после изменения конфигурации пина
    }

    bool digitalRead(int pinnum) {
        i2c_dev->read(&_readbuf, 1);
        return (_readbuf >> pinnum) & 0x1;
    }

   private:
    uint8_t _readbuf = 0, _writebuf = 0;
    uint8_t _pinConfig;  // Конфигурация пинов (вход/выход)

    bool updateRegister() {
        uint8_t outputValue = (_writebuf & ~_pinConfig) | (_pinConfig);
        return i2c_dev->write(&outputValue, 1); // Отправляем обновленное значение регистра на устройство
    }

    Adafruit_I2CDevice *i2c_dev;
};

class Pcf8574Driver : public IoTGpio {
   private:
    Adafruit_PCF8574_mod _pcf;

   public:
    Pcf8574Driver(int index, String addr) : IoTGpio(index) {
        if (!_pcf.begin(hexStringToUint8(addr), &Wire)) {
            Serial.println("Ошибка инициализации PCF8574."); // Переводим на русский
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
            Serial.println("Неправильный индекс Pcf8574. Должен быть 0 - 4."); // Переводим на русский
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

    IoTGpio* getGpioDriver() {
        return _driver;
    }

    ~Pcf8574() {
        delete _driver;
    }
};

void* getAPI_Pcf8574(String subtype, String param) {
    if (subtype == F("Pcf8574")) {
        return new Pcf8574(param);
    } else {
        return nullptr;
    }
}
