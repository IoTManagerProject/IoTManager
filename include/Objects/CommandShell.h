#pragma once

#include <Arduino.h>

#include "Base/CircularBuffer.h"
#include "Objects/Terminal.h"
#include "Objects/Runner.h"

class CommandShell {
   public:
    CommandShell(Runner *runner);

    void setTerm(Terminal *term);
    Terminal *term();

    void showGreetings(bool = true);
    void showFarewell(bool = true);

    void clearHistory();
    void addHistory(const char *);
    bool getHistoryInput(String &);
    void setEditLine(const String &);
    bool active();
    void loop();

   private:
    size_t printGreetings(Print *);
    size_t printFarewell(Print *);
    size_t printPrompt(Print *);

    void onOpen(Print *out);
    void onClose(Print *out);
    void onData(const char *);
    void onHistory(Print *out);
    bool getLastInput(String &);

   private:
    CircularBuffer<String, 4> _history;
    Terminal *_term;
    Runner *_runner;
    String _path;
    bool _active;
    bool _greetings;
    bool _farewell;
};
