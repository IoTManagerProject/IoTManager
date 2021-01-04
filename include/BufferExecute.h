#pragma once
#include <Arduino.h>

extern void loopCmdAdd(const String& cmdStr);
extern void fileCmdExecute(const String& filename);
extern void csvCmdExecute(String& cmdStr);
extern void spaceCmdExecute(String& cmdStr);
extern void loopCmdExecute();
extern void addKey(String& key, String& keyNumberTable, int number);
extern int getKeyNum(String& key, String& keyNumberTable);

extern void buttonIn();
extern void buttonInSet();






