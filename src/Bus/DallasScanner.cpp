#include "Bus/DallasScanner.h"

#include "Sensors/DallasSensor.h"
#include "Utils/PrintMessage.h"

const char* MODULE = "DallasScanner";

DallasScanner::DallasScanner() : BusScanner(TAG_ONE_WIRE, 1) {}

bool DallasScanner::syncScan() {
    if (!Dallas::dallasTemperature) {
        pm.error(String("OneWire inactive"));
        return false;
    }
    size_t countSensors = Dallas::dallasTemperature->getDeviceCount();
    DeviceAddress* sensorsUnique = new DeviceAddress[countSensors];
    // определяем в каком режиме питания подключены сенсоры
    addResult("Mode: " + String(Dallas::dallasTemperature->isParasitePowerMode() ? "Parasite" : "Normal") + ": ");
    for (size_t i = 0; i < countSensors; i++) {
        Dallas::dallasTemperature->getAddress(sensorsUnique[i], i);
    }
    // выводим полученные адреса
    for (size_t i = 0; i < countSensors; i++) {
        addResult(sensorsUnique[i]);
    }
    return countSensors > 0;
}
