#include "Module/Telnet.h"

bool Telnet::onInit() {
    _server = new WiFiServer(_port);
    _term = new Terminal();
    _term->enableControlCodes();
    _term->enableEcho(false);
    _term->setStream(&_client);

    // _shell = new CommandShell(Cli::get());
    _shell->setTerm(_term);
    return true;
}

void Telnet::onEnd() {
    delete _server;
}

bool Telnet::onStart() {
    _server->begin();
    _server->setNoDelay(true);
    return _server->status() != CLOSED;
}

void Telnet::onStop() {
    if (hasClient()) {
        _client.stop();
    }
    _server->stop();
}

bool Telnet::hasClient() { return _client.connected(); }

void Telnet::sendData(const String& data) {
    if (hasClient()) {
        _client.write(data.c_str());
    }
}

void Telnet::setEventHandler(TelnetEventHandler h) { _eventHandler = h; }

void Telnet::onLoop() {
    if (_server->hasClient()) {
        if (!_client) {
            _client = _server->available();
        } else {
            if (!_client.connected()) {
                _server->stop();
                _client = _server->available();
            } else {
                WiFiClient rejected;
                rejected = _server->available();
                rejected.stop();
            }
        }
    }

    if (_lastConnected != hasClient()) {
        _lastConnected = hasClient();
        if (_lastConnected) {
            onConnect();
        } else {
            onDisconnect();
        }
    }

    if (hasClient() && _shell != nullptr) _shell->loop();
}

bool Telnet::isShellActive() {
    return _shell->active();
}

void Telnet::onConnect() {
    if (_eventHandler) {
        _eventHandler(TE_CONNECTED, &_client);
    }
}

void Telnet::onDisconnect() {
    if (_eventHandler) {
        _eventHandler(TE_DISCONNECTED, nullptr);
    }
}
