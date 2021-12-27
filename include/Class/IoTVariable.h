#pragma once

#include <WString.h>

class IoTVariable {
    public:
        IoTVariable();
        ~IoTVariable();

        virtual String execute(String command);
        virtual void loop();

        void init(String key, String value);
        void setValue(String value);
        void getValue(String value);
        void regEvent(String value, String consoleInfo);

        String _widgetName;  //название виджета на фронтэнде для правильного отображения информации из меременной
        String _key;  //код переменной для идентификации событий, команд в сценариях
        String _title;

    private:
        String _value;  //значение переменной
};