#pragma once

#include <Arduino.h>

#include "Dallas.h"

#include "Base/BaseSensor.h"
#include "Utils/TimeUtils.h"
#include "Utils/PrintMessage.h"

class DallasSensor : public BaseSensor,
                     public OneWireAddressAssigned {
   public:
    DallasSensor(const String& name) : BaseSensor(name, VT_FLOAT), OneWireAddressAssigned{this} {
        _state = DALLAS_IDLE;
    };

    void onAssign() override {
        _obj = new Dallas(getAddress());
        if (!_obj->isConnected()) {
            Serial.print("not found: ");
            Serial.println(getName());
        }
    }

    const bool hasValue() override {
        bool res = false;
        switch (_state) {
            case DALLAS_IDLE:
                if (_obj->requestTemperature()) {
                    _state = DALLAS_CONVERSATION;
                    _requestConversationTime = millis();
                    res = false;
                }
                break;
            case DALLAS_CONVERSATION:
                if (millis_since(_requestConversationTime) >= _obj->getWaitTimeForConversion()) {
                    if (_obj->isConversionComplete()) {
                        _value = _obj->getTempC();
                        _state = DALLAS_IDLE;
                        res = true;
                    }
                };
                break;
            default:
                break;
        };
        return res;
    }
    const String onReadSensor() override {
        return String(_value, 2);
    }

   private:
    enum DallasSensorState {
        DALLAS_IDLE,
        DALLAS_CONVERSATION
    };
    float _value;
    unsigned long _requestConversationTime;
    DallasSensorState _state;
    Dallas* _obj;
};
