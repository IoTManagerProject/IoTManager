#pragma once
#include <Arduino.h>

#include "Cmd.h"
#include "Global.h"

class Scenario {
   public:
    //void loop() {
    //    if (!jsonReadBool(configSetupJson, "scen")) {
    //        return;
    //    }
    //    String allBlocks = scenario;
    //    allBlocks.replace("\r\n", "\n");
    //    allBlocks.replace("\r", "\n");
    //    allBlocks += "\n";
//
    //    String incommingEvent = selectToMarker(eventBuf, ",");
    //    String incommingEventKey = selectToMarker(incommingEvent, " ");
    //    String incommingEventValue = selectToMarkerLast(incommingEvent, " ");
//
    //    while (allBlocks.length() > 1) {
    //        String oneBlock = selectToMarker(allBlocks, "end\n");
    //        String condition = selectToMarker(oneBlock, "\n");
    //        if (condition.indexOf("&&") != -1) {  //если двойное условие
    //            String condition1 = selectFromMarkerToMarker(condition, " && ", 0);
    //            String condition2 = selectFromMarkerToMarker(condition, " && ", 1);
//
    //            //если хотя бы одно условие совпало с этим событием то второе тянем из json!!!!!!!!!!!!!!!!!!!!!!!!!
    //            if (isScenarioNeedToDo(condition1, incommingEventKey, incommingEventValue, 1)) {
    //                if (isScenarioNeedToDoJson(condition2)) {
    //                    oneBlock = deleteBeforeDelimiter(oneBlock, "\n");
    //                    oneBlock.replace("end", "");
    //                    SerialPrint("I", "Scenario", condition1 + " && " + condition2 + " \n" + oneBlock);
    //                    spaceCmdExecute(oneBlock);
    //                }
    //            } else if (isScenarioNeedToDo(condition2, incommingEventKey, incommingEventValue, 1)) {
    //                if (isScenarioNeedToDoJson(condition1)) {
    //                    oneBlock = deleteBeforeDelimiter(oneBlock, "\n");
    //                    oneBlock.replace("end", "");
    //                    SerialPrint("I", "Scenario", condition1 + " && " + condition2 + " \n" + oneBlock);
    //                    spaceCmdExecute(oneBlock);
    //                }
    //            }
    //        } else if (condition.indexOf("+-") != -1) {  //если гистерезис
    //            if (isScenarioNeedToDo(condition, incommingEventKey, incommingEventValue, 2)) {
    //                oneBlock = deleteBeforeDelimiter(oneBlock, "\n");
    //                oneBlock.replace("end", "");
    //                SerialPrint("I", "Scenario", condition + " \n" + oneBlock);
    //                spaceCmdExecute(oneBlock);
    //            }
    //        } else {  //остальные случаи
    //            if (isScenarioNeedToDo(condition, incommingEventKey, incommingEventValue, 1)) {
    //                oneBlock = deleteBeforeDelimiter(oneBlock, "\n");
    //                oneBlock.replace("end", "");
    //                SerialPrint("I", "Scenario", condition + " \n" + oneBlock);
    //                spaceCmdExecute(oneBlock);
    //            }
    //        }
    //        allBlocks = deleteBeforeDelimiter(allBlocks, "end\n");
    //        eventBuf = deleteBeforeDelimiter(eventBuf, ",");
    //    }
    //}

    void loop2() {
        if (!jsonReadBool(configSetupJson, "scen")) {
            return;
        }
        String allBlocks = scenario;
        allBlocks.replace("\r\n", "\n");
        allBlocks.replace("\r", "\n");
        allBlocks += "\n";

        String incommingEvent = selectToMarker(eventBuf, ",");
        String incommingEventKey = selectToMarker(incommingEvent, " ");
        String incommingEventValue = selectToMarkerLast(incommingEvent, " ");

        while (allBlocks.length() > 1) {
            String oneBlock = selectToMarker(allBlocks, "end\n");
            String condition = selectToMarker(oneBlock, "\n");
            if (condition.indexOf("&&") != -1) {  //если двойное условие
                condition = condition += " && ";
                int conditionCnt = itemsCount2(condition, "&&") - 1;  //посчитаем количество условий

                bool *arr = new bool[conditionCnt];
                for (int i = 0; i < conditionCnt; i++) {
                    arr[i] = false;
                }

                //есть ли входящее событие хотя бы одном из условий и удавлетварено ли оно?
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

                bool result = true;
                for (int i = 0; i < conditionCnt; i++) {
                    if (!arr[i]) {
                        result = false;
                    }
                }

                delete [] arr;

                if (result) {
                    oneBlock = deleteBeforeDelimiter(oneBlock, "\n");
                    oneBlock.replace("end", "");
                    SerialPrint("I", "Scenario", "multiconditions action \n" + oneBlock);
                    spaceCmdExecute(oneBlock);
                }
            } else if (condition.indexOf("+-") != -1) {  //если гистерезис
                if (isScenarioNeedToDo(condition, incommingEventKey, incommingEventValue, 2)) {
                    oneBlock = deleteBeforeDelimiter(oneBlock, "\n");
                    oneBlock.replace("end", "");
                    SerialPrint("I", "Scenario", condition + " \n" + oneBlock);
                    spaceCmdExecute(oneBlock);
                }
            } else {  //остальные случаи
                if (isScenarioNeedToDo(condition, incommingEventKey, incommingEventValue, 1)) {
                    oneBlock = deleteBeforeDelimiter(oneBlock, "\n");
                    oneBlock.replace("end", "");
                    SerialPrint("I", "Scenario", condition + " \n" + oneBlock);
                    spaceCmdExecute(oneBlock);
                }
            }
            allBlocks = deleteBeforeDelimiter(allBlocks, "end\n");
            eventBuf = deleteBeforeDelimiter(eventBuf, ",");
        }
    }

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
        String jsonValue = getValue(setEventKey);
        if (isConditionMatch(setEventSign, jsonValue, setEventValue)) {
            res = true;
        }
        return res;
    }

    bool isScenarioNeedToDoJson2(String &condition, String &incommingEventKey, String &incommingEventValue) {
        bool res = false;
        String setEventKey = selectFromMarkerToMarker(condition, " ", 0);
        if (isEventExist(incommingEventKey, setEventKey)) {
            String setEventSign;
            String setEventValue;
            preCalculation(condition, setEventSign, setEventValue);
            if (isConditionMatch(setEventSign, incommingEventValue, setEventValue)) {
                res = true;
            }
        }
        return res;
    }

    void preCalculation(String &condition, String &setEventSign, String &setEventValue) {
        setEventSign = selectFromMarkerToMarker(condition, " ", 1);
        setEventValue = selectFromMarkerToMarker(condition, " ", 2);
        if (!isDigitDotCommaStr(setEventValue)) {
            setEventValue = getValue(setEventValue);
        }
    }

    void preCalculationGisteresis(String &condition, String &setEventSign, String &setEventValue) {
        setEventSign = selectFromMarkerToMarker(condition, " ", 1);
        setEventValue = selectFromMarkerToMarker(condition, " ", 2);
        if (!isDigitDotCommaStr(setEventValue)) {
            String setEventValueName = selectToMarker(setEventValue, "+-");
            String gisteresisValue = selectToMarkerLast(setEventValue, "+-");
            gisteresisValue.replace("+-", "");
            String value = getValue(setEventValueName);
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
extern void streamEventUDP(String event);