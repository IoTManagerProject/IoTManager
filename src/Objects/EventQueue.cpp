#include "Objects/EventQueue.h"

EventQueue events;

void EventQueue::push(const String item) {
    _pool.push_back(item);
}

const String EventQueue::pop() {
    String item = _pool.back();
    _pool.pop_back();
    return item;
}

size_t EventQueue::available() {
    return _pool.size();
};
