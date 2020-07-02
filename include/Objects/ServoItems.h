#include "Base/BaseServo.h"

class ServoItems {
   public:
    ServoItems();

    BaseServo* add(const String& name, const String& pin, const String& value,
                   const String& min_value, const String& max_value, const String& min_deg, const String& max_deg);

    BaseServo* get(const String& name);

    size_t count();

    BaseServo* last();

   private:
    std::vector<BaseServo> _items;
};

extern ServoItems myServo;