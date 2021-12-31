#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Utils/StringUtils.h"

#include "Class/IoTVariable.h"
#include "Class/IoTModule.h"


class IoTVariableButtonOut: public IoTVariable {
    private:
        //описание переменных экземпляра Variable - аналог глобальных переменных из Arduino
        bool _state;

        //описание параметров передаваемых из настроек переменной из веба
        bool _isInvert;
        int _pin;

    public:
        //аналог setup() из Arduino
        IoTVariableButtonOut(String parameters) {
            //передаем часть базовых параметров в конструктор базового класса для обеспечения работы его методов
            init(jsonReadStr(parameters, "key"), jsonReadStr(parameters, "id"));  
            
            _pin = jsonReadBool(parameters, "pin");
            _isInvert = jsonReadBool(parameters, "inv");

            _state = this->loadValue(_id); //прочитали из памяти 
            if (_pin) {
                pinMode(_pin, OUTPUT);
                this->execute(String(_state)); //установили это состояние
            }
        }
        
        ~IoTVariableButtonOut() {}

        //аналог loop() из Arduino
        void loop() {  
            
        }

        //вызывается при выполнении команды связанной с конкретным экземпляром переменной, передаем команду целиком после идентификатора в сценарии (минус пробел)
        String execute(String command) { 
            if (command != "" && _pin > 0) {
                if (command == "change") {
                    _state = !digitalRead(_pin);
                    digitalWrite(_pin, _state);
                } else {
                    int newState = command.toInt();
                    if (_isInvert) {
                        digitalWrite(_pin, !newState);
                    } else {
                        digitalWrite(_pin, newState);
                    }
                }
                selfExec();
                regEvent((String)_state, "");
            }
            return "";
        }

        //EXPEREMENTAL.вызывается при любом изменении переменной (задумка для реализации возможности вызова своего кода при клике на кнопку в веб-интерфейсе. 
        //Создаем модуль с реализацией и выбираем в конфигурации нужный виджет с кнопкой) для других реализаций
        void selfExec() {

        }

        String getValue(String key) {
            return (String)_state;
        }
};

//технический класс для взаимодействия с ядром, меняются только названия
class IoTModuleButtonOut: public IoTModule {
    //обязательный метод для инициализации экземпляра переменной, вызывается при чтении конфигурации.
    void* initInstance(String parameters) {
        return new IoTVariableButtonOut(parameters);
    };

    //обязательный к заполнению метод, если модуль использует свои глобальные переменные. Необходимо сбросить и очистить используемую память.
    void clear() {
       //и так чисто 
    }

    //обязательный метод для отправки информации о модуле, 
    ModuleInfo getInfo() {
        ModuleInfo MI;
        MI.name = "button-out";
        MI.title = "Кнопка управляющая пином";
        MI.parameters = "{\"key\": \"button-out\", \"id\": \"var\", \"pin\": \"2\", \"inv\": \"0\"}";
        MI.type = "Variable";
        return MI;
    };
};

//точка входа в модуль для заполнения вектора, требуется только изменить имя и прописать в файле api.cpp
    void* getApiIoTVariableButtonOut() {
    return new IoTModuleButtonOut();
}