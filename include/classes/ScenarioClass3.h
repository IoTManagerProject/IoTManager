#pragma once
#include "Global.h"
#include "MqttClient.h"
#include "Buffers.h"

class Scenario {
   public:
    void loop2() {
        if (!jsonReadBool(settingsFlashJson, "scen")) {
            return;
        }
        String allBlocks = scenario;
        allBlocks += "\n";

        String incommingEvent = selectToMarker(eventBuf, ",");

        String incommingEventKey = selectToMarker(incommingEvent, " ");
        String incommingEventValue = selectToMarkerLast(incommingEvent, " ");

        while (allBlocks.length()) {
            String oneBlock = selectToMarker(allBlocks, "end\n");
            String condition = selectToMarker(oneBlock, "\n");

            //логическое и
            if (condition.indexOf("&&") != -1) {
                condition = condition += " && ";

                //посчитаем количество условий
                int conditionCnt = itemsCount2(condition, "&&") - 1;

                //создадим и заполним динамический массив
                bool *arr = new bool[conditionCnt];
                for (int i = 0; i < conditionCnt; i++) {
                    arr[i] = false;
                }

                //есть ли входящее событие хотя бы в одном из условий и удавлетварено ли оно?
                int evenInConditionNum = -1;
                for (int i = 0; i < conditionCnt; i++) {
                    String buf = selectFromMarkerToMarker(condition, " && ", i);
                    if (isScenarioNeedToDo(buf, incommingEventKey, incommingEventValue, 1)) {
                        arr[i] = true;
                        evenInConditionNum = i;
                    }
                }

                //если да то проверяем остальные условия по json
                if (evenInConditionNum >= 0) {
                    for (int i = 0; i < conditionCnt; i++) {
                        String buf = selectFromMarkerToMarker(condition, " && ", i);
                        if (i != evenInConditionNum) {
                            if (isScenarioNeedToDoJson(buf)) {
                                arr[i] = true;
                            }
                        }
                    }
                }

                //все элементы массива должны быть true
                bool result = true;
                for (int i = 0; i < conditionCnt; i++) {
                    if (!arr[i]) {
                        result = false;
                    }
                }

                delete[] arr;

                if (result) {
                    oneBlock = deleteBeforeDelimiter(oneBlock, "\n");
                    oneBlock.replace("end", "");

                    // SerialPrint('I', "Event done", incommingEvent);
                SerialPrint('I', M_SCENARIO, F("All conditions are matched"));
                    spaceCmdExecute(oneBlock);
                }

                //логическое или
            } else if (condition.indexOf("||") != -1) {
                condition = condition += " || ";

                //посчитаем количество условий
                int conditionCnt = itemsCount2(condition, "||") - 1;

                //создадим и заполним динамический массив
                bool *arr = new bool[conditionCnt];
                for (int i = 0; i < conditionCnt; i++) {
                    arr[i] = false;
                }

                //есть ли входящее событие хотя бы в одном из условий и удавлетварено ли оно?
                int evenInConditionNum = -1;
                for (int i = 0; i < conditionCnt; i++) {
                    String buf = selectFromMarkerToMarker(condition, " || ", i);
                    if (isScenarioNeedToDo(buf, incommingEventKey, incommingEventValue, 1)) {
                        arr[i] = true;
                        evenInConditionNum = i;
                    }
                }

                //если да то проверяем остальные условия по json
                if (evenInConditionNum >= 0) {
                    for (int i = 0; i < conditionCnt; i++) {
                        String buf = selectFromMarkerToMarker(condition, " || ", i);
                        if (i != evenInConditionNum) {
                            if (isScenarioNeedToDoJson(buf)) {
                                arr[i] = true;
                            }
                        }
                    }
                }

                //хотя бы один элемент массива должн быть true
                bool result = false;
                for (int i = 0; i < conditionCnt; i++) {
                    if (arr[i]) {
                        result = true;
                    }
                }

                delete[] arr;

                if (result) {
                    oneBlock = deleteBeforeDelimiter(oneBlock, "\n");
                    oneBlock.replace("end", "");

                    // SerialPrint('I', "Event done", incommingEvent);
                SerialPrint('I', M_SCENARIO, F("One of all condition are matched"));
                    spaceCmdExecute(oneBlock);
                }

                //если гистерезис
            } else if (condition.indexOf("+-") != -1) {
                if (isScenarioNeedToDo(condition, incommingEventKey, incommingEventValue, 2)) {
                    oneBlock = deleteBeforeDelimiter(oneBlock, "\n");
                    oneBlock.replace("end", "");

                    // SerialPrint('I', "Event done", incommingEvent);
                SerialPrint('I', M_SCENARIO, "Condition are matched: " + condition);
                    spaceCmdExecute(oneBlock);
                }

                //остальные случаи
            } else {
                if (isScenarioNeedToDo(condition, incommingEventKey, incommingEventValue, 1)) {
                    oneBlock = deleteBeforeDelimiter(oneBlock, "\n");
                    oneBlock.replace("end", "");

                    // SerialPrint('I', "Event done", incommingEvent);
                SerialPrint('I', M_SCENARIO, "Condition are matched: " + condition);
                    spaceCmdExecute(oneBlock);
                }
            }
            allBlocks = deleteBeforeDelimiter(allBlocks, "end\n");
        }
        eventBuf = deleteBeforeDelimiter(eventBuf, ",");
    }

