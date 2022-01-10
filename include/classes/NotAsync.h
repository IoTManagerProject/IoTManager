#pragma once
#include <Arduino.h>
#include <stdint.h>

#include <functional>

typedef std::function<void(void*)> NotAsyncCb;

struct NotAsyncItem {
    bool test;
    NotAsyncCb cb;
    void* cb_arg;
    volatile bool is_used = false;
};

class NotAsync {
   private:
    uint8_t size;
    uint8_t task = 0;
    NotAsyncItem* items = NULL;
    void handle(NotAsyncCb f, void* arg);

   public:
    NotAsync(uint8_t size);
    ~NotAsync();

    void add(uint8_t i, NotAsyncCb, void* arg);
    void make(uint8_t task);
    void loop();
};

extern NotAsync* myNotAsyncActions;