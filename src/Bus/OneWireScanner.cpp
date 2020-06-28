#include "Bus/OneWireScanner.h"
#include "DallasTemperature.h"
#include "Utils/PrintMessage.h"
#include "Global.h"

const char* MODULE = "OneWire";

OneWireScanner::OneWireScanner(String& out, uint8_t pin) : BusScanner(TAG_ONE_WIRE, out, 1) {
    _bus = oneWireBus.get(pin);
    if (!_bus) {
        pm.error(String("bus is null"));
    }
}

bool OneWireScanner::syncScan() {
    pm.info(String("scanning..."));
    sensors.setOneWire(_bus);
    size_t countSensors = sensors.getDeviceCount();
    DeviceAddress* sensorsUnique = new DeviceAddress[countSensors];
    // определяем в каком режиме питания подключены сенсоры
    addResult("Mode: " + String(sensors.isParasitePowerMode() ? "Parasite" : "Normal") + ": ");
    for (size_t i = 0; i < countSensors; i++) {
        sensors.getAddress(sensorsUnique[i], i);
    }
    // выводим полученные адреса
    for (size_t i = 0; i < countSensors; i++) {
        addResult(*sensorsUnique[i], i < countSensors - 1);
    }
    return countSensors > 0;
}
