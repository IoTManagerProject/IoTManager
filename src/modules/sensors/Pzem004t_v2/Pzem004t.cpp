
#include "Global.h"
#include "classes/IoTItem.h"

#include "PZEMSensor.h"
//#include "modules/sensors/UART/Uart.h"
#include "classes/IoTUart.h"
#include <map>

// глобальные списки необходимы для хранения объектов об экземплярах Pzem . Ключ - адрес
std::map<String, PZEMSensor*> pzemSensorArray;
//PZEMContainer _pzemCntr;
Stream* _myUARTpzem = nullptr;

PZEMSensor* getPzemSensor(String addr) {
    if (pzemSensorArray.find(addr) == pzemSensorArray.end()) 
        return nullptr;
    return pzemSensorArray[addr];    
}

class Pzem004v : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004v(String parameters) : IoTItem(parameters) {
             jsonRead(parameters, "addr", addr);
            pzem = getPzemSensor(addr);
    }

    void doByInterval() {
        if (pzem) {
            bool online = false;
            value.valD = pzem->values(online)->voltage;
            if (online) {
                regEvent(value.valD, "Pzem V");
            } else {
                regEvent(NAN, "Pzem V");
                SerialPrint("E", "Pzem", "V error", _id);
            }
        } else {
            regEvent(NAN, "Pzem");
            SerialPrint("E", "Pzem", "initialization error", _id);
            // Запросим pzem, если он не был создан из-за отсутствия UART, если в конфигурации не в правильном порядке
            pzem = getPzemSensor(addr);
        }
    }

    ~Pzem004v(){};
};

class Pzem004a : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004a(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, "addr", addr);
            pzem = getPzemSensor(addr);
    }

    void doByInterval() {
        if (pzem) {
            bool online = false;
            value.valD = pzem->values(online)->current;
            if (online) {
                regEvent(value.valD, "Pzem A");
            } else {
                regEvent(NAN, "Pzem A");
                SerialPrint("E", "Pzem", "A error", _id);
            }
        } else {
            regEvent(NAN, "Pzem");
            SerialPrint("E", "Pzem", "initialization error", _id);
            pzem = getPzemSensor(addr);
        }
    }

    ~Pzem004a(){};
};

class Pzem004w : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004w(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, "addr", addr);
            pzem = getPzemSensor(addr);
    }

    void doByInterval() {
        if (pzem) {
            bool online = false;
            value.valD = pzem->values(online)->power;
            if (online) {
                regEvent(value.valD, "Pzem W");
            } else {
                regEvent(NAN, "Pzem W");
                SerialPrint("E", "Pzem", "W error", _id);
            }
        } else {
            regEvent(NAN, "Pzem");
            SerialPrint("E", "Pzem", "initialization error", _id);
            pzem = getPzemSensor(addr);
        }
    }

    ~Pzem004w(){};
};

class Pzem004wh : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004wh(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, "addr", addr);
            pzem = getPzemSensor(addr);
    }

    void doByInterval() {
        if (pzem) {
            bool online = false;
            value.valD = pzem->values(online)->energy;
            if (online) {
                regEvent(value.valD, "Pzem Wh");
            } else {
                regEvent(NAN, "Pzem Wh");
                SerialPrint("E", "Pzem", "Wh error", _id);
            }
        } else {
            regEvent(NAN, "Pzem");
            SerialPrint("E", "Pzem", "initialization error", _id);
            pzem = getPzemSensor(addr);
        }
    }

    ~Pzem004wh(){};
};

class Pzem004hz : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004hz(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, "addr", addr);
            pzem = getPzemSensor(addr);
    }

    void doByInterval() {
        if (pzem) {
            bool online = false;
            value.valD = pzem->values(online)->freq;
            if (online) {
                regEvent(value.valD, "Pzem Hz");
            } else {
                regEvent(NAN, "Pzem Hz");
                SerialPrint("E", "Pzem", "Hz error", _id);
            }
        } else {
            regEvent(NAN, "Pzem");
            SerialPrint("E", "Pzem", "initialization error", _id);
            pzem = getPzemSensor(addr);
        }
    }

    ~Pzem004hz(){};
};

