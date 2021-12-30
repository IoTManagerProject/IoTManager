#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Utils/StringUtils.h"

#include "Class/IoTVariable.h"
#include "Class/IoTModule.h"


class IoTVariableVirtual: public IoTVariable {
    private:
        //описание переменных экземпляра Variable - аналог глобальных переменных из Arduino
        String value;

        //описание параметров передаваемых из настроек переменной из веба
        
    public:
        //аналог setup() из Arduino
        IoTVariableVirtual(String parameters) {
            //передаем часть базовых параметров в конструктор базового класса для обеспечения работы его методов
            init(jsonReadStr(parameters, "key"), jsonReadStr(parameters, "id"));  
            
        }
        
        ~IoTVariableVirtual() {}

        //аналог loop() из Arduino
        void loop() {  
            
        }

        //вызывается при выполнении команды связанной с конкретным экземпляром переменной
        String execute(String command) { 
        
            return "";
        }

        //вызывается при любом изменении переменной
        void selfExec() {

        }
};

//технический класс для взаимодействия с ядром, меняются только названия
class IoTModuleVariable: public IoTModule {
    //обязательный метод для инициализации экземпляра переменной, вызывается при чтении конфигурации.
    void* initInstance(String parameters) {
        return new IoTVariableVirtual(parameters);
    };

    //обязательный к заполнению метод, если модуль использует свои глобальные переменные. Необходимо сбросить и очистить используемую память.
    void clear() {
       //и так чисто 
    }

    //обязательный метод для отправки информации о модуле, 
    ModuleInfo getInfo() {
        ModuleInfo MI;
        MI.name = "variable";
        MI.title = "Переменная для хранения значений пользователя";
        MI.parameters = "{\"key\": \"variable\", \"id\": \"var\"}";
        MI.type = "Variable";
        return MI;
    };
};

//точка входа в модуль для заполнения вектора, требуется только изменить имя и прописать в файле api.cpp
    void* getApiIoTVariableVirtual() {
    return new IoTModuleVariable();
}