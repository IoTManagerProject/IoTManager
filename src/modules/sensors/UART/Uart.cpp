
#include "Global.h"
#include "classes/IoTUart.h"
#include "classes/IoTItem.h"
#include <Arduino.h>

#include "modules/sensors/UART/Uart.h"

#ifdef ESP8266
    SoftwareSerial* myUART = nullptr;
#else
    Stream* myUART = nullptr;
#endif

class IoTmUART : public IoTUart {
   private:
    int _eventFormat = 0;   // 0 - нет приема, 1 - json IoTM, 2 - Nextion
    char _inc;
    String _inStr = "";     // буфер приема строк в режимах 0, 1, 2

   public:
    IoTmUART(String parameters) : IoTUart(parameters) {
        jsonRead(parameters,  "eventFormat", _eventFormat);
        myUART = _myUART;
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
                id.replace(".val", "_val");
                id.replace(".txt", "_txt");
                generateOrder(id, valStr);
            break;
        }
    }

    void uartHandle() {
        if (!_myUART) return;
        if (_myUART->available()) {
            if (_eventFormat == 3) {    // третий режим, значит ожидаем бинарный пакет данных от dwin оставлен для совместимости с предыдущей версией модуля
                // используйте новый модуль для dwin DwinI
            } else {
                _inc = _myUART->read();
                if (_inc == 0xFF) {
                    _inc = _myUART->read();
                    _inc = _myUART->read();
                    _inStr = "";
                    return;
                }

                if (_inc == '\r') return;
                
                if (_inc == '\n') {
                    analyzeString(_inStr);
                    _inStr = "";
                } else _inStr += _inc;
            }
        }
    }

    void onRegEvent(IoTItem* eventItem) {
        if (!_myUART || !eventItem) return; 
        int indexOf_;
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
                indexOf_ = printStr.indexOf("_");
                if (indexOf_ == -1) return;  // пропускаем событие, если нет используемого признака типа данных - _txt или _vol
                
                if (printStr.indexOf("_txt") > 0) {
                    printStr.replace("_txt", ".txt=\"");
                    printStr += eventItem->getValue();
                    printStr += "\"";
                } else if (printStr.indexOf("_val") > 0) { 
                    printStr += eventItem->getValue();
                    printStr.replace(".", "");
                    printStr.replace("_val", ".val=");
                } else {
                    if (indexOf_ == printStr.length()-1) printStr.replace("_", "");
                    else printStr.replace("_", ".");
                    printStr += "=";
                    printStr += eventItem->getValue();
                }

                uartPrintFFF(printStr);
            break;
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (command == "printFFF") {
            if (param.size() == 2) {
                String strToUart = "";
                strToUart = param[0].valS;

                if (param[1].valD) 
                    uartPrintFFF("\"" + strToUart + "\"");
                else
                    uartPrintFFF(strToUart);
            }
        } else {    // не забываем, что переопределяем execute и нужно проверить что в базовом классе проверяется
            return IoTUart::execute(command, param);
        }

        return {}; 
    }

    void uartPrintFFF(const String& msg) {
        if (_myUART) {
            _myUART->print(msg);
            _myUART->write(0xff);
            _myUART->write(0xff);
            _myUART->write(0xff);
        }
    }
};

void* getAPI_UART(String subtype, String param) {
    if (subtype == F("UART")) {
        return new IoTmUART(param);
    } else {
        return nullptr;
    }
}
