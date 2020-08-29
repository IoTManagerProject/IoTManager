#ifdef SERVO_ENABLED
#include "Servo/Servos.h"
Servos myServo;

Servos::Servos(){};

Servo *Servos::create(uint8_t num, uint8_t pin) {
    // Ищем среди ранее созданных
    for (size_t i = 0; i < _items.size(); i++) {
        auto item = _items.at(i);
        if (item.num == num) {
            if (item.pin != pin) {
                item.obj->attach(pin);
                item.pin = pin;
            };
            return item.obj;
        }
    }
    // Добавляем новый
    Servo_t newItem{num, pin};
    newItem.obj = new Servo();
    newItem.obj->attach(pin);
    _items.push_back(newItem);
    return newItem.obj;
}

Servo *Servos::get(uint8_t num) {
    // Ищем среди ранее созданных
    for (size_t i = 0; i < _items.size(); i++) {
        auto item = _items.at(i);
        if (item.num == num) {
            return item.obj;
        }
    }
    return nullptr;
}

size_t Servos::count() {
    return _items.size();
}
#endif
