#pragma once
#include "Global.h"

class SendJson;

class SendJson {
   public:
    SendJson();
    ~SendJson();

    void sendFile(String path, uint8_t num);

    void loop();

    void sendWs(String& jsonArrayElement);

    void sendMqtt(String& jsonArrayElement);

    uint8_t _num;

   private:
    File file;
    String _path;
};

extern SendJson* sendConfigJson;
extern SendJson* sendWigdetsJson;
