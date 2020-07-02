#include "Base/Stateble.h"

Stateble::Stateble(int initial) {
    _state = initial;
}

void Stateble::setStateAsBool(bool value) {
    _state = value;
    onStateChange();
}

void Stateble::setState(int value) {
    _state = value;
    onStateChange();
}

void Stateble::toogleState() {
    _state = !_state;
    onStateChange();
}

int Stateble::getState() const {
    return _state;
}