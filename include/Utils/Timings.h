#include <Arduino.h>

enum Timings_t { MT_ONE,
                 MT_TWO,
                 NUM_TIMINGS };

struct Timing {
    unsigned long _total_mu;
    unsigned long _min_mu;
    unsigned long _max_mu;

    Timing() : _total_mu{0}, _min_mu{999999}, _max_mu{0} {};

    void reset() {
        _total_mu = 0;
        _min_mu = 999999;
        _max_mu = 0;
    }

    void add(unsigned long time_mu) {
        if (time_mu == 0) return;

        _total_mu += time_mu;

        if (_min_mu > time_mu) {
            _min_mu = time_mu;
        }

        if (_max_mu < time_mu) {
            _max_mu = time_mu;
        }
    }
};

static const char* module_name[NUM_TIMINGS] = {"strings", "boolean"};

struct Timings {
    Timing mu[NUM_TIMINGS];

    unsigned long _counter;
    unsigned long _start;
    unsigned long long _total;

    Timings() : _counter{0}, _start{0} {};

    void add(size_t module, unsigned long now = micros()) {
        unsigned long time = now - _start;
        _total += time;
        mu[module].add(time);
        _start = now;
    }

    void count() {
        _counter++;
        _start = micros();
    }

    void print() {
        if (!_counter) {
            return;
        };
        Serial.printf("lp/ms: %llu ", _counter / _total);
        for (size_t i = 0; i < NUM_TIMINGS; i++) {
            Serial.printf("%s: %.2f%% ", module_name[i], ((float)mu[i]._total_mu / _total) * 100);
            mu[i].reset();
        }
        Serial.println();
        _counter = 0;
        _total = 0;
    };
};