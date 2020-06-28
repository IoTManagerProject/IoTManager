#include "Bus/DallasScanner.h"

#include "Utils/PrintMessage.h"
#include "Objects/DallasSensors.h"

const char* MODULE = "DallasScanner";

DallasScanner::DallasScanner() : BusScanner(TAG_ONE_WIRE, 1) {}

bool DallasScanner::syncScan() {
    if (!dallasTemperature) {
        pm.error(String("DallasTemperature is null"));
        return false;
    }
    size_t countSensors = dallasTemperature->getDeviceCount();
    DeviceAddress* sensorsUnique = new DeviceAddress[countSensors];
    // определяем в каком режиме питания подключены сенсоры
    addResult("Mode: " + String(dallasTemperature->isParasitePowerMode() ? "Parasite" : "Normal") + ": ");
    for (size_t i = 0; i < countSensors; i++) {
        dallasTemperature->getAddress(sensorsUnique[i], i);
    }
    // выводим полученные адреса
    for (size_t i = 0; i < countSensors; i++) {
        addResult(sensorsUnique[i]);
    }
    return countSensors > 0;
}
