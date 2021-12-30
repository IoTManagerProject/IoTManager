#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Utils/StringUtils.h"
#include "Class/IoTSensor.h"
#include "Class/IoTModule.h"

#include <Bounce2.h>

class IoTSensorButtonIn: public IoTSensor {
    private:
        //описание переменных экземпляра датчика - аналог глобальных переменных из Arduino
        //для работы библиотеки с несколькими линиями  необходимо обеспечить каждый экземпляр класса ссылками на объекты настроенные на эти линии
        Bounce* bButton;
        boolean status;

        //описание параметров передаваемых из настроек датчика из веба
        unsigned int _pin;
        unsigned int _db;

    public:
        //аналог setup() из Arduino
        IoTSensorButtonIn(String parameters) {
            //передаем часть базовых параметров в конструктор базового класса для обеспечения работы его методов
            init(jsonReadStr(parameters, "key"), jsonReadStr(parameters, "id"), 0);  
            _pin = jsonReadInt(parameters, "pin");
            _db = jsonReadInt(parameters, "db");           

            bButton = new Bounce();
            bButton->attach(_pin, INPUT);
            bButton->interval(_db);
            status = true;
        }
        
        ~IoTSensorButtonIn() {
            delete bButton;
        }

        //аналог loop() из Arduino, но квотируемый по времени параметром interval
        void doByInterval() {  
            bButton->update();
            if (bButton->fell()) {
                status = 1;
                regEvent((String)status, "");  //обязательный вызов для отправки результата работы
            }
            if (bButton->rose()) {
                status = 0;
                regEvent((String)status, "");  //обязательный вызов для отправки результата работы
            }
        }
};

//технический класс для взаимодействия с ядром, меняются только названия
class IoTModuleButtonIn: public IoTModule {
    //обязательный метод для инициализации экземпляра датчика, вызывается при чтении конфигурации. Нужно учитывать, что некоторые датчики могут обеспечивать
    //несколько измерений, для каждого будет отдельный вызов.
    void* initInstance(String parameters) {
        return new IoTSensorButtonIn(parameters);
    };

    //обязательный к заполнению метод, если модуль использует свои глобальные переменные. Необходимо сбросить и очистить используемую память.
    void clear() {
       //и так чисто 
    }

    //обязательный метод для отправки информации о модуле, 
    ModuleInfo getInfo() {
        ModuleInfo MI;
        MI.name = "button-in";
        MI.title = "Кнопка физическая, чтение состояния пина (подключается проводами к устройству)";
        MI.parameters = "{\"key\": \"button-in\", \"id\": \"btn\", \"pin\": \"2\", \"db\": \"20\"}";
        MI.type = "Sensor";
        return MI;
    };
};

//точка входа в модуль для заполнения вектора, требуется только изменить имя и прописать в файле api.cpp
void* getApiIoTSensorButtonIn() {
    return new IoTModuleButtonIn();
}