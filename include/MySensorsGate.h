#include "Consts.h"
#include "Global.h"
#ifdef MYSENSORS
//#define MY_DEBUG
#define MY_RADIO_RF24
#define MY_RF24_PA_LEVEL RF24_PA_HIGH //RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
//#define MY_ENCRYPTION_SIMPLE_PASSWD "XpenBam"
//#define MY_RF24_DATARATE RF24_2MBPS				
#define MY_GATEWAY_SERIAL
#include "MySensors.h"
extern void receive(const MyMessage &message);
extern String parseToString(const MyMessage &message);
#endif

