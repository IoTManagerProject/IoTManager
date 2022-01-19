#pragma once
#include "Global.h"
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
