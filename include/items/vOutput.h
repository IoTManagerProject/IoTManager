#pragma once
#include <Arduino.h>

#include "Global.h"

class Output;

typedef std::vector<Output> MyOutputVector;

class Output {
   public:

    Output(String key);
    ~Output();
    
    void execute(String value);

   private:

    String _key;

};

extern MyOutputVector* myOutput;

extern void output();
extern void outputExecute();

