#pragma once
#include "Global.h"
#include <queue>
#include <iostream>

using namespace std;

struct QueueItems {
    String myword;
};

class QueueFromStruct;

class QueueFromStruct {
   public:
    QueueFromStruct();
    ~QueueFromStruct();

    void push(QueueItems word);
    void pop();
    QueueItems front();

   private:
    queue<QueueItems> queue1;
    QueueItems tmpItem;
};

extern QueueFromStruct* myQueueStruct;

//=======проверка очереди из структур=================
// myQueueStruct = new QueueFromStruct;
// QueueItems myItem;
// myItem.myword = "word1";
// myQueueStruct->push(myItem);
// myItem.myword = "word2";
// myQueueStruct->push(myItem);
// myItem.myword = "word3";
// myQueueStruct->push(myItem);
// Serial.println(myQueueStruct->front().myword);
// Serial.println(myQueueStruct->front().myword);
// Serial.println(myQueueStruct->front().myword);
