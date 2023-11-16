#pragma once
#include <Arduino.h>
#include "classes/IoTItem.h"

#ifdef ESP8266
#include <SoftwareSerial.h>
#else
#include <HardwareSerial.h>
#include <SoftwareSerial.h>
#endif


class IoTUart : public IoTItem {
    public:
        IoTUart(const String& parameters);
        ~IoTUart();

        virtual void uartHandle();

        virtual void loop();

        void uartPrintStrInUTF16(const char *strUTF8, int length);
        void uartPrintln(const String& msg);
        void uartPrint(const String& msg);
        void uartPrintHex(const String& msg);
        virtual IoTValue execute(String command, std::vector<IoTValue> &param);

    protected:
#ifdef ESP8266
        SoftwareSerial* _myUART;
#else
        Stream* _myUART;
#endif

};