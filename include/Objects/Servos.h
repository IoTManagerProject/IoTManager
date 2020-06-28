#pragma once

#include <Arduino.h>
#include <Servo.h>

struct Servo_t {
    uint8_t num;
    uint8_t pin;
    Servo* obj;
    Servo_t(uint8_t num, uint8_t pin) : num{num}, pin{pin}, obj{nullptr} {};
};

class Servos {
   public:
    Servos();
    Servo* get(uint8_t num);
    Servo* create(uint8_t num, uint8_t pin);
    size_t count();

   private:
    std::vector<Servo_t> _items;
};

extern Servos myServo;