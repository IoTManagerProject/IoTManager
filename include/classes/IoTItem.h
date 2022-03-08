#pragma once
#include "classes/IoTGpio.h"

struct IoTValue {
    float valD = 0;
    String valS = "";
    bool isDecimal = true;
};

class IoTItem {
   public:
    IoTItem(String parameters);
    virtual ~IoTItem() {}
    virtual void loop();
    virtual void doByInterval();
    virtual IoTValue execute(String command, std::vector<IoTValue> &param);

    void regEvent(String value, String consoleInfo);
    void regEvent(float value, String consoleInfo);

    String getSubtype();
    String getID();
    String getValue();

    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;

    IoTValue value;  // хранение основного значения, котрое обновляется из сценария, execute(), loop() или doByInterval()
    
    bool iAmDead = false;   // признак необходимости удалить объект из базы
    bool iAmLocal = true;   // признак локальной переменной

    virtual IoTGpio* getGpioDriver();
    virtual void setValue(IoTValue Value);

   protected:
    String _subtype;
    String _id;
    unsigned long _interval;
    
    float _multiply;  // умножаем на значение
    float _plus;  // увеличиваем на значение
    int _map1;
    int _map2;
    int _map3;
    int _map4;
    int _round;  // 1, 10, 100, 1000, 10000
};

IoTItem* findIoTItem(String name);  // поиск экземпляра элемента модуля по имени
String getItemValue(String name);  // поиск плюс получение значения
StaticJsonDocument<JSON_BUFFER_SIZE>* getLocalItemsAsJSON();  // сбор всех локальных занчений Items


class externalVariable: IoTItem {   // объект, создаваемый при получении информации о событии на другом контроллере для хранения информации о событии указанное время

    public:
        externalVariable(String parameters); 
        ~externalVariable();
        void doByInterval();   // для данного класса doByInterval+int выполняет роль счетчика обратного отсчета до уничтожения

};