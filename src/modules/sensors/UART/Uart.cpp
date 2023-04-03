
#include "Global.h"
#include "classes/IoTItem.h"
#include <Arduino.h>

#include "modules/sensors/UART/Uart.h"

#ifdef ESP8266
    SoftwareSerial* myUART = nullptr;
#else
    HardwareSerial* myUART = nullptr;
#endif

class IoTmUART : public IoTItem {
   private:
    int _eventFormat = 0;   // 0 - нет приема, 1 - json IoTM, 2 - Nextion
    char _inc;
    String _inStr = "";     // буфер приема строк в режимах 0, 1, 2
    uint8_t _headerBuf[260];    // буфер для приема пакета dwin
    int _headerIndex = 0;       // счетчик принятых байт пакета

#ifdef ESP8266
    SoftwareSerial* _myUART = nullptr;
#else
    HardwareSerial* _myUART = nullptr;
#endif

   public:
    IoTmUART(String parameters) : IoTItem(parameters) {
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
            if (_eventFormat == 3) {    // третий режим, значит ожидаем бинарный пакет данных от dwin
                _headerBuf[_headerIndex] = _myUART->read();

                // ищем валидный заголовок пакета dwin, проверяя каждый следующий байт
                if (_headerIndex == 0 && _headerBuf[_headerIndex] != 0x5A || 
                    _headerIndex == 1 && _headerBuf[_headerIndex] != 0xA5 || 
                    _headerIndex == 2 && _headerBuf[_headerIndex] == 0 || 
                    _headerIndex == 3 && _headerBuf[_headerIndex] == 0x82 ) {
                    _headerIndex = 0;
                    return;
                }

                if (_headerIndex == _headerBuf[2] + 2) {    // получили все данные из пакета
                    // Serial.print("ffffffff");
                    // for (int i=0; i<=_headerIndex; i++)
                    //     Serial.printf("%#x ", _headerBuf[i]);
                    // Serial.println("!!!");
                    
                    String valStr, id = "_";
                    if (_headerIndex == 8) {    // предполагаем, что получили int16
                       valStr = (String)((_headerBuf[7] << 8) | _headerBuf[8]);
                    }

                    char buf[5];
                    hex2string(_headerBuf + 4, 2, buf);
                    id += (String)buf;

                    IoTItem* item = findIoTItemByPartOfName(id);
                    if (item) {
                        //Serial.printf("received data: %s for VP: %s for ID: %s\n", valStr, buf, item->getID());
                        generateOrder(item->getID(), valStr);
                    }
                    
                    _headerIndex = 0;
                    return;
                }
                    
                _headerIndex++;

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
                printStr = eventItem->getID();
                indexOf_ = printStr.indexOf("_");
                uint8_t sizeOfVPPart = printStr.length() - indexOf_ - 1;
                if (indexOf_ == -1 || !_myUART || sizeOfVPPart < 4 || indexOf_ == 0)  return;  // пропускаем событие, если нет признака _ или признак пустой
                
                char typeOfVP = sizeOfVPPart == 5 ? printStr.charAt(indexOf_ + 5) : 0;
                String VP = printStr.substring(indexOf_ + 1, indexOf_ + 5);

                if (typeOfVP == 0) {    // если не указан тип, то додумываем на основании типа данных источника
                    if (eventItem->value.isDecimal)
                        typeOfVP = 'i';
                    else
                        typeOfVP = 's';
                }

                if (typeOfVP == 'i') {
                    _myUART->write(0x5A);
                    _myUART->write(0xA5);
                    _myUART->write(0x05);   // размер данных отправляемых с учетом целых чисел int
                    _myUART->write(0x82);   // требуем запись в память
                    uartPrintHex(VP);       // отправляем адрес в памяти VP
                    
                    if (!eventItem->value.isDecimal) {
                        eventItem->value.valD = atoi(eventItem->value.valS.c_str());
                    }

                    _myUART->write(highByte((int)eventItem->value.valD));
                    _myUART->write(lowByte((int)eventItem->value.valD));
                }

                if (typeOfVP == 's') {
                    if (eventItem->value.isDecimal) {
                        eventItem->value.valS = eventItem->getValue();
                    }

                    // подсчитываем количество символов отличающихся от ASCII, для понимания сколько символов состоит из дух байт
                    int u16counter = 0;
                    const char* valSptr = eventItem->value.valS.c_str();
                    for (int i=0; i < eventItem->value.valS.length(); i++) {
                        if (valSptr[i] > 200) u16counter++;
                    }

                    _myUART->write(0x5A);
                    _myUART->write(0xA5);
                    _myUART->write((eventItem->value.valS.length() - u16counter) * 2 + 5);  // подсчитываем и отправляем размер итоговой строки + служебные байты
                    _myUART->write(0x82);   // требуем запись в память
                    uartPrintHex(VP);   // отправляем адрес в памяти VP
                    uartPrintStrInUTF16(eventItem->value.valS.c_str(), eventItem->value.valS.length());     // отправляем строку для записи
                    _myUART->write(0xFF);   // терминируем строку, чтоб экран очистил все остальное в элементе своем
                    _myUART->write(0xFF);

                    //Serial.printf("fffffffff %#x %#x %#x %#x \n", Data[0], Data[1], Data[2], Data[3]);
                }

                if (typeOfVP == 'f') {
                    _myUART->write(0x5A);
                    _myUART->write(0xA5);
                    _myUART->write(0x07);   // размер данных отправляемых с учетом дробных чисел dword
                    _myUART->write(0x82);   // требуем запись в память
                    uartPrintHex(VP);       // отправляем адрес в памяти VP
                    
                    byte hex[4] = {0};
                    if (!eventItem->value.isDecimal) {
                       eventItem->value.valD = atof(eventItem->value.valS.c_str()); 
                    }

                    byte* f_byte = reinterpret_cast<byte*>(&(eventItem->value.valD));
                    memcpy(hex, f_byte, 4);

                    _myUART->write(hex[3]);
                    _myUART->write(hex[2]);
                    _myUART->write(hex[1]);
                    _myUART->write(hex[0]);
                }

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
        return new IoTmUART(param);
    } else {
        return nullptr;
    }
}
