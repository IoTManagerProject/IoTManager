#pragma once
#include "Global.h"
#ifdef QUEUE_FROM_STR
#include <queue>
//#include <iostream> долбанный стрим сука

using namespace std;

struct QueueItems {
    String myword;
    uint8_t num;
};

class QueueFromStruct;

class QueueFromStruct {
   public:
    QueueFromStruct();
    ~QueueFromStruct();

    void push(QueueItems word);
    void pop();
    QueueItems front();
    bool empty();

   private:
    queue<QueueItems> queue1;
    QueueItems tmpItem;
};

extern QueueFromStruct* filesQueue;

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

#endif
