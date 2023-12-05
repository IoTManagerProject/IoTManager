
#define DEBUG_SERIAL_ENABLE
#include "Global.h"
#include "classes/IoTUart.h"
#include "ESPNexUpload.h"
bool updated = false;

class Nextion : public IoTUart
{
private:
    String _url;
    String _host;
    int _tx, _rx, _speed, _line;
    bool _UpTelegram;
    char _inc;
    String _inStr = "";     // буфер приема строк в режимах 0, 1, 2

    // Выводим русские буквы на экран Nextion (преобразуем в кодировку ISO-8859-5)
    String convertRUS(String text)
    {
        const char *in = text.c_str();
        String out;
        if (in == NULL)
            return out;

        uint32_t codepoint = 0;
        while (*in != 0)
        {
            uint8_t ch = (uint8_t)(*in);
            if (ch <= 0x7f)
                codepoint = ch;
            else if (ch <= 0xbf)
                codepoint = (codepoint << 6) | (ch & 0x3f);
            else if (ch <= 0xdf)
                codepoint = ch & 0x1f;
            else if (ch <= 0xef)
                codepoint = ch & 0x0f;
            else
                codepoint = ch & 0x07;
            ++in;
            if (((*in & 0xc0) != 0x80) && (codepoint <= 0x10ffff))
            {
                if (codepoint <= 255)
                {
                    out += (char)codepoint;
                }
                else
                {
                    if (codepoint > 0x400)
                        out += (char)(codepoint - 0x360);
                }
            }
        }
        return out;
    }    

public:
    Nextion(String parameters) : IoTUart(parameters)
    {
        _url = jsonReadStr(parameters, "url");
        _url = "/" + _url;
        _host = jsonReadStr(parameters, "host");
        jsonRead(parameters, "rx", _rx);
        jsonRead(parameters, "tx", _tx);
        jsonRead(parameters,  "speed", _speed);
        jsonRead(parameters,  "line", _line);
        jsonRead(parameters, "uploadTelegram", _UpTelegram);
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {

        if (command == "Update")
        {
            updateServer();
        }
        else if (command == "printFFF") 
        {
            if (param.size() == 2) 
            //UART.printFFF("auto.val=1",0)
            {
                String strToUart = "";
                strToUart = param[0].valS;

                if (param[1].valD) 
                    uartPrintFFF("\"" + strToUart + "\"");
                else
                    uartPrintFFF(strToUart);
            }
            // отправка данных на Nextion - UART.printFFF("t1.txt=", ID_vidget + " °", 1);
            // или UART.printRusFFF("buttons.bt0.txt=","  Гостинная",1);
            if (param.size() == 3)
            {
                String strToUart = "";
                strToUart = param[0].valS;

                if (param[2].valD)
                    uartPrintFFF(strToUart + "\"" + param[1].valS + "\"");
                else
                    uartPrintFFF(strToUart + param[1].valS);
            }
        }        
        // отправка кирилических символов на Nextion (русские буквы)
        else if (command == "printRusFFF")
        {
            if (param.size() == 2)
            {
                String strToUart = "";
                strToUart = param[0].valS;

                if (param[1].valD)
                    uartPrintFFF(convertRUS("\"" + strToUart + "\""));
                else
                    uartPrintFFF(convertRUS(strToUart));
            }
            if (param.size() == 3)
            {
                String strToUart = "";
                strToUart = param[0].valS;

                if (param[2].valD)
                    uartPrintFFF(convertRUS(strToUart + "\"" + param[1].valS + "\""));
                else
                    uartPrintFFF(convertRUS(strToUart + param[1].valS));
            }
        }// else {    // не забываем, что переопределяем execute и нужно проверить что в базовом классе проверяется
       //     return IoTUart::execute(command, param);
      //  }
        return {}; 
    }

    void onModuleOrder(String &key, String &value) {
        if (key == "uploadServer") {
            updateServer();
        }
    }

    void uartPrintFFF(const String& msg) {
        if (_myUART) {
            SerialPrint("I", F("Nextion"), "uartPrintFFF -> "+msg+" +FFFFFF");
            _myUART->print(msg);
            _myUART->write(0xff);
            _myUART->write(0xff);
            _myUART->write(0xff);
        }
    }
//---------------------NEXTION-UART---START------------------------
    void uartHandle() {
        if (!_myUART) return;
        if (_myUART->available()) {
            _inc = _myUART->read();
            if (_inc == 0xFF) {
                _inc = _myUART->read();
                _inc = _myUART->read();
                _inStr = "";
                return;
            }

            if (_inc == '\r') return;
                
            if (_inc == '\n') {
                if (_inStr.indexOf("=") == -1) {   // если входящее сообщение не по формату, то работаем как в режиме 0
                    setValue(_inStr);
                    return;
                }
                String id = selectToMarker(_inStr, "=");
                String valStr = selectToMarkerLast(_inStr, "=");
                valStr.replace("\"", "");
                id.replace(".val", "_val");
                id.replace(".txt", "_txt");
                generateOrder(id, valStr);
                _inStr = "";
            } else _inStr += _inc;
        }
    }

     void onRegEvent(IoTItem* eventItem) {
        if (!_myUART || !eventItem) return; 
        int indexOf_;
        String printStr = "";

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

            uartPrintFFF(convertRUS(printStr));
    }
 
//---------------------NEXTION-UART---END------------------------

//---------------------NEXTION-UPDATE---START------------------------
    void updateServer()
    {
            SerialPrint("I", F("NextionUpdate"), "Update .... ");

            if (!updated)
            {
                SerialPrint("I", F("NextionUpdate"), "connecting to  " + (String)_host);
                HTTPClient http;
#if defined ESP8266
                if (!http.begin(_host, 80, _url))
                    SerialPrint("I", F("NextionUpdate"), "connection failed  ");
#elif defined ESP32
                if (!http.begin(String("http://") + _host + _url))
                    SerialPrint("I", F("NextionUpdate"), "connection failed  ");
#endif

                SerialPrint("I", F("NextionUpdate"), "Requesting file:   " + (String)_url);
                int code = http.GET();
                // Update the nextion display
                if (code == 200)
                    flashNextion(http);
                else
                    SerialPrint("I", F("NextionUpdate"), "HTTP error:    " + (String)http.errorToString(code).c_str());

                http.end();
                SerialPrint("I", F("NextionUpdate"), "Closing connection    ");
            }
    }

    void uploadNextionTlgrm(String &url)
    {
        if (!_UpTelegram)
            return;
        if (!updated)
        {
            SerialPrint("I", F("NextionUpdate"), "connecting to  " + url);

            HTTPClient http;

#ifdef ESP8266 
            SerialPrint("I", F("NextionUpdate"), "Update impossible esp8266: Change boards to esp32 :)");          
            return;
#else 
            if (!http.begin(url)) // пингуем файл
                SerialPrint("I", F("NextionUpdate"), "connection failed  ");
#endif
            SerialPrint("I", F("NextionUpdate"), "Requesting file: OK" );
            int code = http.GET();
            // Update the nextion display
            if (code == 200)
                flashNextion(http);
            else
                SerialPrint("I", F("NextionUpdate"), "HTTP error:    " + (String)http.errorToString(code).c_str());

            http.end();
            SerialPrint("I", F("NextionUpdate"), "Closing connection    ");
        }
    }

    void flashNextion(HTTPClient &http)
    {
        int contentLength = http.getSize();
        SerialPrint("I", F("NextionUpdate"), "File received. Update Nextion...   ");
        bool result;
        ESPNexUpload nexUp(_speed, _line, _rx, _tx);
        nexUp.setUpdateProgressCallback([]()
                                          { SerialPrint("I", F("NextionUpdate"), "...   "); });

        result = nexUp.prepareUpload(contentLength);
        if (!result)
        {
            SerialPrint("I", F("NextionUpdate"), "Error:    " + (String)nexUp.statusMessage);
        }
        else
        {
            SerialPrint("I", F("NextionUpdate"), "Start upload. File size is:     " + (String)contentLength);
            result = nexUp.upload(*http.getStreamPtr());
            if (result)
            {
                updated = true;
                SerialPrint("I", F("NextionUpdate"), "Succesfully updated Nextion!     ");
            }
            else
            {
                SerialPrint("I", F("NextionUpdate"), "Error updating Nextion:      " + (String)nexUp.statusMessage);
            }
            nexUp.end();
        }
    }
//---------------------NEXTION-UPDATE---END------------------------

    ~Nextion(){};

};

void *getAPI_Nextion(String subtype, String param)
{
    if (subtype == F("Nextion"))
    {
        return new Nextion(param);
    }
    else
    {
        return nullptr;
    }
}
