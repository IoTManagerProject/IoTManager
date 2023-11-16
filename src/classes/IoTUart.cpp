#include "Global.h"
#include "classes/IoTUart.h"


IoTUart::IoTUart(const String& parameters) : IoTItem(parameters) { 
    int _tx, _rx, _speed, _line;
    jsonRead(parameters, "rx", _rx);
    jsonRead(parameters, "tx", _tx);
    jsonRead(parameters,  "speed", _speed);
    jsonRead(parameters,  "line", _line);

#ifdef ESP8266
    _myUART = new SoftwareSerial(_rx, _tx);
    _myUART->begin(_speed);
#endif
#ifdef ESP32
    if (_line >= 0) {
        _myUART = new HardwareSerial(_line);
        ((HardwareSerial*)_myUART)->begin(_speed, SERIAL_8N1, _rx, _tx);
    } else {
        _myUART = new SoftwareSerial(_rx, _tx);
        ((SoftwareSerial*)_myUART)->begin(_speed);
    }
#endif
}

void IoTUart::loop() {
    uartHandle();
}

void IoTUart::uartHandle() {}

IoTUart::~IoTUart() {}


void IoTUart::uartPrintStrInUTF16(const char *strUTF8, int length) {   
    // очень жесткий но быстрый способ конвертирования UTF-8 в UTF-16, но с поддержкой только кириллицы и двух байт в UTF-8
    // не определяются исключения по формату UTF-8
    for (int i=0; i < length; i++) {
        if (strUTF8[i] < 176) {     // если байт соответствует коду ASCII, значит берем как есть, но расширяем до двух байт
            //_myUART->write(0x00);
            _myUART->write(strUTF8[i]);
        } else {                    // иначе понимаем, что имеем дело с двумя байтами (да UTF8 может иметь и больше, но это ограничение наше)
            _myUART->write(0x04);   // указываем номер диапазона символов кириллицы первым байтом на выходе
            
            if (strUTF8[i] == 208) {    // если первый байт символа в первом диапазоне
                if (strUTF8[i+1] == 129) _myUART->write(0x01);  // исключение для символа 'ё'
                else  _myUART->write(strUTF8[i+1] - 128);   // применяем смещение 128 и отправляем второй байт
            }

            if (strUTF8[i] == 209) {    // если первый байт символа во втором диапазоне
                if (strUTF8[i+1] == 145) _myUART->write(0x51);  // исключение для символа 'Ё'
                else _myUART->write(strUTF8[i+1] - 64);     // применяем смещение 64 и отправляем второй байт
            }

            i++;    // пропускаем второй байт входной строки
        }
    }
}

void IoTUart::uartPrintln(const String& msg) {
    if (_myUART) {
        _myUART->println(msg);
    }
}

void IoTUart::uartPrint(const String& msg) {
    if (_myUART) {
        _myUART->print(msg);
    }
}

void IoTUart::uartPrintHex(const String& msg) {
    if (!_myUART) return;
    
    unsigned char Hi, Lo;
    uint8_t byteTx;
    const char* strPtr = msg.c_str();
    while ((Hi = *strPtr++) && (Lo = *strPtr++)) {
        byteTx = (ChartoHex(Hi) << 4) | ChartoHex(Lo);
        _myUART->write(byteTx);
    }
}

IoTValue IoTUart::execute(String command, std::vector<IoTValue> &param) {
    if (command == "println") { 
        if (param.size() == 1) {
            //if (param[0].isDecimal) uartPrintln((String)param[0].valD);
            //else uartPrintln(param[0].valS);
            uartPrintln(param[0].valS);
        }
    } else if (command == "print") { 
        if (param.size() == 1) {
            //if (param[0].isDecimal) uartPrint((String)param[0].valD);
            //else uartPrint(param[0].valS);
            uartPrintln(param[0].valS);
        }
    } else if (command == "printHex") {
        if (param.size() == 1) {
            uartPrintHex(param[0].valS);
        }
    } 

    return {};
}