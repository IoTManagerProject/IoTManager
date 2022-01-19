#include "classes/QueueInst.h"

QueueInstance::QueueInstance(String text) {
    _text = text;
}
QueueInstance::~QueueInstance() {}

String QueueInstance::get() {
    return _text;
}
