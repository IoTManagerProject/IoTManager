#pragma once

#include "Base/KeyValueStore.h"

class Widget {
   public:
    Widget();
    ~Widget();

    bool loadTemplate(const char* name);

    void write(const String& key, const String& value);

    void writeInt(const String& key, const String& value);

    void writeFloat(const String& key, const String& value);

    const String asJson();

   private:
    const String getFilename(const char* name);

   private:
    KeyValueFile* _content;
};