#include "Scenario.h"

#include "Global.h"
#include "Cmd.h"

static const char* MODULE = "Scen";

namespace Scenario {
static const size_t NUM_LINES = 40;

String _content = "";
bool _block_enabled[NUM_LINES];
bool _ready_flag = false;

bool isBlockEnabled(size_t num) {
    return _block_enabled[num];
}

void enableBlock(size_t num, boolean enable) {
    _block_enabled[num] = enable;
}

void reinit() {
    _ready_flag = false;
}

void init() {
    readFile(DEVICE_SCENARIO_FILE, _content);
    memset(_block_enabled, 1, sizeof(_block_enabled[0]) * NUM_LINES);
    _ready_flag = true;
}

void process(EventQueue* queue) {
    if (!_ready_flag) {
        init();
    }

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
        if (isBlockEnabled(line++)) {
            //выделяем первую строку самого сценария  button1 = 1 (условие)
            String condition = selectToMarker(block, "\n");
            String obj = selectFromMarkerToMarker(condition, " ", 0);

            // если поступившее событие равно событию заданному buttonSet1
            if (obj == event) {
                String operation = selectFromMarkerToMarker(condition, " ", 1);  //читаем знак  (=)
                String value = selectFromMarkerToMarker(condition, " ", 2);      //читаем значение (1)

                if (value.indexOf("digit") != -1) {
                    value = liveData.read(value);
                }
                if (value.indexOf("time") != -1) {
                    value = liveData.read(value);
                }

                boolean flag = false;
                String param = liveData.read(obj);
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