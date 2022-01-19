#pragma once
#include "Global.h"
#include <queue>
#include <iostream>

using namespace std;

class QueueBuf;

class QueueBuf {
   public:
    QueueBuf();
    ~QueueBuf();

    void push(int element);
    void pop();
    int front();

   private:
    queue<int> queue1;
};

extern QueueBuf* myQueue;