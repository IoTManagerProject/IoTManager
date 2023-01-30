
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
    int _eventFormat = 0;   // 0 - нет приема, 1 - json IoTM, 2 - Nextion

#ifdef ESP8266
    SoftwareSerial* _myUART = nullptr;
#else
    HardwareSerial* _myUART = nullptr;
#endif

   public:
    UART(String parameters) : IoTItem(parameters) {
        int _tx, _rx, _speed, _line;
        jsonRead(parameters,  "tx", _tx);
        jsonRead(parameters,  "rx", _rx);
        jsonRead(parameters,  "speed", _speed);
        jsonRead(parameters,  "line", _line);
        jsonRead(parameters,  "eventFormat", _eventFormat);

#ifdef ESP8266
        myUART = _myUART = new SoftwareSerial(_rx, _tx);
        _myUART->begin(_speed);
#endif
#ifdef ESP32
        myUART = _myUART = new HardwareSerial(_line);
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
                id.replace(".val", "_val");
                id.replace(".txt", "_txt");
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
                //Serial.println(printStr + " fff " + indexOf_);
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

            case 3:             // формат событий для Dwin
            //for (int i=0; i<2; i++) {
                printStr = eventItem->getID();
                indexOf_ = printStr.indexOf("_");
                if (indexOf_ == -1 || !_myUART) return;  // пропускаем событие, если нет используемого признака типа данных - _txt или _vol
                
                String VP = selectToMarkerLast(printStr, "_");

                _myUART->write(0x5A);
                _myUART->write(0xA5);

                if (eventItem->value.isDecimal) {  // пока отправляем только целые числа
                    _myUART->write(0x05);   // размер данных отправляемых с учетом целых чисел int
                    _myUART->write(0x82);   // требуем запись в память
                    uartPrintHex(VP);       // отправляем адрес в памяти VP

                    byte raw[2];
                    (int&)raw = eventItem->value.valD;
                    _myUART->write(raw[1]);
                    _myUART->write(raw[0]);
                } else {
                    // подсчитываем количество символов отличающихся от ASCII, для понимания сколько символов состоит из дух байт
                    int u16counter = 0;
                    const char* valSptr = eventItem->value.valS.c_str();
                    //Serial.print("iiiii ");
                    for (int i=0; i < eventItem->value.valS.length(); i++) {
                        if (valSptr[i] > 200) u16counter++;
                        //Serial.printf("%d ", valSptr[i]);
                    }
                    //Serial.println();

                    _myUART->write((eventItem->value.valS.length() - u16counter) * 2 + 5);  // подсчитываем и отправляем размер итоговой строки + служебные байты
                    _myUART->write(0x82);   // требуем запись в память
                    uartPrintHex(VP);   // отправляем адрес в памяти VP
                    Serial.println("ffffff " + VP);
                    //_myUART->write(0x53);
                    //_myUART->write(0x00);
                    uartPrintStrInUTF16(eventItem->value.valS.c_str(), eventItem->value.valS.length());     // отправляем строку для записи
                    _myUART->write(0xFF);   // терминируем строку, чтоб экран очистил все остальное в элементе своем
                    _myUART->write(0xFF);

                    //uint8_t Data[8] = {0x00, 0x31, 0x00, 0x44, 0x04, 0x10, 0x00, 0x00};
                    //uartPrintArray(Data, 6);
                    //Serial.printf("fffffffff %#x %#x %#x %#x \n", Data[0], Data[1], Data[2], Data[3]);
                }
            //}    
            break;
        }
    }

    void uartPrintStrInUTF16(const char *strUTF8, int length) {   
        // очень жесткий но быстрый способ конвертирования UTF-8 в UTF-16, но с поддержкой только кириллицы и двух байт в UTF-8
        // не определяются исключения по формату UTF-8
        for (int i=0; i < length; i++) {
            if (strUTF8[i] < 176) {     // если байт соответствует коду ASCII, значит берем как есть, но расширяем до двух байт
                _myUART->write(0x00);
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

    void uartPrintArray(uint8_t *_Data, uint8_t _Size) {
        for (size_t i = 0; i < _Size; i++) _myUART->write(_Data[i]);
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
        } else if (command == "printFFF") {
            if (param.size() == 2) {
                String strToUart = "";
                // if (param[0].isDecimal) 
                //     strToUart = param[0].valD;  
                // else 
                //     strToUart = param[0].valS;
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
