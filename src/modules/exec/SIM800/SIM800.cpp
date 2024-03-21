
#include "Global.h"
#include "classes/IoTUart.h"

class Sim800 : public IoTUart
{
private:
    bool _debug;
    String _number;
    char _inc;
    String _inStr = ""; // буфер приема строк

public:
    Sim800(String parameters) : IoTUart(parameters)
    {
        _number = jsonReadStr(parameters, "number");
        jsonRead(parameters, "debug", _debug);
    }

    void _printUart(bool ln, String str)
    {
        if (!_myUART)
            return;
        if (!ln)
        {
            _myUART->print(str);
            if (_debug)
                SerialPrint("I", F("SIM800"), "<- print(" + str + ")");
        }
        else
        {
            _myUART->println(str);
            if (_debug)
                SerialPrint("I", F("SIM800"), "<- println(" + str + ")");
        }
    }

    void sendSms(String sms, String num)
    {
        _printUart(1, "AT+CMGF=1"); // переводим в текстовый режим
        delay(2);
        _printUart(1, "AT+CMGS=\"" + num + "\"");
        delay(2);
        //_printUart(1, sms + "\r\n" + String((char)26));
        _myUART->println(sms + "\r\n" + String((char)26));
            if (_debug)
                SerialPrint("I", F("SIM800"), "<- println(" + sms + ")");
        // _myUART->print((char)26); // код ctrl+c что является командой передачи сообщения
    }

    void doByInterval()
    {
        if (_myUART)
        {
            // _myUART->println("AT");       // должен ответить OK
            // _myUART->println("AT+CSQ");   // уровень сигнала в dB
            // _myUART->println("AT+CCID");  // если есть сим карта, то вернет её номер
            //_printUart(1, "AT+COPS?"); //+COPS: 0,0,"MTS-RUS" - оператор
            _printUart(1, "AT+CPAS"); //+CPAS: 0 - готов к работе
            //_printUart(1, "AT+CREG?"); // проверка регистрации в сети, второй паратетр: 1-регистрация в сети, 5-роуминг
            // _myUART->println("ATI");      // имя модуля и его номер
        }
    }

    void uartHandle()
    {
        if (!_myUART)
            return;

        if (_myUART->available())
        {
            _inc = _myUART->read();

            if (_inc == '\r')
            {
                return;
            }
            if (_inc == '\n')
            {
                _inStr += "";//_inc;
                if (_debug && _inStr != "")
                    SerialPrint("I", F("SIM800"), "-> " + _inStr);

                if (_inStr.indexOf("CPAS") != -1)
                {
                    if (_inStr.indexOf("+CPAS: 0") != -1)
                        setValue("OK");
                    else
                        setValue("NO");
                }
                _inStr = "";
                return;
            }
            else
                _inStr += _inc;
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (!_myUART)
            return {};
        if (command == "sms")
        {
            if (param.size() == 1)
            {
                sendSms(param[0].valS, _number);
            }
            else if (param.size() == 2)
            {
                sendSms(param[0].valS, param[1].valS);
            }
        }
        // отправка кирилических символов на Nextion (русские буквы)
        else if (command == "print")
        {
            if (param.size() == 2)
            {
                String strToUart = "";
                strToUart = param[0].valS;

                if (param[1].valD)
                    _printUart(0, "\"" + strToUart + "\"");
                else
                    _printUart(0, strToUart);
            }
            if (param.size() == 3)
            {
                String strToUart = "";
                strToUart = param[0].valS;

                if (param[2].valD)
                    _printUart(0, strToUart + "\"" + param[1].valS + "\"");
                else
                    _printUart(0, strToUart + param[1].valS);
            }
        }
        else if (command == "println")
        {
            if (param.size() == 2)
            {
                String strToUart = "";
                strToUart = param[0].valS;

                if (param[1].valD)
                    _printUart(1, "\"" + strToUart + "\"");
                else
                    _printUart(1, strToUart);
            }
            if (param.size() == 3)
            {
                String strToUart = "";
                strToUart = param[0].valS;

                if (param[2].valD)
                    _printUart(1, strToUart + "\"" + param[1].valS + "\"");
                else
                    _printUart(1, strToUart + param[1].valS);
            }
        }
        else if (command == "printHex")
        {
            unsigned char Hi, Lo;
            uint8_t byteTx;
            const char *strPtr = param[0].valS.c_str();
            while ((Hi = *strPtr++) && (Lo = *strPtr++))
            {
                byteTx = (ChartoHex(Hi) << 4) | ChartoHex(Lo);
                _myUART->write(byteTx);
            }
            if (_debug)
                SerialPrint("I", F("SIM800"), "<- printHex(" + String(byteTx, HEX) + ")");
        }

        return {};
    }

    ~Sim800(){};
};

void *getAPI_SIM800(String subtype, String param)
{
    if (subtype == F("sim800"))
    {
        return new Sim800(param);
    }
    else
    {
        return nullptr;
    }
}
