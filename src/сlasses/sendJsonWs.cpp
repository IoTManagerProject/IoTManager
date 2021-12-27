#include "сlasses/sendJsonWs.h"

SendJsonWs::SendJsonWs() {}
SendJsonWs::~SendJsonWs() {}

void SendJsonWs::send(String& jsonArrayElement, uint8_t& _num) {
    standWebSocket.sendTXT(_num, jsonArrayElement);
}
