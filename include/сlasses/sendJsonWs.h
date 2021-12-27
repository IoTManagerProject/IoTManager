#pragma once
#include "Global.h"

class SendJsonWs;

class SendJsonWs {
   public:
    SendJsonWs();
    ~SendJsonWs();

    void send(String& jsonArrayElement, uint8_t& _num);
};
