#pragma once

#include <Arduino.h>

#include "Class/LineParsing.h"
#include "Global.h"

class SensorConverting : public LineParsing {
   public:
    SensorConverting() : LineParsing(){};

    int mapping(int input) {
        if (_map != "") {
            _map.replace("map[", "");
            _map.replace("]", "");
            input = map(input,
                        selectFromMarkerToMarker(_map, ",", 0).toInt(),
                        selectFromMarkerToMarker(_map, ",", 1).toInt(),
                        selectFromMarkerToMarker(_map, ",", 2).toInt(),
                        selectFromMarkerToMarker(_map, ",", 3).toInt());
        }
        return input;
    }

    float correction(float input) {
        _c.replace("c[", "");
        _c.replace("]", "");
        float coef = _c.toFloat();
        input = input * coef;
        return input;
    }
};
//extern SensorConverting* mySensorConverting;