class Pzem004pf : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004pf(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, "addr", addr);
            pzem = getPzemSensor(addr);
    }

    void doByInterval() {
        if (pzem) {
            bool online = false;
            value.valD = pzem->values(online)->pf;
            if (online) {
                regEvent(value.valD, "Pzem Pf");
            } else {
                regEvent(NAN, "Pzem Pf");
                SerialPrint("E", "Pzem", "Pf error", _id);
            }
        } else {
            regEvent(NAN, "Pzem");
            SerialPrint("E", "Pzem", "initialization error", _id);
            pzem = getPzemSensor(addr);
        }
    }

    ~Pzem004pf(){};
};

class Pzem004cmd : public IoTItem {
   private:
    String addr;
   // int changeaddr;
   // String setaddr;
   // int reset;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004cmd(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, F("addr"), addr);
    //    jsonRead(parameters, F("changeaddr"), changeaddr);
    //    jsonRead(parameters, F("setaddr"), setaddr);
    //    jsonRead(parameters, F("reset"), reset);

        pzem = getPzemSensor(addr);
    }

    void doByInterval() {
        if (pzem) {
        }
    }

    void onModuleOrder(String &key, String &value) {
        if (pzem) {
            if (key == "changeaddr") {
                if (pzem->setAddress(hexStringToUint8(value))) {
                    SerialPrint("i", "Pzem", "address set: " + value);
                } else {
                    SerialPrint("i", "Pzem", "set adress error: " + value);
                }
            
            } else if (key == "reset") {
                if (pzem->reset()) {
                    SerialPrint("i", "Pzem", "reset done");
                } else {
                    SerialPrint("i", "Pzem", "reset error");
                }
            }
        }
    }

    ~Pzem004cmd(){};
};

class Pzem004uart : public IoTUart {
   public:
    Pzem004uart(String parameters) : IoTUart(parameters) {
        _myUARTpzem = _myUART;
        //Обновим везде Uart, если в конфигурации не в правильном порядке
        for (auto it = pzemSensorArray.begin(); it != pzemSensorArray.end(); it ++)
        {   
            if (it->second == nullptr){
                it->second = new PZEMSensor(_myUARTpzem, hexStringToUint8(it->first));
             //   SerialPrint("i", "Pzem", "create pzemSensor");
            }
            it->second->updateSerial(_myUARTpzem);
           // SerialPrint("i", "Pzem", "update serial pzemuart");
        }
    }

    ~Pzem004uart(){};
};



void* getAPI_Pzem004_v2(String subtype, String param) {

    if (subtype == F("Pzem004v") || subtype == F("Pzem004a") || subtype == F("Pzem004w") 
    || subtype == F("Pzem004wh") || subtype == F("Pzem004hz") || subtype == F("Pzem004pf") 
    || subtype == F("Pzem004cmd") )  
    {
        SerialPrint("i", "Pzem", "create constructor"+ subtype);
        String addr;
        jsonRead(param, "addr", addr);
        if (_myUARTpzem) { 
            if (pzemSensorArray.find(addr) == pzemSensorArray.end()) {
                pzemSensorArray[addr] = new PZEMSensor(_myUARTpzem, hexStringToUint8(addr));
               // SerialPrint("i", "Pzem", "create map");
            } else { // Обновление UART нужно для смены пинов для уже созданных объектов в map
                pzemSensorArray[addr]->updateSerial(_myUARTpzem);
               // SerialPrint("i", "Pzem", "create serial constructor");
            }
        }else{// если нет UART, то и библиотеку pzem создаем пустой, что бы потом обновить при создании uart
            pzemSensorArray[addr] = nullptr;
        }
    }

    if (subtype == F("Pzem004v")) {
        return new Pzem004v(param);
    } else if (subtype == F("Pzem004a")) {
        return new Pzem004a(param);
    } else if (subtype == F("Pzem004w")) {
        return new Pzem004w(param);
    } else if (subtype == F("Pzem004wh")) {
        return new Pzem004wh(param);
    } else if (subtype == F("Pzem004hz")) {
        return new Pzem004hz(param);
    } else if (subtype == F("Pzem004pf")) {
        return new Pzem004pf(param);
    } else if (subtype == F("Pzem004cmd")) {
        return new Pzem004cmd(param);
    } else if (subtype == F("Pzem004uart")) {
        return new Pzem004uart(param);
    } else {
        return nullptr;
    }
}
