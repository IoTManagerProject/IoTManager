
#include "Global.h"
#include "classes/IoTItem.h"
#include <Arduino.h>

#include "modules/sensors/UART/Uart.h"

#ifdef ESP8266
    SoftwareSerial* myUART = nullptr;
#else
    HardwareSerial* myUART = nullptr;
#endif

class UART : public IoTItem {
   private:
    int _tx;
    int _rx;
    int _speed;
    int _eventFormat = 0;   // 0 - нет приема, 1 - json IoTM, 2 - Nextion

#ifdef ESP8266
    SoftwareSerial* _myUART = nullptr;
#else
    HardwareSerial* _myUART = nullptr;
#endif

   public:
    UART(String parameters) : IoTItem(parameters) {
        jsonRead(parameters,  "tx", _tx);
        jsonRead(parameters,  "rx", _rx);
        jsonRead(parameters,  "speed", _speed);
        jsonRead(parameters,  "eventFormat", _eventFormat);

#ifdef ESP8266
        myUART = _myUART = new SoftwareSerial(_tx, _rx);
        _myUART->begin(_speed);
#endif
#ifdef ESP32
        _myUART = new HardwareSerial(2);
        _myUART->begin(_speed, SERIAL_8N1, _rx, _tx);
#endif
    }

    // проверяем формат и если событие, то регистрируем его
    void analyzeString(const String& msg) {
        switch (_eventFormat) {
            case 0:             // не указан формат, значит все полученные данные воспринимаем как общее значение из UART
                setValue(msg);
            break;
            
            case 1:             // формат событий IoTM с использованием json, значит создаем временную копию
                analyzeMsgFromNet(msg);
            break;
            
            case 2:             // формат команд от Nextion ID=Value
                if (msg.indexOf("=") == -1) {   // если входящее сообщение не по формату, то работаем как в режиме 0
                    setValue(msg);
                    break;
                }
                String id = selectToMarker(msg, "=");
                String valStr = selectToMarkerLast(msg, "=");
                valStr.replace("\"", "");
                generateOrder(id, valStr);
            break;
        }
    }

    void uartHandle() {
        if (!_myUART) return;
        
        if (_myUART->available()) {
            static String inStr = "";
            char inc;
            
            inc = _myUART->read();
            if (inc == 0xFF) {
                inc = _myUART->read();
                inc = _myUART->read();
                inStr = "";
                return;
            }

            if (inc == '\r') return;
            
            if (inc == '\n') {
                analyzeString(inStr);
                inStr = "";
            } else inStr += inc;
        }
    }

    void onRegEvent(IoTItem* eventItem) {
        if (!_myUART || !eventItem) return; 

        String printStr = "";
        switch (_eventFormat) {
            case 0: return;     // не указан формат, значит не следим за событиями
            case 1:             // формат событий IoTM с использованием json
                if (eventItem->isGlobal()) {
                    eventItem->getNetEvent(printStr);
                    _myUART->println(printStr);
                }
            break;
            
            case 2:             // формат событий для Nextion ID=Value0xFF0xFF0xFF
                printStr += eventItem->getID();
                if (printStr.indexOf("_") == -1) return;  // пропускаем событие, если нет используемого признака типа данных - _txt или _vol
                if (printStr.indexOf("_val") > 0) {
                    printStr.replace("_val", ".val=");
                    printStr += eventItem->getValue();
                } else if (printStr.indexOf("_txt") > 0) {
                    printStr.replace("_txt", ".txt=");
                    printStr += "\"";
                    printStr += eventItem->getValue();
                    printStr += "\"";
                } else return;
                uartPrintFFF(printStr);
            break;
        }
    }

    virtual void loop() {
        uartHandle();
    }

    void uartPrintFFF(const String& msg) {
        if (_myUART) {
            _myUART->print(msg);
            _myUART->write(0xff);
            _myUART->write(0xff);
            _myUART->write(0xff);
        }
    }

    void uartPrintln(const String& msg) {
        if (_myUART) {
            _myUART->println(msg);
        }
    }

    void uartPrint(const String& msg) {
        if (_myUART) {
            _myUART->print(msg);
        }
    }

    void uartPrintHex(const String& msg) {
        if (!_myUART) return;
        
        unsigned char Hi, Lo;
        uint8_t byteTx;
        const char* strPtr = msg.c_str();
        while ((Hi = *strPtr++) && (Lo = *strPtr++)) {
            byteTx = (ChartoHex(Hi) << 4) | ChartoHex(Lo);
            _myUART->write(byteTx);
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (command == "println") { 
            if (param.size() == 1) {
                if (param[0].isDecimal) uartPrintln((String)param[0].valD);
                else uartPrintln(param[0].valS);
            }
        } else if (command == "print") { 
            if (param.size() == 1) {
                if (param[0].isDecimal) uartPrint((String)param[0].valD);
                else uartPrint(param[0].valS);
            }
        } else if (command == "printHex") {
            if (param.size() == 1) {
                uartPrintHex(param[0].valS);
            }
        } else if (command == "printFFF") {
            if (param.size() == 2) {
                String strToUart = "";
                if (param[0].isDecimal) 
                    strToUart = param[0].valD;  
                else 
                    strToUart = param[0].valS;

                if (param[1].valD) 
                    uartPrintFFF("\"" + strToUart + "\"");
                else
                    uartPrintFFF(strToUart);
            }
        } 

        return {}; 
    }

};

void* getAPI_UART(String subtype, String param) {
    if (subtype == F("UART")) {
        return new UART(param);
    } else {
        return nullptr;
    }
}
