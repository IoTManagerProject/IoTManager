#include "classes/IoTDB.h"

IoTDB::IoTDB() {}
IoTDB::~IoTDB() {}

//добавим элемент в конец очереди
void IoTDB::push(QueueItems word) {
    queue1.push(word);
}

//удалим элемент из начала очереди
void IoTDB::pop() {
    if (!queue1.empty()) {
        queue1.pop();
    }
}

//вернуть элемент из начала очереди и удалить его
QueueItems IoTDB::front() {
    if (!queue1.empty()) {
        tmpItem = queue1.front();
        queue1.pop();
    }
    return tmpItem;
}

bool IoTDB::empty() {
    return queue1.empty();
}

IoTDB* myDB;
