#include "classes/QueueBuf.h"

QueueBuf::QueueBuf() {}
QueueBuf::~QueueBuf() {}

//добавим элемент в конец очереди
void QueueBuf::push(QueueInstance instance) {
    queue1.push(instance);
}

//удалим элемент из начала очереди
void QueueBuf::pop() {
    if (!queue1.empty()) {
        queue1.pop();
    }
}

//вернуть элемент из начала очереди и удалить его
QueueInstance QueueBuf::front() {
    QueueInstance instance("");
    if (!queue1.empty()) {
        instance = queue1.front();
        queue1.pop();
    }
    return instance;
}

QueueBuf* myQueue;
