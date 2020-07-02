#pragma once

#include "Arduino.h"

class KeyValueStore {
   public:
    KeyValueStore();
    float readFloat(String name);
    void writeFloat(String name, float value);
    void writeInt(String name, int value);
    int readInt(const String& name);
    String write(String name, String value);
    String read(const String& obj);
    String get();

   protected:
    String _pool;
};

class KeyValueFile : public KeyValueStore {
   public:
    KeyValueFile(const char* filename);

    void save();
    void load();

   private:
    char _filename[33];
};