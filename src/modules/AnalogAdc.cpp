#include "Global.h"
#include "Classes/IoTSensor.h"

//Это файл сенсора, в нем осуществляется чтение сенсора.
//для добавления сенсора вам нужно скопировать этот файл и переименовать в нем AnalogAdc
//сочетание слов на название Вашего сенсора

class AnalogAdc : public IoTSensor {
   private:
    //=======================================================================================================
    // Секция переменных.
    //Это секция где Вы можете объявлять переменные и объекты arduino библиотек, что бы
    //впоследствии использовать их в loop и setup
    unsigned int _pin;

   public:
    //=======================================================================================================
    // setup()
    //это аналог setup из arduino. Здесь вы можете выполнять методы инициализации сенсора.
    //Такие как ...begin и подставлять в них параметры полученные из web интерфейса.
    //Все параметры хранятся в переменнной parameters, вы можете прочитать любой параметр используя jsonRead функции:
    // jsonReadStr, jsonReadBool, jsonReadInt
    AnalogAdc(String parameters) {
        init(jsonReadStr(parameters, "key"), jsonReadStr(parameters, "id"), jsonReadInt(parameters, "int"));

        _pin = jsonReadInt(parameters, "pin");
    }
    //=======================================================================================================
    // loop()
    //это аналог loop из arduino. Здесь вы должны выполнить чтение вашего сенсора в переменную float
    //а затем выполнить regEvent - это регистрация произошедшего события чтения
    //здесь так же доступны все переменные в секции переменных, и полученные в setup
    void doByInterval() {
        float value = analogRead(_pin);

        regEvent((String)value, "AnalogAdc");
    }
    //=======================================================================================================

    ~AnalogAdc();
};

//данную функцию вы должны создать по образу и подобию, изменив в ней AnalogAdc на название Вашего сенсора
void* getAPI_AnalogAdc(String parameters) {
    String subtype;
    if (!jsonRead(parameters, F("subtype"), subtype)) {  //если нет такого ключа в представленном json или он не валидный
        SerialPrint(F("E"), F("Config"), F("json error AnalogAdc"));
        return nullptr;
    } else {
        if (subtype == F("AnalogAdc")) {
            return new AnalogAdc(parameters);
        } else {
            return nullptr;
        }
    }
}
