#pragma once

#include "Global.h"

#include "Module/Module.h"
#include "Module/Terminal.h"
#include "Module/CommandShell.h"

#include <functional>

enum TelnetEvent_t {
    TE_CONNECTED,
    TE_DISCONNECTED
};

typedef std::function<void(TelnetEvent_t, WiFiClient*)> TelnetEventHandler;

class Telnet : public Module {
   public:
    Telnet(uint16_t port) : _port{port}, _lastConnected{false} {};

   public:
    void setEventHandler(TelnetEventHandler);
    void sendData(const String&);
    bool hasClient();
    bool isShellActive();

   protected:
    bool onInit() override;
    void onEnd() override;
    bool onStart() override;
    void onStop() override;
    void onLoop() override;

   private:
    void onConnect();
    void onDisconnect();
    void onData();
    void onOpen();
    void onClose();

   private:
    TelnetEventHandler _eventHandler;
    uint16_t _port;
    bool _lastConnected;
    WiFiClient _client;
    WiFiServer* _server;
    Terminal* _term;
    CommandShell* _shell;
};
