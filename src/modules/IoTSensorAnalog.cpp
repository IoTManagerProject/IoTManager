#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Utils/StringUtils.h"
#include "Classes/IoTSensor.h"
#include <map>
class IoTSensorAnalog : public IoTSensor {
   private:
    //описание переменных экземпляра датчика - аналог глобальных переменных
    //описание параметров передаваемых из настроек датчика из веба
    unsigned int _pin;

   public:
    //аналог setup()
    IoTSensorAnalog(String parameters) {
        //передаем часть базовых параметров в конструктор базового класса для обеспечения работы его методов
        init(jsonReadStr(parameters, "key"), jsonReadStr(parameters, "id"), jsonReadInt(parameters, "int"));

        _pin = jsonReadInt(parameters, "pin");
    }

    //аналог loop()
    void doByInterval() {
        float value = analogRead(_pin);

        regEvent((String)value, "analog");  //обязательный вызов для отправки результата работы
    }

    ~IoTSensorAnalog() {}
};