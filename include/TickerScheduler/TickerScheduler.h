#pragma once

#include "Arduino.h"
#include <stdint.h>

#include <Ticker.h>
#include <functional>

void tickerFlagHandle(volatile bool *flag);

typedef std::function<void(void *)> tscallback_t;

struct TickerSchedulerItem {
    Ticker t;
    bool flag = false;
    tscallback_t cb;
    void *cb_arg;
    uint32_t period;
    volatile bool is_used = false;
};

class TickerScheduler {
   private:
    uint8_t size;
    TickerSchedulerItem *items = NULL;

    void handleTicker(tscallback_t, void *, bool *flag);
    static void handleTickerFlag(bool *flag);

   public:
    TickerScheduler(uint8_t size);
    ~TickerScheduler();

    bool add(uint8_t i, uint32_t period, tscallback_t, void *, boolean shouldFireNow = false);
    bool remove(uint8_t i);
    bool enable(uint8_t i);
    bool disable(uint8_t i);
    void enableAll();
    void disableAll();
    void update();
};
