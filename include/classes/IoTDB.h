#pragma once
#include "Global.h"
#include <queue>

using namespace std;

struct QueueItems {
    String myword;
    uint8_t num;
};

class IoTDB;

class IoTDB {
   public:
    IoTDB();
    ~IoTDB();

    void push(QueueItems word);
    void pop();
    QueueItems front();
    bool empty();

   private:
    queue<QueueItems> queue1;
    QueueItems tmpItem;
};

extern IoTDB* myDB;
