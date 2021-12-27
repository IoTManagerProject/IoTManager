#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Utils/StringUtils.h"
#include "Class/IoTSensor.h"
#include "Class/IoTModule.h"


#include "Wire.h"
#include "SHT2x.h"

extern std::vector<IoTModule*> iotModules;  //v3dev: вектор ссылок базового класса IoTModule - интерфейсы для общения со всеми поддерживаемыми системой модулями

SHT2x* sht = nullptr;

class IoTSensorSHT20: public IoTSensor {
    private:
        //описание переменных экземпляра датчика - аналог глобальных переменных из Arduino

        //описание параметров передаваемых из настроек датчика из веба
        float _c;

    public:
        //аналог setup() из Arduino
        IoTSensorSHT20(String parameters) {
            //передаем часть базовых параметров в конструктор базового класса для обеспечения работы его методов
            init(jsonReadStr(parameters, "key"), jsonReadStr(parameters, "id"), jsonReadInt(parameters, "int"));  
            _c = jsonReadFloat(parameters, "c");           

            if (!sht) {
                sht = new SHT2x;
                sht->begin();
            }                
        }
        
        ~IoTSensorSHT20() {}

        //аналог loop() из Arduino но квотируемый по времени параметром interval
        void doByInterval() {  
            //запускаем опрос измерений
            float value;
            if (getKey() == "anydataTemp") {
                value = sht->getTemperature();
            } else {
                value = sht->getHumidity();
            }
            value = _c * value;

            regEvent((String)value, "");  //обязательный вызов для отправки результата измерений
        }
};

//технический класс для взаимодействия с ядром, меняются только названия
class IoTModuleSHT20: public IoTModule {
    //обязательный метод для инициализации экземпляра датчика, вызывается при чтении конфигурации. Нужно учитывать, что некоторые датчики могут обеспечивать
    //несколько измерений, для каждого будет отдельный вызов.
    void* initInstance(String parameters) {
        return new IoTSensorSHT20(parameters);
    };

    //обязательный к заполнению метод, если модуль использует свои глобальные переменные. Необходимо сбросить и очистить используемую память.
    void clear() {
  
    }

    //обязательный метод для отправки информации о модуле, 
    ModuleInfo getInfo() {
        ModuleInfo MI;
        MI.name = "sht20";
        MI.title = "Датчик температуры и влажности SHT2x, HTU2x and Si70xx";
        //v3dev: key - это внутренний маркер-ключ определяющий значение для измерений, на этапе апробации на ver3 установлено значение = типу виджета, т.к. в таблице настройки отсутствует парамтер key в этой интерпретации
        MI.parameters = "{\"key\": \"anydataTemp\", \"id\": \"SHT20\", \"int\": \"10\", \"c\": \"1\"}, {\"key\": \"anydataHum\", \"id\": \"SHT20\", \"int\": \"10\", \"c\": \"1\"}";
        MI.type = "Sensor";
        return MI;
    };
};

//точка входа в модуль для заполнения вектора, требуется только изменить имя и прописать в файле api.cpp
void getApiIoTSensorSHT20() {
    iotModules.push_back(new IoTModuleSHT20());
    return;
}