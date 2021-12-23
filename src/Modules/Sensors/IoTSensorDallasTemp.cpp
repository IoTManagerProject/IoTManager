#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Utils/StringUtils.h"
#include "Class/IoTSensor.h"
#include "Class/IoTModule.h"


#include "DallasTemperature.h"
#include <OneWire.h>

extern std::vector<IoTModule*> iotModules;  //v3dev: вектор ссылок базового класса IoTModule - интерфейсы для общения со всеми поддерживаемыми системой модулями

#define IOTDALLASTEMPKEY "dallas-temp"

class IoTSensorDallas: public IoTSensor {
    private:
        //описание переменных экземпляра датчика - аналог глобальных переменных из Arduino
        OneWire* oneWire;
        DallasTemperature sensors;

        //описание параметров передаваемых из настроек датчика из веба
        String _addr;
        unsigned int _pin;
        unsigned int _index;

    public:
        //аналог setup() из Arduino
        IoTSensorDallas(String parameters) {
            init(jsonReadInt(parameters, "int"), IOTDALLASTEMPKEY);  //передаем часть базовых параметров в конструктор базового класса для обеспечения работы его методов
            _pin = jsonReadInt(parameters, "pin");
            _index = jsonReadInt(parameters, "index");
            _addr = jsonReadStr(parameters, "addr");             

            oneWire = new OneWire((uint8_t)_pin);
            sensors.setOneWire(oneWire);
            sensors.begin();
            sensors.setResolution(12);
        }
        
        ~IoTSensorDallas() {}

        //аналог loop() из Arduino но квотируемый по времени параметром interval
        void doByInterval() {  
            SerialPrint("I", "Sensor", "Вызывается doByInterval");
            
            sensors.requestTemperatures();
            
            DeviceAddress deviceAddress;
            if (_addr == "") {
                sensors.getAddress(deviceAddress, _index);
            } else {
                string2hex(_addr.c_str(), deviceAddress);
            }

            float value = sensors.getTempC(deviceAddress);
            
            char addrStr[20] = "";
            hex2string(deviceAddress, 8, addrStr); 

            regEvent((String)value, "addr: " + String(addrStr));  //обязательный вызов для отправки результата работы
        }
};

//технический класс для взаимодействия с ядром, меняются только названия
class IoTModuleDallasTemp: public IoTModule {
    //обязательный метод для инициализации экземпляра датчика, вызывается при чтении конфигурации. Нужно учитывать, что некоторые датчики могут обеспечивать
    //несколько измерений, для каждого будет отдельный вызов.
    void* initInstance(String parameters) {
        return new IoTSensorDallas(parameters);
    };

    //обязательный метод для отправки информации о модуле, 
    ModuleInfo getInfo() {
        ModuleInfo MI;
        MI.key = IOTDALLASTEMPKEY;
        MI.name = "Датчик температуры Ds18b20";
        MI.parameters = "{\"addr\": \"\", \"int\": \"10\", \"pin\": \"18\", \"index\": \"0\"}";
        MI.type = "Sensor";
        return MI;
    };
};

//точка входа в модуль для заполнения вектора, требуется только изменить имя и прописать в файле api.cpp
void getApiIoTSensorDallasTemp() {
    iotModules.push_back(new IoTModuleDallasTemp());
    return;
}