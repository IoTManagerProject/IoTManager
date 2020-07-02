#pragma once

#include "Objects/SensorItem.h"
#include "Base/ValueMapper.h"

class AnalogSensorItem : public SensorItem {
   public:
    AnalogSensorItem(const String& name, const String& assign,
                     long min_value, long max_value, long min_mapped, long max_mapped) : AnalogSensorItem(name, assign,
                                                                                                          new ValueMapper(min_value, max_value, min_mapped, max_mapped)){};

    AnalogSensorItem(const String& name, const String& assign, ValueMapper* mapper) : SensorItem(name, assign), _mapper{mapper} {
        pinMode(getPin(), INPUT);
    };

    int read() {
        return _mapper->getValue(onRead());
    }

    int onRead() override;

   private:
    ValueMapper* _mapper;
};