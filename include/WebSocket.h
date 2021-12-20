#pragma once

#include "Global.h"
void wsInit();
void wsSendSetup();
void wsPublishData(String topic, String data);
void sendDataWs();
