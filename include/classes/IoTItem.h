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

    void checkIntFromNet();

    virtual void regEvent(const String& value, const String& consoleInfo, bool error = false, bool genEvent = true);
    virtual void regEvent(float value, const String& consoleInfo, bool error = false, bool genEvent = true);

    String getSubtype();

    String getID();
    int getIntFromNet();
    virtual String getValue();
    long getInterval();
    bool isGlobal();
    
    void setInterval(long interval);
    void setIntFromNet(int interval);

    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;

    IoTValue value;  // хранение основного значения, которое обновляется из сценария, execute(), loop() или doByInterval()

    //bool iAmDead = false;  // признак необходимости удалить объект из базы
    bool iAmLocal = true;  // признак того, что айтем был создан локально

    bool enableDoByInt = true;

    virtual IoTGpio* getGpioDriver();
    virtual void setValue(const IoTValue& Value, bool genEvent = true);
    virtual void setValue(const String& valStr, bool genEvent = true);
    String getRoundValue();
    void getNetEvent(String& event);

    // хуки для системных событий
    virtual void onRegEvent(IoTItem* item);
    virtual void onMqttRecive(char* topic, uint8_t* payload, size_t length);

    //методы для графиков
    virtual void publishValue();
    virtual void clearValue();
    virtual void setPublishDestination(int type, int wsNum = -1);
    virtual void clearHistory();
    virtual void setTodayDate();

   protected:
    bool _needSave = false;  // признак необходимости сохранять и загружать значение элемента на flash
    String _subtype = "";
    String _id = "errorId";     // если будет попытка создания Item без указания id, то элемент оставит это значение
    long _interval = 0;
    int _intFromNet = -2;   // количество секунд доверия, пришедших из сети вместе с данными для текущего ИД
                            // -2 - данные не приходили, скорее всего, элемент локальный, доверие есть
                            // -1 - данные приходили и обратный отсчет дошел до нуля, значит доверия нет 

    float _multiply;  // умножаем на значение
    float _plus;      // увеличиваем на значение
    int _map1 = 0;
    int _map2 = 0;
    int _map3 = 0;
    int _map4 = 0;
    int _round = 1;  // 1, 10, 100, 1000, 10000

    bool _global = false;  // характеристика айтема, что ему нужно слать и принимать события из внешнего мира
};

IoTItem* findIoTItem(const String& name);                            // поиск экземпляра элемента модуля по имени
String getItemValue(const String& name);                             // поиск плюс получение значения
bool isItemExist(const String& name);                                // существует ли айтем
StaticJsonDocument<JSON_BUFFER_SIZE>* getLocalItemsAsJSON();  // сбор всех локальных значений Items

IoTItem* createItemFromNet(const String& itemId, const String& value, int interval);
IoTItem* createItemFromNet(const String& msgFromNet);
void analyzeMsgFromNet(const String& msg, String altId = "");

// class externalVariable : IoTItem {  // объект, создаваемый при получении информации о событии на другом контроллере для хранения информации о событии указанное время

//    public:
//     externalVariable(const String& parameters);
//     ~externalVariable();
//     void doByInterval();  // для данного класса doByInterval+int выполняет роль счетчика обратного отсчета до уничтожения
// };