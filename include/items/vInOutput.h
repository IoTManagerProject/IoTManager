#pragma once
#include <Arduino.h>

#include "Global.h"

class InOutput;

typedef std::vector<InOutput> MyInOutputVector;

class InOutput {
   public:

    InOutput(String key, String widget);
    ~InOutput();
    
    void execute(String value);

   private:

    String _key;

};

extern MyInOutputVector* myInOutput;

extern void inOutput();
extern void inOutputExecute();
