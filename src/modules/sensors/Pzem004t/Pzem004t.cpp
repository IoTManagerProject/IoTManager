
#include "Global.h"
#include "classes/IoTItem.h"

#include "PZEMSensor.h"
#include "SoftUART.h"

#include <map>

// std::map<String, Adafruit_Pzem004*> bmes;
//
// class Pzem004t : public IoTItem {
//    private:
//     Adafruit_Pzem004* _bme;
//
//    public:
//     Pzem004t(Adafruit_Pzem004* bme, String parameters) : IoTItem(parameters) {
//         _bme = bme;
//     }
//
//     void doByInterval() {
//         value.valD = _bme->readTemperature();
//         if (value.valD < 145)
//             regEvent(value.valD, "Pzem004t");
//         else
//             SerialPrint("E", "Sensor Pzem004t", "Error");
//     }
//
//     ~Pzem004t(){};
// };
//
// class Pzem004h : public IoTItem {
//    private:
//     Adafruit_Pzem004* _bme;
//
//    public:
//     Pzem004h(Adafruit_Pzem004* bme, String parameters) : IoTItem(parameters) {
//         _bme = bme;
//     }
//
//     void doByInterval() {
//         value.valD = _bme->readHumidity();
//         if (value.valD < 100)
//             regEvent(value.valD, "Pzem004h");
//         else
//             SerialPrint("E", "Sensor Pzem004h", "Error");
//     }
//
//     ~Pzem004h(){};
// };
//
// class Pzem004p : public IoTItem {
//    private:
//     Adafruit_Pzem004* _bme;
//
//    public:
//     Pzem004p(Adafruit_Pzem004* bme, String parameters) : IoTItem(parameters) {
//         _bme = bme;
//     }
//
//     void doByInterval() {
//         value.valD = _bme->readPressure();
//         if (value.valD > 0) {
//             value.valD = value.valD / 1.333224 / 100;
//             regEvent(value.valD, "Pzem004p");
//         } else
//             SerialPrint("E", "Sensor Pzem004p", "Error");
//     }
//
//     ~Pzem004p(){};
// };
//
// void* getAPI_Pzem004(String subtype, String param) {
//     if (subtype == F("Pzem004t") || subtype == F("Pzem004h") || subtype == F("Pzem004p")) {
//         String addr;
//         jsonRead(param, "addr", addr);
//
//         if (bmes.find(addr) == bmes.end()) {
//             bmes[addr] = new Adafruit_Pzem004();
//             bmes[addr]->begin(hexStringToUint8(addr));
//         }
//
//         if (subtype == F("Pzem004t")) {
//             return new Pzem004t(bmes[addr], param);
//         } else if (subtype == F("Pzem004h")) {
//             return new Pzem004h(bmes[addr], param);
//         } else if (subtype == F("Pzem004p")) {
//             return new Pzem004p(bmes[addr], param);
//         }
//     }
//
//     return nullptr;
// }
