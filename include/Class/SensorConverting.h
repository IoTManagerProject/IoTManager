#pragma once

#include <Arduino.h>

#include "Class/LineParsing.h"
#include "Global.h"

class SensorConverting : public LineParsing {
   public:
    SensorConverting() : LineParsing(){};

    int mapping(String key, int input) {
        String map_ = jsonReadStr(configOptionJson, key + "_map");
        if (map_ != "") {
            map_.replace("map[", "");
            map_.replace("]", "");
            input = map(input,
                        selectFromMarkerToMarker(map_, ",", 0).toInt(),
                        selectFromMarkerToMarker(map_, ",", 1).toInt(),
                        selectFromMarkerToMarker(map_, ",", 2).toInt(),
                        selectFromMarkerToMarker(map_, ",", 3).toInt());
        }
        return input;
    }

    float correction(String key, float input) {
        _c.replace("c[", "");
        _c.replace("]", "");
        float coef = _c.toFloat();
        input = input * coef;
        return input;
    }
};
//extern SensorConverting* mySensorConverting;