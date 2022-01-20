#pragma once
#include "Global.h"
#ifdef QUEUE_FROM_INST
#include <queue>
#include <iostream>

using namespace std;

class QueueInstance;

class QueueInstance {
   public:
    QueueInstance(String text);
    ~QueueInstance();

    String get();

   private:
    String _text;
};
#endif
