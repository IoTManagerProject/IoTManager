#pragma once

#include "Utils/JsonUtils.h"
#include <WString.h>

class IoTVariable {
    public:
        IoTVariable();
        ~IoTVariable();

        virtual String execute(String command);
        virtual void selfExec();
        virtual void loop();
        virtual String getValue(String key);

        void init(String key, String id);
        void regEvent(String value, String consoleInfo);
        String loadValue(String id);

        String getKey();
        String getID();
        
    protected:
        String _key;  //имя переменной, для идентификации при работе с несколькими переменными в одном модуле
        String _id;  //код переменной для идентификации событий, команд в сценариях и логах

};