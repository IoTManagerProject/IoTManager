#pragma once

#include <Arduino.h>
#include "Global.h"

extern void addItem(String name);
extern void delAllItems();
extern uint8_t getNewElementNumber(String file);
extern uint8_t getFreePin();
