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

    void load() {
        _scenarioTmp = scenario;
    }

    //button-out1 = 1
    //button-out2 1
    //button-out3 1
    //end

    void calculate() {
        _scenarioTmp += "\n";
        _scenarioTmp.replace("\r\n", "\n");
        _scenarioTmp.replace("\r", "\n");

        _scenBlok = selectToMarker(_scenarioTmp, "end\n");

        _condition = selectToMarker(_scenBlok, "\n");  //button-out1 = 1

        _conditionParam = selectFromMarkerToMarker(_condition, " ", 0);  //button-out1
        _conditionSign = selectFromMarkerToMarker(_condition, " ", 1);   //=
        _conditionValue = selectFromMarkerToMarker(_condition, " ", 2);  //1

        if (!isDigitStr(_conditionValue)) _conditionValue = jsonReadStr(configLiveJson, _conditionValue);

        _eventParam = selectToMarker(eventBuf, ",");
        _eventValue = jsonReadStr(configLiveJson, _conditionParam);
    }

    void delOneScenBlock() {
        _scenarioTmp = deleteBeforeDelimiter(_scenarioTmp, "end\n");
        //Serial.println(_scenarioTmp);
    }

    void delOneEvent() {
        eventBuf = deleteBeforeDelimiter(eventBuf, ",");
    }

    bool isIncommingEventInScenario() {
        bool ret = false;
        if (_conditionParam == _eventParam) {
            ret = true;
            //Serial.println("true");
        }
        return ret;
    }

    bool isConditionSatisfied() {
        boolean flag = false;

        if (_conditionSign == "=") {
            flag = _eventValue == _conditionValue;
        } else if (_conditionSign == "!=") {
            flag = _eventValue != _conditionValue;
        } else if (_conditionSign == "<") {
            flag = _eventValue.toInt() < _conditionValue.toInt();
        } else if (_conditionSign == ">") {
            flag = _eventValue.toInt() > _conditionValue.toInt();
        } else if (_conditionSign == ">=") {
            flag = _eventValue.toInt() >= _conditionValue.toInt();
        } else if (_conditionSign == "<=") {
            flag = _eventValue.toInt() <= _conditionValue.toInt();
        }

        if (flag) Serial.println("[I] Scenario event: " + _condition);

        return flag;
    }

    void loop() {
        this->load();  //после этого мы получили все сценарии
        while (_scenarioTmp.length() > 1) {
            this->calculate();                         //получаем данные для первого блока
            if (this->isIncommingEventInScenario()) {  //если вошедшее событие есть в сценарии
                if (this->isConditionSatisfied()) {    //если вошедшее событие выполняет условие сценария
                    _scenBlok = deleteBeforeDelimiter(_scenBlok, "\n");
                    Serial.println("[>] Making: " + _scenBlok);
                    spaceExecute(_scenBlok);
                }
            }
            this->delOneScenBlock();  //удалим использованный блок
        }
        this->delOneEvent();
    }
};
extern Scenario* myScenario;