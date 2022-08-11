#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;


class Sonar : public IoTItem {
   private:
    unsigned int _pinTrig, _pinEcho;

   public:
    Sonar(String parameters): IoTItem(parameters) {
        _pinTrig = jsonReadInt(parameters, "pinTrig");
        _pinEcho = jsonReadInt(parameters, "pinEcho");

        pinMode(_pinTrig, OUTPUT); 
        pinMode(_pinEcho, INPUT);
    }
    
    void doByInterval() {
        // value.valD = IoTgpio.analogRead(_pin);
        // для большей точности установим значение LOW на пине Trig
        digitalWrite(_pinTrig, LOW); 
        delayMicroseconds(2); 
        // Теперь установим высокий уровень на пине Trig
        digitalWrite(_pinTrig, HIGH);
        // Подождем 10 μs 
        delayMicroseconds(10); 
        digitalWrite(_pinTrig, LOW); 
        // Рассчитаем расстояние
        value.valD = pulseIn(_pinEcho, HIGH) / 58;
        // Выведем значение в Serial Monitor
        Serial.print(value.valD); 
        Serial.println(" cm");

        regEvent(value.valD, "Sonar"); 
    }

    ~Sonar() {};
};


void* getAPI_Sonar(String subtype, String param) {
    if (subtype == F("Sonar")) {
        return new Sonar(param);
    } else {
        return nullptr;
    }
}
