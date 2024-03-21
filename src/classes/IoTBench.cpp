#include "Global.h"
#include "classes/IoTBench.h"

IoTBench::IoTBench(const String &parameters) : IoTItem(parameters)
{
    /*    int _tx, _rx, _speed, _line;
        jsonRead(parameters, "rx", _rx);
        jsonRead(parameters, "tx", _tx);
        jsonRead(parameters,  "speed", _speed);
        jsonRead(parameters,  "line", _line);
    */
}

void IoTBench::preLoadFunction() {}
void IoTBench::postLoadFunction() {}
void IoTBench::preTaskFunction(const String &id) {}
void IoTBench::postTaskFunction(const String &id) {}

// void IoTBench::uartHandle() {}

IoTBench::~IoTBench() {}
