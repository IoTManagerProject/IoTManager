#include "Global.h"
#include "classes/IoTUart.h"


class DwinI : public IoTUart {
   private:
    uint8_t _headerBuf[260];    // буфер для приема пакета dwin
    int _headerIndex = 0;       // счетчик принятых байт пакета

   public:
    DwinI(String parameters) : IoTUart(parameters) {
        
    }

    void uartHandle() {
        if (!_myUART) return;
        
        if (_myUART->available()) {
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
        }
    }

    void onRegEvent(IoTItem* eventItem) {
        if (!_myUART || !eventItem) return; 
        int indexOf_;
        String printStr = "";
        
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
    }

    void onModuleOrder(String &key, String &value) {
        if (key == "uploadUI") {
            //SerialPrint("i", F("DwinI"), "Устанавливаем UI: " + value);
            
        }
    }
    
    ~DwinI(){
        
    };
};

void *getAPI_DwinI(String subtype, String param) {
    if (subtype == F("DwinI")) {
        return new DwinI(param);
    } else {
        return nullptr;
    }
}
