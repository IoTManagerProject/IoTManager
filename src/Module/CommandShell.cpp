#include "Module/CommandShell.h"

#include "Config.h"

CommandShell::CommandShell(Runner *runner) : _term(nullptr), _runner(runner), _path('\\'), _active(false), _greetings(false), _farewell(false) {}

void CommandShell::setTerm(Terminal *term) {
    _term = term;
    if (_term != nullptr) {
        _term->setOnEvent([this](TerminalEventEnum event, Stream *out) {
            switch (event) {
                case EVENT_OPEN: {
                    onOpen(out);
                    break;
                }
                case EVENT_CLOSE:
                    onClose(out);
                    break;
                case EVENT_TAB:
                    onHistory(out);
                    break;
                default:
                    break;
            }
        });
        _term->setOnReadLine([this](const char *str) { onData(str); });
    }
}

Terminal *CommandShell::term() {
    return _term;
}

void CommandShell::loop() {
    if (_term != nullptr) _term->loop();
}

bool CommandShell::active() { return _active; }

void CommandShell::showGreetings(bool enabled) { _greetings = enabled; }

void CommandShell::showFarewell(bool enabled) { _farewell = enabled; }

void CommandShell::onOpen(Print *out) {
    if (_greetings) printGreetings(out);
    printPrompt(out);
    _active = true;
}

void CommandShell::onClose(Print *out) {
    out->println();
    if (_farewell) printFarewell(out);
    _active = false;
}

void CommandShell::onHistory(Print *out) {
    if (!_history.size()) return;

    if (_term->getLine().available()) {
        out->println();
        printPrompt(out);
    }
    String str;
    if (getLastInput(str)) {
        setEditLine(str);
    };
}

void CommandShell::onData(const char *str) {
    addHistory(str);

    _runner->run(str, _term);

    printPrompt(_term);
}

void CommandShell::clearHistory() { _history.reset(); }

void CommandShell::setEditLine(const String &str) {
    _term->setLine((const uint8_t *)str.c_str(), str.length());
}

bool CommandShell::getLastInput(String &str) {
    return _history.pop_back(str);
}

void CommandShell::addHistory(const char *str) {
    String buf;
    if (str == NULL || !strlen(str) || (_history.peek(buf) && buf.equals(str)))
        return;
    _history.push(str);
}

size_t CommandShell::printGreetings(Print *p) {
    return p->println(F("Hello"));
}

size_t CommandShell::printFarewell(Print *p) {
    return p->println(F("Bye-bye"));
}

size_t CommandShell::printPrompt(Print *p) {
    size_t n = 0;
    n += p->print(config.general()->getBroadcastName().c_str());
    n += p->print('>');
    n += p->print(' ');
    return n;
}