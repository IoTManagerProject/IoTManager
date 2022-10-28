#pragma once
#include "classes/IoTGpio.h"

struct IoTValue {
    float valD = 0;
    String valS = "";

    bool isDecimal = true;
};

class IoTItem {
   public:
    IoTItem(const String &parameters);
    virtual ~IoTItem() {}
    virtual void loop();
    virtual void doByInterval();
    virtual IoTValue execute(String command, std::vector<IoTValue>& param);

    virtual void regEvent(const String& value, const String& consoleInfo, bool error = false);
    virtual void regEvent(float value, const String& consoleInfo, bool error = false);

    String getSubtype();

    String getID();
    virtual String getValue();

    void setInterval(unsigned long interval);

    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;

    IoTValue value;  // хранение основного значения, которое обновляется из сценария, execute(), loop() или doByInterval()

    bool iAmDead = false;  // признак необходимости удалить объект из базы
    bool iAmLocal = true;  // признак того, что айтем был создан локально

    bool enableDoByInt = true;

    virtual IoTGpio* getGpioDriver();
    virtual void setValue(const IoTValue& Value, bool generateEvent = true);
    virtual void setValue(const String& valStr, bool generateEvent = true);
    String getRoundValue();

    //методы для графиков
    virtual void publishValue();
    virtual void clearValue();
    virtual void setPublishDestination(int type, int wsNum = -1);
    virtual void clearHistory();
    virtual void setTodayDate();

   protected:
    bool _needSave = false;  // признак необходимости сохранять и загружать значение элемента на flash
    String _subtype;
    String _id;
    unsigned long _interval;

    float _multiply;  // умножаем на значение
    float _plus;      // увеличиваем на значение
    int _map1;
    int _map2;
    int _map3;
    int _map4;
    int _round;  // 1, 10, 100, 1000, 10000

    bool _global = false;  // характеристика айтема, что ему нужно слать и принимать события из внешнего мира
};

IoTItem* findIoTItem(const String& name);                            // поиск экземпляра элемента модуля по имени
String getItemValue(const String& name);                             // поиск плюс получение значения
bool isItemExist(const String& name);                                // существует ли айтем
StaticJsonDocument<JSON_BUFFER_SIZE>* getLocalItemsAsJSON();  // сбор всех локальных значений Items

class externalVariable : IoTItem {  // объект, создаваемый при получении информации о событии на другом контроллере для хранения информации о событии указанное время

   public:
    externalVariable(const String& parameters);
    ~externalVariable();
    void doByInterval();  // для данного класса doByInterval+int выполняет роль счетчика обратного отсчета до уничтожения
};