#pragma once
#include <Arduino.h>

#include "Cmd.h"
#include "Global.h"

class Scenario {
   protected:
    String _scenarioTmp;

    String _condition;
    String _conditionParam;
    String _conditionSign;
    String _conditionValue;

    String _scenBlok;

    String _event;
    String _eventParam;
    String _eventValue;

   public:
    Scenario() : _scenarioTmp{""},

                 _condition{""},
                 _conditionParam{""},
                 _conditionSign{""},
                 _conditionValue{""},

                 _scenBlok{""},

                 _event{""},
                 _eventParam{""},
                 _eventValue{""}

                 {};

    void load(String& scenario) {
        _scenarioTmp = scenario;
    }

    void calculate() {
        _scenarioTmp += "\n";
        _scenarioTmp.replace("\r\n", "\n");
        _scenarioTmp.replace("\r", "\n");

        _scenBlok = selectToMarker(_scenarioTmp, "end");
        _condition = selectToMarker(_scenBlok, "\n");
        _conditionParam = selectFromMarkerToMarker(_condition, " ", 0);
        _conditionSign = selectFromMarkerToMarker(_condition, " ", 1);
        _conditionValue = selectFromMarkerToMarker(_condition, " ", 2);
        _eventValue = jsonReadStr(configLiveJson, _conditionParam);
    }

    void delOneScenBlock() {
        _scenarioTmp = deleteBeforeDelimiter(_scenarioTmp, "end\n");
        //Serial.println(_scenarioTmp);
    }

    void delOneEvent() {
        eventBuf = deleteBeforeDelimiter(eventBuf, ",");
    }

    bool checkIncommingEvent() {
        bool ret = false;
        if (_conditionParam == _eventParam) ret = true;
        return ret;
    }

    bool compare() {
        boolean flag = false;

        if (_conditionSign == "=") {
            flag = _eventParam == _conditionValue;
        } else if (_conditionSign == "!=") {
            flag = _eventParam != _conditionValue;
        } else if (_conditionSign == "<") {
            flag = _eventParam.toInt() < _conditionValue.toInt();
        } else if (_conditionSign == ">") {
            flag = _eventParam.toInt() > _conditionValue.toInt();
        } else if (_conditionSign == ">=") {
            flag = _eventParam.toInt() >= _conditionValue.toInt();
        } else if (_conditionSign == "<=") {
            flag = _eventParam.toInt() <= _conditionValue.toInt();
        }

        return flag;
    }

    void loop() {
        this->load(scenario);  //после этого мы получили все сценарии
        //Serial.println("loaded");

        while (_scenarioTmp.length()) {
            
            this->calculate();  //получаем данные для первого блока

            if (this->checkIncommingEvent()) {
                if (this->compare()) {
                    _scenBlok = deleteBeforeDelimiter(_scenBlok, "\n");
                    spaceExecute(_scenBlok);
                    Serial.println(_scenBlok);
                }
            }

            this->delOneScenBlock();  //удалим использованный блок
        }
        this->delOneEvent();
    }
};
extern Scenario* myScenario;