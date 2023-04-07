
#include "Global.h"
#include "classes/IoTItem.h"

#include "PZEMSensor.h"
#include "modules/sensors/UART/Uart.h"
#include <map>

// глобальные списки необходимы для хранения объектов об экземплярах Pzem . Ключ - адрес
std::map<String, PZEMSensor*> pzemSensorArray;
//PZEMContainer _pzemCntr;



class Pzem004v : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004v(PZEMSensor* _pzem, String parameters) : IoTItem(parameters) {
            pzem = _pzem;
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
            regEvent(NAN, "Pzem V");
            SerialPrint("E", "Pzem", "V error", _id);
        }
    }

    ~Pzem004v(){};
};

class Pzem004a : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004a(PZEMSensor* _pzem, String parameters) : IoTItem(parameters) {
            pzem = _pzem;
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
            regEvent(NAN, "Pzem V");
            SerialPrint("E", "Pzem", "V error", _id);
        }
    }

    ~Pzem004a(){};
};

class Pzem004w : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004w(PZEMSensor* _pzem, String parameters) : IoTItem(parameters) {
            pzem = _pzem;
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
            regEvent(NAN, "Pzem V");
            SerialPrint("E", "Pzem", "V error", _id);
        }
    }

    ~Pzem004w(){};
};

class Pzem004wh : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004wh(PZEMSensor* _pzem, String parameters) : IoTItem(parameters) {
            pzem = _pzem;
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
            regEvent(NAN, "Pzem V");
            SerialPrint("E", "Pzem", "V error", _id);
        }
    }

    ~Pzem004wh(){};
};

class Pzem004hz : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004hz(PZEMSensor* _pzem, String parameters) : IoTItem(parameters) {
            pzem = _pzem;
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
            regEvent(NAN, "Pzem V");
            SerialPrint("E", "Pzem", "V error", _id);
        }
    }

    ~Pzem004hz(){};
};

class Pzem004pf : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004pf(PZEMSensor* _pzem, String parameters) : IoTItem(parameters) {
            pzem = _pzem;
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
            regEvent(NAN, "Pzem V");
            SerialPrint("E", "Pzem", "V error", _id);
        }
    }

    ~Pzem004pf(){};
};

class Pzem004cmd : public IoTItem {
   private:
    String addr;
    int changeaddr;
    String setaddr;
    int reset;
    PZEMSensor* pzem = nullptr;

   public:
    Pzem004cmd(PZEMSensor* _pzem, String parameters) : IoTItem(parameters) {
        jsonRead(parameters, F("addr"), addr);
        jsonRead(parameters, F("changeaddr"), changeaddr);
        jsonRead(parameters, F("setaddr"), setaddr);
        jsonRead(parameters, F("reset"), reset);

        pzem = _pzem;
        if (pzem && myUART) {
            if (changeaddr == 1) {
                if (pzem->setAddress(hexStringToUint8(setaddr))) {
                    SerialPrint("i", "Pzem", "address set: " + setaddr);
                } else {
                    SerialPrint("i", "Pzem", "set adress error");
                }
            }
            if (reset == 1) {
                if (pzem->reset()) {
                    SerialPrint("i", "Pzem", "reset done");
                } else {
                    SerialPrint("i", "Pzem", "reset error");
                }
            }
        } else SerialPrint("i", "Pzem", "Pzem command error");
    }

    void doByInterval() {
        if (pzem) {
        }
    }

    ~Pzem004cmd(){};
};

void* getAPI_Pzem004(String subtype, String param) {
    PZEMSensor *pzem = nullptr;
    if (myUART) {
        if (subtype == F("Pzem004v") || subtype == F("Pzem004a") || subtype == F("Pzem004w") 
        || subtype == F("Pzem004wh") || subtype == F("Pzem004hz") || subtype == F("Pzem004pf") 
        || subtype == F("Pzem004cmd") )  {
            String addr;
            jsonRead(param, "addr", addr);
            if (pzemSensorArray.find(addr) == pzemSensorArray.end()) {
                //   pzem = new PZEMSensor(myUART, hexStringToUint8(addr));
                pzemSensorArray[addr] = new PZEMSensor(myUART, hexStringToUint8(addr));
            } else {
                pzemSensorArray[addr]->updateSerial(myUART);
            }
            pzem = pzemSensorArray[addr];
        }
    }

    if (subtype == F("Pzem004v")) {
        return new Pzem004v(pzem, param);
    } else if (subtype == F("Pzem004a")) {
        return new Pzem004a(pzem, param);
    } else if (subtype == F("Pzem004w")) {
        return new Pzem004w(pzem, param);
    } else if (subtype == F("Pzem004wh")) {
        return new Pzem004wh(pzem, param);
    } else if (subtype == F("Pzem004hz")) {
        return new Pzem004hz(pzem, param);
    } else if (subtype == F("Pzem004pf")) {
        return new Pzem004pf(pzem, param);
    } else if (subtype == F("Pzem004cmd")) {
        return new Pzem004cmd(pzem, param);
    } else {
        return nullptr;
    }
}
