#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Utils/StringUtils.h"
#include "Class/IoTSensor.h"
#include "Class/IoTModule.h"


#include "DallasTemperature.h"
#include <OneWire.h>
#include <map>

extern std::vector<IoTModule*> iotModules;  //v3dev: вектор ссылок базового класса IoTModule - интерфейсы для общения со всеми поддерживаемыми системой модулями
#define IOTDALLASTEMPKEY "dallas-temp"

//глобальные списки необходимы для хранения объектов об активных линиях 1-wire используемых разными датчиками из модуля. Ключ - номер пина
std::map<int, OneWire*> oneWireTemperatureArray;
std::map<int, DallasTemperature*> sensorsTemperatureArray;

class IoTSensorDallas: public IoTSensor {
    private:
        //описание переменных экземпляра датчика - аналог глобальных переменных из Arduino
        //для работы библиотеки с несколькими линиями  необходимо обеспечить каждый экземпляр класса ссылками на объекты настроенные на эти линии
        OneWire* oneWire;
        DallasTemperature* sensors;

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

            //учитываем, что библиотека может работать с несколькими линиями на разных пинах, поэтому инициируем библиотеку, если линия ранее не использовалась
            if (oneWireTemperatureArray.find(_pin) == oneWireTemperatureArray.end()) {
                oneWire = new OneWire((uint8_t)_pin);
                sensors = new DallasTemperature();
                sensors->setOneWire(oneWire);
                sensors->begin();
                sensors->setResolution(12);

                oneWireTemperatureArray[_pin] = oneWire;
                sensorsTemperatureArray[_pin] = sensors;
            } else {
                oneWire = oneWireTemperatureArray[_pin];
                sensors = sensorsTemperatureArray[_pin];
            }
        }
        
        ~IoTSensorDallas() {}

        //аналог loop() из Arduino но квотируемый по времени параметром interval
        void doByInterval() {  
            //запускаем опрос измерений у всех датчиков на линии
            sensors->requestTemperatures();
            
            //Определяем адрес. Если парамтер addr не установлен, то узнаем адрес по индексу
            DeviceAddress deviceAddress;
            if (_addr == "") {
                sensors->getAddress(deviceAddress, _index);
            } else {
                string2hex(_addr.c_str(), deviceAddress);
            }
            //получаем температуру по адресу
            float value = sensors->getTempC(deviceAddress); 
           
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