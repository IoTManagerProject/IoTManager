#include "TickerScheduler/TickerScheduler.h"

TickerScheduler::TickerScheduler(uint8_t size) {
    this->items = new TickerSchedulerItem[size];
    this->size = size;

    _loop_cnt = 0;
    _total_mu = 0;
}

TickerScheduler::~TickerScheduler() {
    for (size_t i = 0; i < this->size; i++) {
        this->remove(i);
    }

    delete[] this->items;
    this->items = NULL;
    this->size = 0;
}

void TickerScheduler::handleTickerFlag(bool* flag) {
    if (!*flag) {
        *flag = true;
    }
}

void TickerScheduler::handleTicker(tscallback_t f, void* arg, bool* flag) {
    if (*flag) {
        *flag = false;
        f(arg);
    }
}

bool TickerScheduler::add(size_t i, uint32_t period, tscallback_t f, void* arg, boolean shouldFireNow) {
    if (i >= this->size || this->items[i].is_used) {
        return false;
    }

    this->items[i].cb = f;
    this->items[i].cb_arg = arg;
    this->items[i].flag = shouldFireNow;
    this->items[i].period = period;
    this->items[i].is_used = true;

    enable(i);

    return true;
}

bool TickerScheduler::remove(size_t i) {
    if (i >= this->size || !this->items[i].is_used) {
        return false;
    }

    this->items[i].is_used = false;
    this->items[i].t.detach();
    this->items[i].flag = false;
    this->items[i].cb = NULL;

    return true;
}

bool TickerScheduler::disable(size_t i) {
    if (i >= this->size || !this->items[i].is_used) {
        return false;
    }

    this->items[i].t.detach();

    return true;
}

bool TickerScheduler::enable(size_t i) {
    if (i >= this->size || !this->items[i].is_used) {
        return false;
    }

    bool* flag = &this->items[i].flag;
    this->items[i].t.attach_ms(this->items[i].period, TickerScheduler::handleTickerFlag, flag);

    return true;
}

void TickerScheduler::disableAll() {
    for (size_t i = 0; i < this->size; i++) {
        disable(i);
    }
}

void TickerScheduler::enableAll() {
    for (size_t i = 0; i < this->size; i++) {
        enable(i);
    }
}

void TickerScheduler::update() {
    unsigned long loop_start = micros();
    for (size_t i = 0; i < this->size; i++) {
        if (this->items[i].is_used) {
            unsigned long ticket_start = micros();

            handleTicker(this->items[i].cb, this->items[i].cb_arg, &this->items[i].flag);

            this->items[i].metric.add(micros() - ticket_start);
        }
    }
    _total_mu += (micros() - loop_start);
    _loop_cnt++;
}

void TickerScheduler::print(Print& p) {
    if (!_total_mu || !_loop_cnt) {
        return;
    }
    unsigned long long total_ms = _total_mu / 1000;
    p.printf("loops: %llu total: %llu ms ", _loop_cnt, total_ms);
    for (size_t i = 0; i < size; i++) {
        Timing* tt = &items[i].metric;
        float per = (float)(tt->_total_mu / 1000) / total_ms * 100;
        if (!per) {
            continue;
        }
        p.printf("%d: %2.2f%%%s", i, per, i < size - 1 ? ", " : "\n");
    }
}

void TickerScheduler::reset() {
    for (size_t i = 0; i < this->size; i++) {
        this->items[i].metric.reset();
    }
    _total_mu = 0;
    _loop_cnt = 0;
}