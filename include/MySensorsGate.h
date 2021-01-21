#include "Consts.h"
#include "Global.h"
#ifdef MYSENSORS
//#define MY_DEBUG
#define MY_RADIO_RF24
#define MY_RF24_PA_LEVEL RF24_PA_LOW
#define MY_GATEWAY_SERIAL
#define CHILD_ID 1
#include "MySensors.h"
extern void receive(const MyMessage &message);
#endif