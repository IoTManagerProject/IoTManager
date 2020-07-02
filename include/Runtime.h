#pragma once

#include "Arduino.h"

namespace Runtime {
String get();
void save();
void load();
void writeInt(String name, int value);
int readInt(const String& name);
String write(String name, String value);
String read(const String& obj);
}  // namespace Runtime
