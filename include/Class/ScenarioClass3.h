#pragma once
#include <Arduino.h>
#include "Cmd.h"
#include "Global.h"

class Scenario {

public:

    void loop() {
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

            String setEventKey = selectFromMarkerToMarker(condition, " ", 0);

            if (incommingEventKey == setEventKey) {

                String setEventSign = selectFromMarkerToMarker(condition, " ", 1);
                String setEventValue = selectFromMarkerToMarker(condition, " ", 2);

                if (!isDigitStr(setEventValue)) {
                    if (setEventValue.indexOf("+-") != -1) {
                        String setEventValueName = selectToMarker(setEventValue, "+-");
                        String gisteresisValue = selectToMarkerLast(setEventValue, "+-");
                        gisteresisValue.replace("+-", "");
                        String value = getValue(setEventValueName);
                        String upValue = String(value.toFloat() + gisteresisValue.toFloat());
                        String lowValue = String(value.toFloat() - gisteresisValue.toFloat());

                        if (setEventSign == ">") {
                            setEventValue = upValue;
                        }
                        else if (setEventSign == "<") {
                            setEventValue = lowValue;

                        }
                    } 
                    else {
                        setEventValue = getValue(setEventValue);
                    }
                }

                boolean flag = false;

                if (setEventSign == "=") {
                    flag = incommingEventValue == setEventValue;
                }
                else if (setEventSign == "!=") {
                    flag = incommingEventValue != setEventValue;
                }
                else if (setEventSign == "<") {
                    flag = incommingEventValue.toFloat() < setEventValue.toFloat();
                }
                else if (setEventSign == ">") {
                    flag = incommingEventValue.toFloat() > setEventValue.toFloat();
                }
                else if (setEventSign == ">=") {
                    flag = incommingEventValue.toFloat() >= setEventValue.toFloat();
                }
                else if (setEventSign == "<=") {
                    flag = incommingEventValue.toFloat() <= setEventValue.toFloat();
                }

                if (flag) {
                    //SerialPrint("I", "Scenario", "incomming Event Value: " + incommingEventValue);
                    //SerialPrint("I", "Scenario", "set Event Value: " + setEventValue);

                    oneBlock = deleteBeforeDelimiter(oneBlock, "\n");
                    oneBlock.replace("end", "");
                    SerialPrint("I", "Scenario", condition + " \n" + oneBlock);
                    spaceCmdExecute(oneBlock);
                }
            }
            allBlocks = deleteBeforeDelimiter(allBlocks, "end\n");
        }
        eventBuf = deleteBeforeDelimiter(eventBuf, ",");
    }
};

extern Scenario* myScenario;

extern void streamEventUDP(String event);