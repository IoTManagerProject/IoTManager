#include "Global.h"

void handleScenario() {
    if (jsonReadStr(configSetupJson, "scen") == "1") {
        if ((jsonReadStr(configOptionJson, "scenario_status") != "")) {
            int i = 0;
            String str = scenario;  //читаем переменную с сценариями (то что из файла на странице)
            str += "\n";
            str.replace("\r\n", "\n");
            str.replace("\r", "\n");
            while (str.length() != 0) {
                //-----------------------------------------------------------------------------------------------------------------------
                String tmp = selectToMarker(str, "end");  //выделяем первый сценарий из файла вместе с командами
                if (tmp == "") return;
                i++;

                if (scenario_line_status[i] == 1) {
                    //Serial.println(i);
                    String condition = selectToMarker(tmp, "\n");  //выделяем первую строку самого сценария  button1 = 1 (условие)
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
                            tmp = deleteBeforeDelimiter(tmp, "\n");  //удаляем строку самого сценария оставляя только команды
                            stringExecution(tmp);                    //выполняем все команды

                            Serial.println("[SCENARIO] '" + condition + "'");
                            //Serial.println("            " + tmp);
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

void eventGen(String event_name, String number) {  //событие выглядит как имя плюс set плюс номер: button+Set+1

    if (jsonReadStr(configSetupJson, "scen") == "1") {
        String tmp = jsonReadStr(configOptionJson, "scenario_status");  //генерирование события
        //Serial.println(event_name);
        jsonWriteStr(configOptionJson, "scenario_status", tmp + event_name + number + ",");
    }
}

String add_set(String param_name) {
    String num1 = param_name.substring(param_name.length() - 1);
    String num2 = param_name.substring(param_name.length() - 2, param_name.length() - 1);
    if (isDigitStr(num1) && isDigitStr(num2)) {
        param_name = param_name.substring(0, param_name.length() - 2) + "Set" + num2 + num1;
    } else {
        if (isDigitStr(num1)) {
            param_name = param_name.substring(0, param_name.length() - 1) + "Set" + num1;
        }
    }
    return param_name;
}