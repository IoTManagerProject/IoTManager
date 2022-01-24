#pragma once

#include <WString.h>

#define PAGE_UPDATE_ms 1000
#define PAGE_CHANGE_ms 10000

namespace ST7565 {
    void show(const String& data, const String& meta);
    void draw();
}