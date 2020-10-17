#pragma once
#include <Arduino.h>
#include <stdint.h>

#include <functional>

typedef std::function<void(void*)> NotAsincCb;

struct NotAsincItem {
    bool test;
    NotAsincCb cb;
    void * cb_arg;
    volatile bool is_used = false;
};

class NotAsinc {
   private:
    uint8_t size;
    uint8_t task = 0;
    NotAsincItem* items = NULL;
    void handle(NotAsincCb f, void* arg);

   public:
    NotAsinc(uint8_t size);
    ~NotAsinc();

    void add(uint8_t i, NotAsincCb, void* arg);
    void make(uint8_t task);
    void loop();
};
extern NotAsinc* myNotAsincActions;