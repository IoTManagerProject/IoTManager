#pragma once

#include <Arduino.h>
#include "Global.h"

extern void itemsListInit();
extern void addItem2(String param);
extern void addItem(String name);
extern void addPreset(String name);
extern void addPreset2(int num);
extern void delChoosingItems();
extern void delAllItems();
extern uint8_t getNewElementNumber(String file);
extern uint8_t getFreePinAll();
extern bool isPinExist(unsigned int num);
extern uint8_t getFreePinAnalog();
