#pragma once

#include <Arduino.h>
#include "Global.h"

extern void addElement(String name);
extern void delAllElement();
extern int getNewElementNumber(String file);
extern void do_getJsonListFromCsv();
extern String getJsonListFromCsv(String csvFile,int colum);
extern void do_delElement();
extern void delElement(String itemsFile, String itemsLine);