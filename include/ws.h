
#pragma once
#include <Arduino.h>

#include "Consts.h"
#include "Global.h"

class WebSockets;

class WebSockets {
   public:
    WebSockets();
    ~WebSockets();

    void sendFile(String path);

    void loop();

   private:
    File file;
    String _path;
};

extern WebSockets* myWebSockets;
