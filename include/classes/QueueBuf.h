#pragma once
#include "classes/QueueInst.h"
#include "Global.h"
#include <queue>
#include <iostream>

using namespace std;

class QueueBuf;

class QueueBuf {
   public:
    QueueBuf();
    ~QueueBuf();

    void push(QueueInstance instance);
    void pop();
    QueueInstance front();

   private:
    queue<QueueInstance> queue1;
};

extern QueueBuf* myQueue;
