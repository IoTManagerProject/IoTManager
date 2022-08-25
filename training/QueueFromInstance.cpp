#ifdef QUEUE_FROM_INST
#include "classes/QueueFromInstance.h"

QueueFromInstance::QueueFromInstance() {}
QueueFromInstance::~QueueFromInstance() {}

//добавим элемент в конец очереди
void QueueFromInstance::push(QueueInstance instance) {
    queue1.push(instance);
}

//удалим элемент из начала очереди
void QueueFromInstance::pop() {
    if (!queue1.empty()) {
        queue1.pop();
    }
}

//вернуть элемент из начала очереди и удалить его
QueueInstance QueueFromInstance::front() {
    QueueInstance instance("");
    if (!queue1.empty()) {
        instance = queue1.front();
        queue1.pop();
    }
    return instance;
}

// QueueFromInstance* myQueue;
#endif
