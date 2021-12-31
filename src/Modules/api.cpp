#include "Utils/SerialPrint.h"
#include "Class/IoTSensor.h"
#include "Class/IoTModule.h"
#include "Class/IoTVariable.h"

extern std::vector<IoTModule*> iotModules;  //v3dev: вектор ссылок базового класса IoTModule - интерфейсы для общения со всеми поддерживаемыми системой модулями

//объявляем функцию для добавления модуля в вектор
void* getApiIoTSensorDallasTemp();
void*  getApiIoTSensorSHT20();
void* getApiIoTSensorButtonIn();

void* getApiIoTVariableVirtual();
void* getApiIoTVariableButtonOut();

//формируем вектор модулей путем вызова из каждого модуля специальной функции
//в дальнейшем предполагается отключать вызов, если модуль не участвует в сборке
void InitModulesApi() {
    iotModules.push_back((IoTModule*) getApiIoTSensorDallasTemp());
    iotModules.push_back((IoTModule*) getApiIoTSensorSHT20());
    iotModules.push_back((IoTModule*) getApiIoTSensorButtonIn());

    iotModules.push_back((IoTModule*) getApiIoTVariableVirtual());
    iotModules.push_back((IoTModule*) getApiIoTVariableButtonOut());
}