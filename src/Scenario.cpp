#include "Scenario.h"

#include "Global.h"

static const char* MODULE = "Scen";

namespace Scenario {
static const size_t NUM_LINES = 40;

String _content = "";
bool _lines_enabled[NUM_LINES];

bool isEnabled(size_t num) {
    return _lines_enabled[num];
}

void enable(size_t num, boolean enable) {
    _lines_enabled[num] = enable;
}

void init() {
    load();
    memset(_lines_enabled, 1, sizeof(_lines_enabled[0]) * NUM_LINES);
}

void load() {
    _content = readFile(String(DEVICE_SCENARIO_FILE), 2048);
}

void process(EventQueue* queue) {
    if (!queue->available()) {
        return;
    }

    String event = queue->pop();
    if (event.isEmpty()) {
        return;
    }

    String scen = _content;
    scen += "\n";
    scen.replace("\r\n", "\n");
    scen.replace("\r", "\n");

    size_t line = 0;
    while (!scen.isEmpty()) {
        String block = selectToMarker(scen, "end");
        scen = deleteBeforeDelimiter(scen, "end\n");
        if (block.isEmpty()) {
            continue;
        }
        if (isEnabled(line++)) {
            //выделяем первую строку самого сценария  button1 = 1 (условие)
            String condition = selectToMarker(block, "\n");
            String obj = selectFromMarkerToMarker(condition, " ", 0);

            // если поступившее событие равно событию заданному buttonSet1
            if (obj == event) {
                String operation = selectFromMarkerToMarker(condition, " ", 1);  //читаем знак  (=)
                String value = selectFromMarkerToMarker(condition, " ", 2);      //читаем значение (1)

                if (value.indexOf("digit") != -1) {
                    value = readLiveData(value);
                }
                if (value.indexOf("time") != -1) {
                    value = readLiveData(value);
                }

                boolean flag = false;
                String param = readLiveData(obj);
                if (operation == "=") {
                    flag = param == value;
                } else if (operation == "!=") {
                    flag = param != value;
                } else if (operation == "<") {
                    flag = param.toInt() < value.toInt();
                } else if (operation == ">") {
                    flag = param.toInt() > value.toInt();
                } else if (operation == ">=") {
                    flag = param.toInt() >= value.toInt();
                } else if (operation == "<=") {
                    flag = param.toInt() <= value.toInt();
                }

                if (flag) {
                    // удаляем строку самого сценария оставляя только команды
                    block = deleteBeforeDelimiter(block, "\n");
                    // выполняем все команды
                    stringExecute(block);
                }
            }
        }
    }
}

}  // namespace Scenario