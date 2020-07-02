#pragma once

#include "Timings.h"

static const size_t NUM_ITEMS = 10;
static const char* module_name[NUM_ITEMS] = {"Clock", "NonAsync", "Mqtt", "Cmd", "Btn", "Scen", "Udp", "Serial", "TS", "Looger"};

class Metric {
   private:
    Timing _items[NUM_ITEMS];
    unsigned long long _loop_cnt;
    unsigned long long _total_mu;
    unsigned long _start;

   public:
    Metric() : _loop_cnt{0}, _start{0} {};

    void add(size_t num, unsigned long now = micros()) {
        unsigned long time = now - _start;
        _total_mu += time;
        _items[num].add(time);
        _start = now;
    }

    void loop() {
        _loop_cnt++;
        _start = micros();
    }

    void print(Print& p) {
        if (!_total_mu || !_loop_cnt) {
            return;
        }
        unsigned long long total_ms = _total_mu / 1000;
        p.printf("loops: %llu total: %llu ms ", _loop_cnt, total_ms);
        for (size_t i = 0; i < NUM_ITEMS; i++) {
            Timing* tt = &_items[i];
            float per = (float)(tt->_total_mu / 1000) / total_ms * 100;
            if (!per) {
                continue;
            }
            p.printf("%s: %2.2f%%%s", module_name[i], per, i < NUM_ITEMS - 1 ? ", " : "\n");
        }
    }

    void reset() {
        for (size_t i = 0; i < NUM_ITEMS; i++) {
            _items[i].reset();
        }
        _total_mu = 0;
        _loop_cnt = 0;
    }
};