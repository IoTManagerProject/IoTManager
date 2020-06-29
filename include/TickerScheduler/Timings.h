#pragma once

#include <Arduino.h>

struct Timing {
    unsigned long _total_mu;
    unsigned long _min_mu;
    unsigned long _max_mu;

    Timing() : _total_mu{0}, _min_mu{0}, _max_mu{0} {};

    void add(unsigned long time_mu) {
        if (!time_mu) {
            return;
        }
        _total_mu += time_mu;
        if (!_min_mu || (_min_mu > time_mu)) {
            _min_mu = time_mu;
        }
        if (_max_mu < time_mu) {
            _max_mu = time_mu;
        }
    }

    void reset() {
        _total_mu = 0;
        _min_mu = 0;
        _max_mu = 0;
    }
};

