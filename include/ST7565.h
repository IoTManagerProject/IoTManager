#pragma once

#include <WString.h>

#define DISPLAY_REFRESH_ms 1000
#define PAGE_CHANGE_ms 5000
// #define DISPLAY_I2C

namespace ST7565 {
    void show(const String& data, const String& meta);
    void draw();
}