#pragma once
#include "Global.h"
#include "сlasses/sendJsonWs.h"

class SendJson;

class SendJson : SendJsonWs {
   public:
    SendJson();
    ~SendJson();

    void sendFile(String path, uint8_t num);

    void loop();

    uint8_t _num;

   private:
    File file;
    String _path;
};

extern SendJson* mySendJson;
