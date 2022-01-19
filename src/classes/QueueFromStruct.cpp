#include "classes/QueueFromStruct.h"
#ifdef QUEUE_FROM_STR
QueueFromStruct::QueueFromStruct() {}
QueueFromStruct::~QueueFromStruct() {}

//добавим элемент в конец очереди
void QueueFromStruct::push(QueueItems word) {
    queue1.push(word);
}

//удалим элемент из начала очереди
void QueueFromStruct::pop() {
    if (!queue1.empty()) {
        queue1.pop();
    }
}

//вернуть элемент из начала очереди и удалить его
QueueItems QueueFromStruct::front() {
    if (!queue1.empty()) {
        tmpItem = queue1.front();
        queue1.pop();
    }
    return tmpItem;
}

bool QueueFromStruct::empty() {
    return queue1.empty();
}

QueueFromStruct* filesQueue;
#endif
