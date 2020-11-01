#pragma once
#include <Arduino.h>

extern void loopCmdAdd(const String &cmdStr);
extern void fileCmdExecute(const String &filename);
extern void csvCmdExecute(String &cmdStr);
extern void spaceCmdExecute(String &cmdStr);
extern void loopCmdExecute();
extern void addKey(String& key, int number);
extern int getKeyNum(String& key);