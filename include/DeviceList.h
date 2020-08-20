#pragma once

#include <Arduino.h>
#include "Global.h"

extern void addElement(String name);
extern void delAllElement();
extern int getNewElementNumber(String file);
extern void delElement(String itemsFile, String itemsLine);
extern void do_delElement();