   private:
    bool isScenarioNeedToDo(String &condition, String &incommingEventKey, String &incommingEventValue, int type) {
        bool res = false;
        String setEventKey = selectFromMarkerToMarker(condition, " ", 0);
        if (isEventExist(incommingEventKey, setEventKey)) {
            String setEventSign;
            String setEventValue;
            if (type == 1) preCalculation(condition, setEventSign, setEventValue);
            if (type == 2) preCalculationGisteresis(condition, setEventSign, setEventValue);
            if (isConditionMatch(setEventSign, incommingEventValue, setEventValue)) {
                res = true;
            }
        }
        return res;
    }

    bool isScenarioNeedToDoJson(String &condition) {
        bool res = false;
        String setEventKey = selectFromMarkerToMarker(condition, " ", 0);
        String setEventSign;
        String setEventValue;
        preCalculation(condition, setEventSign, setEventValue);
        String jsonValue = getValueJson(setEventKey);
        if (isConditionMatch(setEventSign, jsonValue, setEventValue)) {
            res = true;
        }
        return res;
    }

    // bool isScenarioNeedToDoJson2(String &condition, String &incommingEventKey, String &incommingEventValue) {
    //     bool res = false;
    //     String setEventKey = selectFromMarkerToMarker(condition, " ", 0);
    //     if (isEventExist(incommingEventKey, setEventKey)) {
    //         String setEventSign;
    //         String setEventValue;
    //         preCalculation(condition, setEventSign, setEventValue);
    //         if (isConditionMatch(setEventSign, incommingEventValue, setEventValue)) {
    //             res = true;
    //         }
    //     }
    //     return res;
    // }

    void preCalculation(String &condition, String &setEventSign, String &setEventValue) {
        setEventSign = selectFromMarkerToMarker(condition, " ", 1);
        setEventValue = selectFromMarkerToMarker(condition, " ", 2);
        if (!isDigitDotCommaStr(setEventValue)) {
            setEventValue = getValueJson(setEventValue);
        }
    }

    void preCalculationGisteresis(String &condition, String &setEventSign, String &setEventValue) {
        setEventSign = selectFromMarkerToMarker(condition, " ", 1);
        setEventValue = selectFromMarkerToMarker(condition, " ", 2);
        if (!isDigitDotCommaStr(setEventValue)) {
            String setEventValueName = selectToMarker(setEventValue, "+-");
            String gisteresisValue = selectToMarkerLast(setEventValue, "+-");
            gisteresisValue.replace("+-", "");
            String value = getValueJson(setEventValueName);
            String upValue = String(value.toFloat() + gisteresisValue.toFloat());
            String lowValue = String(value.toFloat() - gisteresisValue.toFloat());
            if (setEventSign == ">") {
                setEventValue = upValue;
            } else if (setEventSign == "<") {
                setEventValue = lowValue;
            }
        }
    }

    bool isEventExist(String &incommingEventKey, String &setEventKey) {
        bool res = false;
        if (incommingEventKey == setEventKey) {
            res = true;
        }
        return res;
    }

    bool isConditionMatch(String &setEventSign, String &incommingEventValue, String &setEventValue) {
        boolean flag = false;
        if (setEventSign == "=") {
            flag = incommingEventValue == setEventValue;
        } else if (setEventSign == "!=") {
            flag = incommingEventValue != setEventValue;
        } else if (setEventSign == "<") {
            flag = incommingEventValue.toFloat() < setEventValue.toFloat();
        } else if (setEventSign == ">") {
            flag = incommingEventValue.toFloat() > setEventValue.toFloat();
        } else if (setEventSign == ">=") {
            flag = incommingEventValue.toFloat() >= setEventValue.toFloat();
        } else if (setEventSign == "<=") {
            flag = incommingEventValue.toFloat() <= setEventValue.toFloat();
        }
        return flag;
    }
};

extern Scenario *myScenario;
