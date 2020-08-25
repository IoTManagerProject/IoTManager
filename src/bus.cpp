#include "Bus/BusScannerFactory.h"
#include "Class/NotAsinc.h"
#include "Global.h"

void busInit() {
    myNotAsincActions->add(
        do_BUSSCAN, [&](void*) {
            doBusScan();
        },
        nullptr);
}

void doBusScan() {
    String res = "";
    BusScanner* scanner = BusScannerFactory::get(configSetupJson, busToScan, res);
    scanner->scan();
    jsonWriteStr(configLiveJson, String(scanner->tag()), res);
}