#include "Global.h"

static const char* MODULE = "Scen";

void loopScenario() {
    bool enabled = jsonReadBool(configSetupJson, "scen");
    if (enabled) {
        if ((jsonReadStr(configOptionJson, "scenario_status") != "")) {
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

                        String sign = selectFromMarkerToMarker(condition, " ", 1);   //читаем знак  (=)
                        String value = selectFromMarkerToMarker(condition, " ", 2);  //читаем значение (1)
                        if (value.indexOf("digit") != -1) {
                            //  value = add_set(value);
                            value = jsonReadStr(configLiveJson, value);
                        }
                        if (value.indexOf("time") != -1) {
                            //  value = add_set(value);
                            value = jsonReadStr(configLiveJson, value);
                        }
                        boolean flag = false;  //если одно из значений совпало то только тогда начинаем выполнять комнады
                        if (sign == "=") {
                            if (jsonReadStr(configLiveJson, param_name) == value) flag = true;
                        }
                        if (sign == "!=") {
                            if (jsonReadStr(configLiveJson, param_name) != value) flag = true;
                        }
                        if (sign == "<") {
                            if (jsonReadStr(configLiveJson, param_name).toInt() < value.toInt()) flag = true;
                        }
                        if (sign == ">") {
                            if (jsonReadStr(configLiveJson, param_name).toInt() > value.toInt()) flag = true;
                        }
                        if (sign == ">=") {
                            if (jsonReadStr(configLiveJson, param_name).toInt() >= value.toInt()) flag = true;
                        }
                        if (sign == "<=") {
                            if (jsonReadStr(configLiveJson, param_name).toInt() <= value.toInt()) flag = true;
                        }

                        if (flag) {
                            block = deleteBeforeDelimiter(block, "\n");  //удаляем строку самого сценария оставляя только команды
                            stringExecute(block);                        //выполняем все команды

                            pm.info(condition + "'");
                        }
                    }
                }
                str = deleteBeforeDelimiter(str, "end\n");  //удаляем первый сценарий
                                                            //-----------------------------------------------------------------------------------------------------------------------
            }
            String tmp2 = jsonReadStr(configOptionJson, "scenario_status");  //читаем файл событий
            tmp2 = deleteBeforeDelimiter(tmp2, ",");                         //удаляем выполненное событие
            jsonWriteStr(configOptionJson, "scenario_status", tmp2);         //записываем обновленный файл событий
            i = 0;
        }
    }
}

//событие выглядит как имя плюс set плюс номер: button+Set+1
void eventGen(String event_name, String number) {
    if (jsonReadStr(configSetupJson, "scen") == "1") {
        String tmp = jsonReadStr(configOptionJson, "scenario_status");  //генерирование события
        jsonWriteStr(configOptionJson, "scenario_status", tmp + event_name + number + ",");
    }
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