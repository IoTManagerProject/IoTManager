#include "Bus/BusScanner.h"

#include "Utils/PresetUtils.h"
#include "Utils/PrintMessage.h"

#include <OneWire.h>

const char* MODULE = "1Wire";

bool OneWireScanner::syncScan() {
    // Connect your 1-wire device to pin 3
    OneWire ds(3);
    uint8_t addr[8];

    pm.info("scanning 1-Wire...");
    while (ds.search(addr)) {
        for (uint8_t i = 0; i < 8; i++) {
            pm.info("found device: " + i);
            addResult(addr[i], i < 7);
        }
    }
    if (OneWire::crc8(addr, 7) != addr[7]) {
        pm.error("CRC!");
        addResult("не найдено");
        return false;
    }
    ds.reset_search();
    return true;
}
