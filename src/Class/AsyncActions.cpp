#include "Class/AsyncActions.h"

AsyncActions::AsyncActions(){};

void AsyncActions::loop() {
    count++;
    if (count > 5000) {

        // Проверяем что переменная содержит указатель - не пустая не null
        // и непосредственно вызываем то, на что это указывает
        // просто пишем имя - без () - это указатель на фунецию.
        // () - вызываем функцию - с пустым набором параметров

        if (_cb != NULL) {
            _cb();
        }
        //или ровно тоже самое
        //if (_cb) _cb();

        if (_pcb) {
            if (_pcb("SomeTextValue")) {
                Serial.println("Got true!");
            } else {
                Serial.println("Got false!");
            }
        }

        count = 0;
    }
}

//передаем внутрь класса функцию любую void функцию без агрументов
void AsyncActions::setCallback(AsyncActionCb cb) {  
    _cb = cb;
}

//передаем внутрь класса функцию любую void функцию с аргументами
void AsyncActions::setCallback(AsyncParamActionCb pcb) {  
    _pcb = pcb;
}
AsyncActions* async;