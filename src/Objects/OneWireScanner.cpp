#include "Sensors/OneWireScanner.h"

#include "Sensors/OneWireBus.h"

#include "Utils/PrintMessage.h"

const char* MODULE = "OneWireScanner";

OneWireScanner::OneWireScanner() : BusScanner(TAG_ONE_WIRE) {}

OneWireScanner::~OneWireScanner(){};

bool OneWireScanner::onInit() {
    return onewire.attached();
}

bool OneWireScanner::onScan() {
    uint8_t addr[8];
    if (!onewire.get()->search(addr)) {
        onewire.get()->reset_search();
        return true;
    }
    onewire.addItem(addr);
    return false;
}
