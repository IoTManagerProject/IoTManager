#pragma once
#include "classes/IoTGpio.h"
//#include "classes/IoTBench.h"

class IoTBench;

struct IoTValue {
    float valD = 0;
    String valS = "";

    bool isDecimal = true;
};

class IoTItem {
   public:
    IoTItem(const String& parameters);
    virtual ~IoTItem() {};
    virtual void loop();
    virtual void doByInterval();
    virtual IoTValue execute(String command, std::vector<IoTValue>& param);

    void checkIntFromNet();

    virtual void regEvent(const String& value, const String& consoleInfo, bool error = false, bool genEvent = true);
    virtual void regEvent(float value, const String& consoleInfo, bool error = false, bool genEvent = true);

    String getSubtype();

    String getID();
    bool isStrInID(const String& str);
    int getIntFromNet();
    virtual String getValue();
    long getInterval();
    bool isGlobal();
    
    void sendSubWidgetsValues(String& id, String& json);

    void setInterval(long interval);
    void setIntFromNet(int interval);

    // unsigned long currentMillis;
    // unsigned long prevMillis;
    // unsigned long difference;
    unsigned long nextMillis=0; // достаточно 1 переменной, надо экономить память
    // задержка следующего вызова, не изменяет текущий _interval
    void suspendNextDoByInt(unsigned long _delay); // 0 - force

    IoTValue value;  // хранение основного значения, которое обновляется из сценария, execute(), loop() или doByInterval()

    // bool iAmDead = false;  // признак необходимости удалить объект из базы
    bool iAmLocal = true;  // признак того, что айтем был создан локально

    bool enableDoByInt = true;

    virtual IoTGpio* getGpioDriver();
    virtual IoTItem* getRtcDriver();
    //virtual IoTItem* getCAMDriver();
    virtual IoTItem* getTlgrmDriver();
    //virtual IoTBench* getBenchmark();
    virtual IoTBench*getBenchmarkTask();
    virtual IoTBench*getBenchmarkLoad();
    virtual unsigned long getRtcUnixTime();

        // делаем доступным модулям отправку сообщений в телеграм
    virtual void sendTelegramMsg(bool often, String msg);
    virtual void sendFoto(uint8_t *buf, uint32_t length, const String &name);
    virtual void editFoto(uint8_t *buf, uint32_t length, const String &name);


    virtual void setValue(const IoTValue& Value, bool genEvent = true);
    virtual void setValue(const String& valStr, bool genEvent = true);
        String getRoundValue();
    void getNetEvent(String& event);
    virtual String getMqttExterSub();

    // хуки для системных событий (должны начинаться с "on")
    virtual void onRegEvent(IoTItem* item);
    virtual void onMqttRecive(String& topic, String& msg);
    virtual void onMqttWsAppConnectEvent();
    virtual void onModuleOrder(String& key, String& value);
    virtual void onTrackingValue(IoTItem* item);  // момент, когда ядро заметило изменение отслеживаемого значения

    // для обновления экрана Nextion из телеграм
    virtual void uploadNextionTlgrm(String &url);

    // методы для графиков (будет упрощено)
    virtual void publishValue();
    virtual void clearValue();
    virtual void setPublishDestination(int type, int wsNum = -1);
    virtual void clearHistory();
    virtual void setTodayDate();

    bool isTracking(IoTItem* item);    // проверка на отслеживание

   protected:
    bool _needSave = false;  // признак необходимости сохранять и загружать значение элемента на flash
    String _subtype = "";
    String _id = "errorId";  // если будет попытка создания Item без указания id, то элемент оставит это значение
    long _interval = 0;
    int _intFromNet = -2;  // количество секунд доверия, пришедших из сети вместе с данными для текущего ИД
                           // -2 - данные не приходили, скорее всего, элемент локальный, доверие есть, в случае прихода сетевого значения с int=0, будет выключен механизм проверки доверия
                           // -1 - данные приходили и обратный отсчет дошел до нуля, значит доверия нет и элемент будет удален при следующем такте loop

    float _multiply;  // умножаем на значение
    float _plus;      // увеличиваем на значение
    int _map1 = 0;
    int _map2 = 0;
    int _map3 = 0;
    int _map4 = 0;
    int _round = 1;  // 1, 10, 100, 1000, 10000
    int _numDigits = 1;     // количество целых значений, не значимые позиции заменяются нулем в строковом формате

    bool _global = false;  // характеристика айтема, что ему нужно слать и принимать события из внешнего мира

    IoTValue* _trackingValue = nullptr;   // указатель на значение родительского элемента изменение которого отслеживается
};

IoTItem* findIoTItem(const String& name);                     // поиск экземпляра элемента модуля по имени
IoTItem* findIoTItemByPartOfName(const String& partName);      // поиск элемента модуля в существующей конфигурации по части имени
String getItemValue(const String& name);                      // поиск плюс получение значения
bool isItemExist(const String& name);                         // существует ли айтем
//StaticJsonDocument<JSON_BUFFER_SIZE>* getLocalItemsAsJSON();  // сбор всех локальных значений Items

IoTItem* createItemFromNet(const String& itemId, const String& value, int interval);
IoTItem* createItemFromNet(const String& msgFromNet);
void analyzeMsgFromNet(const String& msg, String altId = "");

// class externalVariable : IoTItem {  // объект, создаваемый при получении информации о событии на другом контроллере для хранения информации о событии указанное время

//    public:
//     externalVariable(const String& parameters);
//     ~externalVariable();
//     void doByInterval();  // для данного класса doByInterval+int выполняет роль счетчика обратного отсчета до уничтожения
// };