#ifdef QUEUE_FROM_INST
#include "queue/QueueInst.h"

QueueInstance::QueueInstance(String text) {
    _text = text;
}
QueueInstance::~QueueInstance() {}

String QueueInstance::get() {
    return _text;
}

//========проверка очереди из экземпляров======

// myQueue = new QueueFromInstance;

// myQueue->push(QueueInstance("text1"));
// myQueue->push(QueueInstance("text2"));
// myQueue->push(QueueInstance("text3"));

// Serial.println(myQueue->front().get());
// Serial.println(myQueue->front().get());
// Serial.println(myQueue->front().get());
#endif