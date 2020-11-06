#pragma once
#include <Arduino.h>
#include "Cmd.h"
#include "Global.h"

class Scenario {

public:

    void loop() {

        String allBlocks = scenario;
        allBlocks.replace("\r\n", "\n");
        allBlocks.replace("\r", "\n");
        allBlocks += "\n";

        String incommingEvent = selectToMarker(eventBuf, ",");

        while (allBlocks.length() > 1) {
            String oneBlock = selectToMarker(allBlocks, "end\n");
            String condition = selectToMarker(oneBlock, "\n");

            String setEvent = selectFromMarkerToMarker(condition, " ", 0);
            String setEventSign = selectFromMarkerToMarker(condition, " ", 1);
            String setEventValue = selectFromMarkerToMarker(condition, " ", 2);
            if (!isDigitStr(setEventValue)) setEventValue = jsonReadStr(configLiveJson, setEventValue);
            
            String incommingEventValue = jsonReadStr(configLiveJson, incommingEvent);

            if (incommingEvent == setEvent) {

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
                    SerialPrint("I", "Scenario", condition + " set:\n" + oneBlock);
                    spaceCmdExecute(oneBlock);
                }
            }
            allBlocks = deleteBeforeDelimiter(allBlocks, "end\n");
        }
        eventBuf = deleteBeforeDelimiter(eventBuf, ",");
    }
};

extern Scenario* myScenario;