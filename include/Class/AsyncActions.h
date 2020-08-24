#pragma once

#include <stdint.h>

#include <functional>

// Декларируем тип - сигнатуру метода , который мы готовы принять в данном случае это
// должен быть метод без результата и без параметров.
// Новый тип мы называем AsynсActionCb - хотя можешь назвать вообще как нравиться а что значит callBack

//а как вызывать callback и когда их много? когда нужно вызывать например 10 разных из класса?
//

typedef std::function<void(void*)> AsyncActionCb; //метод без результата и параметров

class AsyncActions {
   private:
    long count;
    AsyncActionCb _cb;

   public:
    AsyncActions();
    void loop();
    void setCallback(AsyncActionCb cb) {  //передаем внутрь класса функцию любую void функцию без агрументов
        _cb = cb;
    }
};

extern AsyncActions* async;