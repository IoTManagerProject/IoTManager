#include "Bus/OneWireScanner.h"

#include "Utils/PrintMessage.h"

const char* MODULE = "OneWire";

OneWireScanner::OneWireScanner(String& out, uint8_t pin) : BusScanner(TAG_ONE_WIRE, out, 1) {
    _bus = oneWireBus.get(pin);
}

bool OneWireScanner::syncScan() {
    uint8_t addr[8];
    pm.info("scanning...");
    while (_bus->search(addr)) {
        for (uint8_t i = 0; i < 8; i++) {
            pm.info("found: " + String(i, DEC));
            addResult(addr[i], i < 7);
        }
    }
    if (OneWire::crc8(addr, 7) != addr[7]) {
        pm.error("CRC!");
        return false;
    }
    _bus->reset_search();
    return true;
}
