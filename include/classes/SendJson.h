#pragma once
#include "Global.h"
#ifdef QUEUE_FROM_STR
#include "classes/QueueFromStruct.h"

class SendJson;

class SendJson {
   public:
    SendJson();
    ~SendJson();

    void addFileToQueue(String path, uint8_t num);

    void loop();

    void sendWs(String& jsonArrayElement);

    void sendMqtt(String& jsonArrayElement);

    QueueItems myItem;

   private:
    File file;
    String _path;
    uint8_t _num;
    bool sendingInProgress = false;
};

extern SendJson* sendJsonFiles;
#endif
