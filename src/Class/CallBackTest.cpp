#include "Class/CallBackTest.h"

CallBackTest::CallBackTest(){};

void CallBackTest::loop() {
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
void CallBackTest::setCallback(AsyncActionCb cb) {
    _cb = cb;
}

//передаем внутрь класса функцию любую void функцию с аргументами
void CallBackTest::setCallback(AsyncParamActionCb pcb) {
    _pcb = pcb;
}
CallBackTest* CB;

//CB->setCallback([]() {
//    Serial.println("123");
//});
//
//CB->setCallback([](const String str) {
//    Serial.println(str);
//    return true;
//});