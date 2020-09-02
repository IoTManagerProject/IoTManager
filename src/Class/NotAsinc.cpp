#include "Class/NotAsinc.h"

NotAsinc::NotAsinc(uint8_t size) {
    this->items = new NotAsincItem[size];
    this->size = size;
}

NotAsinc::~NotAsinc() {}

void NotAsinc::add(uint8_t i, NotAsincCb f, void* arg) {
    this->items[i].cb = f;
    this->items[i].cb_arg = arg;
    this->items[i].is_used = true;
}

void NotAsinc::loop() {
        if (this->items[task].is_used) {
            handle(this->items[task].cb, this->items[task].cb_arg);
            task = 0;
        }
}

void NotAsinc::make(uint8_t task) {
   this->task = task;
}

void NotAsinc::handle(NotAsincCb f, void* arg) {
    f(arg);
}
NotAsinc* myNotAsincActions;