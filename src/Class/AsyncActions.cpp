#include "Class/AsyncActions.h"

AsyncActions::AsyncActions(){};

void AsyncActions::loop() {
    count++;
    if (count > 5000) {
        if(_cb) _cb;       //что означает эта запись? это и есть вызов callback?
        count = 0;
    }
}

AsyncActions* async;