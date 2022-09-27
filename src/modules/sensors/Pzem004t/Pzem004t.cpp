
#include "Global.h"
#include "classes/IoTItem.h"

#include "PZEMSensor.h"
#include "modules/sensors/UART/Uart.h"

class Pzem004v : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem;

   public:
    Pzem004v(String parameters) : IoTItem(parameters) {
        addr = jsonReadStr(parameters, "addr");
        if (myUART) {
            pzem = new PZEMSensor(myUART, hexStringToUint8(addr));
        }
    }

    void doByInterval() {
        if (pzem) {
            bool online = false;
            value.valD = pzem->values(online)->voltage;
            if (online) {
                regEvent(value.valD, "Pzem V");
            } else {
                regEvent(NAN, "Pzem V");
                SerialPrint("E", "Pzem", "V error");
            }
        }
    }

    ~Pzem004v(){};
};

class Pzem004a : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem;

   public:
    Pzem004a(String parameters) : IoTItem(parameters) {
        addr = jsonReadStr(parameters, "addr");
        if (myUART) {
            pzem = new PZEMSensor(myUART, hexStringToUint8(addr));
        }
    }

    void doByInterval() {
        if (pzem) {
            bool online = false;
            value.valD = pzem->values(online)->current;
            if (online) {
                regEvent(value.valD, "Pzem A");
            } else {
                regEvent(NAN, "Pzem A");
                SerialPrint("E", "Pzem", "A error");
            }
        }
    }

    ~Pzem004a(){};
};

class Pzem004w : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem;

   public:
    Pzem004w(String parameters) : IoTItem(parameters) {
        addr = jsonReadStr(parameters, "addr");
        if (myUART) {
            pzem = new PZEMSensor(myUART, hexStringToUint8(addr));
        }
    }

    void doByInterval() {
        if (pzem) {
            bool online = false;
            value.valD = pzem->values(online)->power;
            if (online) {
                regEvent(value.valD, "Pzem W");
            } else {
                regEvent(NAN, "Pzem W");
                SerialPrint("E", "Pzem", "W error");
            }
        }
    }

    ~Pzem004w(){};
};

class Pzem004wh : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem;

   public:
    Pzem004wh(String parameters) : IoTItem(parameters) {
        addr = jsonReadStr(parameters, "addr");
        if (myUART) {
            pzem = new PZEMSensor(myUART, hexStringToUint8(addr));
        }
    }

    void doByInterval() {
        if (pzem) {
            bool online = false;
            value.valD = pzem->values(online)->energy;
            if (online) {
                regEvent(value.valD, "Pzem Wh");
            } else {
                regEvent(NAN, "Pzem Wh");
                SerialPrint("E", "Pzem", "Wh error");
            }
        }
    }

    ~Pzem004wh(){};
};

class Pzem004hz : public IoTItem {
   private:
    String addr;
    PZEMSensor* pzem;

   public:
    Pzem004hz(String parameters) : IoTItem(parameters) {
        addr = jsonReadStr(parameters, "addr");
        if (myUART) {
            pzem = new PZEMSensor(myUART, hexStringToUint8(addr));
        }
    }

    void doByInterval() {
        if (pzem) {
            bool online = false;
            value.valD = pzem->values(online)->freq;
            if (online) {
                regEvent(value.valD, "Pzem Hz");
            } else {
                regEvent(NAN, "Pzem Hz");
                SerialPrint("E", "Pzem", "Hz error");
            }
        }
    }

    ~Pzem004hz(){};
};

void* getAPI_Pzem004(String subtype, String param) {
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
    } else {
        return nullptr;
    }
}
