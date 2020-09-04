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
                 _eventValue{""} {};

    void load() {
        _scenarioTmp = scenario;
    }

    void calculate1() {
        _scenBlok = selectToMarker(_scenarioTmp, "end\n");
        _condition = selectToMarker(_scenBlok, "\n");
        _eventParam = selectToMarker(eventBuf, ",");
    }

    bool isIncommingEventInScenario() {
        bool ret = false;
        if (_condition.indexOf(_eventParam) != -1) {
            ret = true;
        }
        return ret;
    }

    void calculate2() {
        _scenarioTmp += "\n";
        _scenarioTmp.replace("\r\n", "\n");
        _scenarioTmp.replace("\r", "\n");

        _conditionParam = selectFromMarkerToMarker(_condition, " ", 0);
        _conditionSign = selectFromMarkerToMarker(_condition, " ", 1);
        _conditionValue = selectFromMarkerToMarker(_condition, " ", 2);
        if (!isDigitStr(_conditionValue)) _conditionValue = jsonReadStr(configLiveJson, _conditionValue);
        _eventValue = jsonReadStr(configLiveJson, _conditionParam);
    }

    void delOneScenBlock() {
        _scenarioTmp = deleteBeforeDelimiter(_scenarioTmp, "end\n");
    }

    void delOneEvent() {
        eventBuf = deleteBeforeDelimiter(eventBuf, ",");
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
        if (!this->isScenarioEnabled()) {
            return;
        }
        this->load();  //после этого мы получили все сценарии
        while (_scenarioTmp.length() > 1) {
            this->calculate1(); //расчет необходимый для ответа на следующий вопрос
            if (this->isIncommingEventInScenario()) {  //если вошедшее событие есть в сценарии
                this->calculate2();
                if (this->isConditionSatisfied()) {  //если вошедшее событие выполняет условие сценария
                    _scenBlok = deleteBeforeDelimiter(_scenBlok, "\n");
                    //Serial.println("   [>] Making: " + _scenBlok);
                    spaceCmdExecute(_scenBlok);
                }
            }
            this->delOneScenBlock();  //удалим использованный блок
        }
        this->delOneEvent();
    }

    bool isScenarioEnabled() {
        return jsonReadBool(configSetupJson, "scen") && eventBuf != "";
    }
};
extern Scenario* myScenario;