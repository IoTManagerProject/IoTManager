#pragma once

#include <Arduino.h>

enum ModuleState_t {
    MOD_INIT,
    MOD_INIT_FAILED,
    MOD_INIT_COMPLETE,
    MOD_START_FAILED,
    MOD_ACTIVE
};

class Module {
   protected:
    virtual bool onInit() { return true; };
    virtual void onEnd(){};
    virtual bool onStart() { return true; }
    virtual void onStop(){};
    virtual void onLoop() = 0;

   protected:
    Print *_out;

   public:
    Module() : _state{MOD_INIT} {}

    bool init(bool force = false) {
        if (_state > MOD_INIT_COMPLETE) {
            return true;
        }
        if (_state == MOD_INIT_FAILED && !force) {
            return false;
        }

        _state = onInit() ? MOD_INIT_COMPLETE : MOD_INIT_FAILED;

        return _state == MOD_INIT_COMPLETE;
    }

    bool start(bool force = false) {
        if (_state == MOD_ACTIVE) {
            return true;
        }
        if (_state == MOD_START_FAILED && !force) {
            return false;
        }
        if (_state < MOD_INIT_COMPLETE) {
            if (!init(force)) {
                return false;
            }
        }
        _state = onStart() ? MOD_ACTIVE : MOD_START_FAILED;
        return _state == MOD_ACTIVE;
    }

    void stop() {
        if (_state < MOD_ACTIVE) {
            return;
        }
        onStop();
        _state = MOD_INIT_COMPLETE;
    };

    void end() {
        if (_state < MOD_INIT_FAILED) {
            return;
        }
        onEnd();
        _state = MOD_INIT;
    };

    void loop() {
        if (_state == MOD_ACTIVE || start()) onLoop();
    };

    void setOutput(Print *p) { _out = p; }

    ModuleState_t getState() {
        return _state;
    }

   private:
    ModuleState_t _state;
};
