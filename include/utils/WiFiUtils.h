#pragma once
#include "Global.h"
#include "MqttClient.h"

typedef enum {
    _scan_for_ws,
    _scan_for_connect
} mode_scan_t;

typedef enum {
    _FindAP_start = 0b00,     // запускаем поиск
    _FindAP_finding,            // ищем
    // _FindAP_sleep,              // спим
    _FindAP_end                 // закончили
} state_scan_t;

typedef struct {
    bool mode_scan_for_ws : 1;
    bool mode_scan_for_connect : 1;
    state_scan_t state_scan : 2;
} wifi_t;

boolean isNetworkActive();
uint8_t getNumAPClients();
void routerConnect();
bool startAPMode();
void AsyncRouterFind(mode_scan_t mode);
uint8_t RSSIquality();
extern void wifiSignalInit();
