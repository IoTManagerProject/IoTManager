#pragma once

#include "Arduino.h"

namespace LiveData {
String get();
String read(const String& obj);
String write(String name, String value);
int readInt(const String& name);
void writeInt(String name, int value);
void writeFloat(String name, float value);

}  // namespace LiveData
