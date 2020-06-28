#include "Global.h"

static const char* MODULE = "Scen";

boolean isScenarioEnabled() {
    return jsonReadBool(configSetupJson, "scen") && jsonReadStr(configOptionJson, "scenario_status") != "";
}

void loopScenario() {
    if (!isScenarioEnabled()) {
        return;
    }
    String str = scenario;
    str += "\n";
    str.replace("\r\n", "\n");
    str.replace("\r", "\n");

    size_t i = 0;
    while (str.length()) {
        String block = selectToMarker(str, "end");
        if (!block.length()) {
            return;
        }
        i++;
        if (scenario_line_status[i] == 1) {
            //выделяем первую строку самого сценария  button1 = 1 (условие)
            String condition = selectToMarker(block, "\n");
            String param_name = selectFromMarkerToMarker(condition, " ", 0);
            String order = jsonReadStr(configOptionJson, "scenario_status");  //читаем весь файл событий
            String param = selectToMarker(order, ",");                        //читаем первое событие из файла событий
            if (param_name == param) {                                        //если поступившее событие равно событию заданному buttonSet1 в файле начинаем его обработку
                String sign = selectFromMarkerToMarker(condition, " ", 1);    //читаем знак  (=)
                String value = selectFromMarkerToMarker(condition, " ", 2);   //читаем значение (1)
                if (value.indexOf("digit") != -1) {
                    //  value = add_set(value);
                    value = jsonReadStr(configLiveJson, value);
                }
                if (value.indexOf("time") != -1) {
                    //  value = add_set(value);
                    value = jsonReadStr(configLiveJson, value);
                }
                // если условие выполнилось, тогда начинаем выполнять комнады
                boolean flag = false;
                String param = jsonReadStr(configLiveJson, param_name);
                if (sign == "=") {
                    flag = param == value;
                } else if (sign == "!=") {
                    flag = param != value;
                } else if (sign == "<") {
                    flag = param.toInt() < value.toInt();
                } else if (sign == ">") {
                    flag = param.toInt() > value.toInt();
                } else if (sign == ">=") {
                    flag = param.toInt() >= value.toInt();
                } else if (sign == "<=") {
                    flag = param.toInt() <= value.toInt();
                }

                if (flag) {
                    // удаляем строку самого сценария оставляя только команды
                    block = deleteBeforeDelimiter(block, "\n");
                    pm.info("do: " + block);
                    // выполняем все команды
                    stringExecute(block);
                }
            }
        }
        str = deleteBeforeDelimiter(str, "end\n");  //удаляем первый сценарий
                                                    //-----------------------------------------------------------------------------------------------------------------------
    }
    String tmp2 = jsonReadStr(configOptionJson, "scenario_status");  //читаем файл событий
    tmp2 = deleteBeforeDelimiter(tmp2, ",");                         //удаляем выполненное событие
    jsonWriteStr(configOptionJson, "scenario_status", tmp2);         //записываем обновленный файл событий
}

// событие: имя + Set + номер
void eventGen(String event_name, String number) {
    if (!jsonReadBool(configSetupJson, "scen")) {
        return;
    }
    // генерирование события
    String tmp = jsonReadStr(configOptionJson, "scenario_status");
    jsonWriteStr(configOptionJson, "scenario_status", tmp + event_name + number + ",");
}

String add_set(String str) {
    String num1 = str.substring(str.length() - 1);
    String num2 = str.substring(str.length() - 2, str.length() - 1);
    if (isDigitStr(num1) && isDigitStr(num2)) {
        str = str.substring(0, str.length() - 2) + "Set" + num2 + num1;
    } else {
        if (isDigitStr(num1)) {
            str = str.substring(0, str.length() - 1) + "Set" + num1;
        }
    }
    return str;
}