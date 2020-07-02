#pragma once

#include "Arduino.h"

namespace Options {
    
String get();
String write(String name, String value);
String read(const String& obj);
void writeInt(String name, int value);
int readInt(const String& name);
}  // namespace Options
