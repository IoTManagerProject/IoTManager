#include "Utils/SerialPrint.h"
void getApiIoTSensorDallasTemp();

int InitModulesApi() {
    SerialPrint("I", "Debug", "call InitModulesApi");
    getApiIoTSensorDallasTemp();
}