#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <functional>

// Декларируем тип - сигнатуру метода , который мы готовы принять в данном случае это
// должен быть метод без результата и без параметров.
// Новый тип мы называем AsynсActionCb - хотя можешь назвать вообще как нравиться а что значит callBack

typedef std::function<void()> AsyncActionCb;                   //метод без результата и параметров
typedef std::function<bool(const String)> AsyncParamActionCb;  //метод без результата и параметров

class CallBackTest {
   private:
    long count;
    AsyncActionCb _cb;
    AsyncParamActionCb _pcb;
    

   public:
    CallBackTest();
    void loop();
    void setCallback(AsyncActionCb cb);
    void setCallback(AsyncParamActionCb pcb);
};
//extern CallBackTest* CB;