#pragma once
#include "Class/CircularBuffer.h"
#include "Global.h"
void wsInit();
void wsSendSetup();
void wsSendSetupBuffer();
void wsPublishData(String topic, String data);
void sendDataWs();
void loopWsExecute();
