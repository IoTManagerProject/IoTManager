#include "classes/NotAsync.h"

NotAsync::NotAsync(uint8_t size) {
    this->items = new NotAsyncItem[size];
    this->size = size;
}

NotAsync::~NotAsync() {}

void NotAsync::add(uint8_t i, NotAsyncCb f, void* arg) {
    this->items[i].cb = f;
    this->items[i].cb_arg = arg;
    this->items[i].is_used = true;
}

void NotAsync::loop() {
    if (this->items[task].is_used) {
        handle(this->items[task].cb, this->items[task].cb_arg);
        task = 0;
    }
}

void NotAsync::make(uint8_t task) {
    this->task = task;
}

void NotAsync::handle(NotAsyncCb f, void* arg) {
    f(arg);
}
NotAsync* myNotAsyncActions;