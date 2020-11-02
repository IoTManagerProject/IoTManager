#include "Cmd.h"
#include "Global.h"
#include "Class/ScenarioClass.h"



boolean isScenarioEnabled() {
    return jsonReadBool(configSetupJson, "scen") && eventBuf != "";
}

//void loopScenario() {
//    if (!isScenarioEnabled()) {
//        return;
//    }
//    String scenarioTmp = scenario;
//    scenarioTmp += "\n";
//    scenarioTmp.replace("\r\n", "\n");
//    scenarioTmp.replace("\r", "\n");
//
//    while (scenarioTmp.length()) {
//        String scenBlok = selectToMarker(scenarioTmp, "end");  //выделяем первый сценарий
//        if (!scenBlok.length()) {
//            return;
//        }
//
//        size_t i = 0;
//        i++;
//        if (scenario_line_status[i] == 1) {
//            String condition = selectToMarker(scenBlok, "\n");  //выделяем условие
//
//            String conditionParam = selectFromMarkerToMarker(condition, " ", 0);  //выделяем параметр условия
//            String order = eventBuf;
//            String eventParam = selectToMarker(order, ",");  //выделяем параметр события
//
//            if (conditionParam == eventParam) {  //если поступившее событие равно событию заданному buttonSet1 в файле начинаем его обработку
//
//                String conditionSign = selectFromMarkerToMarker(condition, " ", 1);  //выделяем знак  (=)
//
//                String conditionValue = selectFromMarkerToMarker(condition, " ", 2);  //выделяем значение (1)
//
//                boolean flag = false;
//
//                String eventParam = jsonReadStr(configLiveJson, conditionParam);  //получаем значение этого параметра события из json
//
//                if (conditionSign == "=") {
//                    flag = eventParam == conditionValue;
//                } else if (conditionSign == "!=") {
//                    flag = eventParam != conditionValue;
//                } else if (conditionSign == "<") {
//                    flag = eventParam.toInt() < conditionValue.toInt();
//                } else if (conditionSign == ">") {
//                    flag = eventParam.toInt() > conditionValue.toInt();
//                } else if (conditionSign == ">=") {
//                    flag = eventParam.toInt() >= conditionValue.toInt();
//                } else if (conditionSign == "<=") {
//                    flag = eventParam.toInt() <= conditionValue.toInt();
//                }
//
//                if (flag) {
//                    scenBlok = deleteBeforeDelimiter(scenBlok, "\n");  // удаляем строку самого сценария оставляя только команды
//                    SerialPrint("I","module","do: " + scenBlok);
//                    spaceCmdExecute(scenBlok);  // выполняем все команды
//                }
//            }
//        }
//        scenarioTmp = deleteBeforeDelimiter(scenarioTmp, "end\n");  //удаляем первый сценарий
//    }
//
//    String eventBufTmp = eventBuf;                          //читаем файл событий
//    eventBufTmp = deleteBeforeDelimiter(eventBufTmp, ",");  //удаляем выполненное событие
//    eventBuf = eventBufTmp;                                 //записываем обновленный файл событий
//}

void eventGen(String event_name, String number) {
    if (!jsonReadBool(configSetupJson, "scen")) {
        return;
    }
    eventBuf = event_name + number + ",";
}

//String add_set(String str) {
//    String num1 = str.substring(str.length() - 1);
//    String num2 = str.substring(str.length() - 2, str.length() - 1);
//    if (isDigitStr(num1) && isDigitStr(num2)) {
//        str = str.substring(0, str.length() - 2) + "Set" + num2 + num1;
//    } else {
//        if (isDigitStr(num1)) {
//            str = str.substring(0, str.length() - 1) + "Set" + num1;
//        }
//    }
//    return str;
//}

//button-out1 = 1
//button-out2 1
//button-out3 1
//end