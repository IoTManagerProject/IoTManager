
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
            value.valD = pzem->values()->voltage;
            regEvent(value.valD, "Pzem Voltage");
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
            value.valD = pzem->values()->current;
            regEvent(value.valD, "Pzem Ampere");
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
            value.valD = pzem->values()->power;
            regEvent(value.valD, "Pzem Watt");
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
            value.valD = pzem->values()->energy;
            regEvent(value.valD, "Pzem Watt/hr");
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
            value.valD = pzem->values()->freq;
            regEvent(value.valD, "Pzem Hz");
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
