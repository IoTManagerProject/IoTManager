#pragma once
#include "Global.h"

class StreamJsonArray;

class StreamJsonArray {
   public:
    StreamJsonArray();
    ~StreamJsonArray();

    void sendFile(String path, uint8_t num);

    void loop();

   private:
    File file;
    String _path;
    uint8_t _num;
};

extern StreamJsonArray* myStreamJsonArray;