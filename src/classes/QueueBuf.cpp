#include "classes/QueueBuf.h"

QueueBuf::QueueBuf() {}
QueueBuf::~QueueBuf() {}

//добавим элемент в конец очереди
void QueueBuf::push(int element) {
    queue1.push(element);
}

//удалим элемент из начала очереди
void QueueBuf::pop() {
    if (!queue1.empty()) {
        queue1.pop();
    }
}

//вернуть элемент из начала очереди и удалить его
int QueueBuf::front() {
    int ret;
    if (!queue1.empty()) {
        ret = queue1.front();
        queue1.pop();
    }
    return ret;
}

QueueBuf* myQueue;