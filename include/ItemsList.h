#pragma once

#include <Arduino.h>
#include "Global.h"

extern void itemsListInit();
extern void addItem(String name);
extern void delChoosingItems();
extern void delAllItems();
extern uint8_t getNewElementNumber(String file);
extern uint8_t getFreePinAll();
extern uint8_t getFreePinAnalog();
