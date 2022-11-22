#include "Global.h"
#include "classes/IoTItem.h"

#include <AHTxx.h>
#include <map>


std::map<String, AHTxx*> ahts;

String getStatus(AHTxx *aht) {
  switch (aht->getStatus()) {
    case AHTXX_NO_ERROR:
      return F("no error");
      break;

    case AHTXX_BUSY_ERROR:
      return F("sensor AHT busy, increase polling time");
      break;

    case AHTXX_ACK_ERROR:
      return F("sensor AHT didn't return ACK, not connected, broken, long wires (reduce speed), bus locked by slave (increase stretch limit)");
      break;

    case AHTXX_DATA_ERROR:
      return F(" AHT: received data smaller than expected, not connected, broken, long wires (reduce speed), bus locked by slave (increase stretch limit)");
      break;

    case AHTXX_CRC8_ERROR:
      return F("AHT: computed CRC8 not match received CRC8, this feature supported only by AHT2x sensors");
      break;

    default:
      return F("AHT: unknown status");    
      break;
  }
}

class AhtXXt : public IoTItem {
   private:
    AHTxx *_aht = nullptr;

   public:
    AhtXXt(AHTxx* aht, String parameters) : IoTItem(parameters) {
        _aht = aht;
    }

    void doByInterval() {
        value.valD = _aht->readTemperature();
        if (value.valD != AHTXX_ERROR) {
            regEvent(value.valD, "AhtXXt");
        } else {
            SerialPrint("E", "Sensor AHTXX", getStatus(_aht), _id);
        }
    }

    ~AhtXXt(){};
};

class AhtXXh : public IoTItem {
   private:
    AHTxx *_aht = nullptr;

   public:
    AhtXXh(AHTxx* aht, String parameters) : IoTItem(parameters) {
        _aht = aht;
    }

    void doByInterval() {
        value.valD = _aht->readHumidity();
        if (value.valD != AHTXX_ERROR) {
            regEvent(value.valD, "AhtXXh");
        } else {
            SerialPrint("E", "Sensor AHTXX", getStatus(_aht), _id);
        }
    }

    ~AhtXXh(){};
};

void* getAPI_AhtXX(String subtype, String param) {
    if (subtype == F("AhtXXt") || subtype == F("AhtXXh")) {
        String addr;
        jsonRead(param, "addr", addr);
        if (addr == "") {
            scanI2C();
            return nullptr;
        }
    
       if (ahts.find(addr) == ahts.end()) {
            int shtType;
            jsonRead(param, "type", shtType);

            ahts[addr] = new AHTxx(hexStringToUint8(addr), (AHTXX_I2C_SENSOR)shtType);

            int pinSCL, pinSDA, i2cFreq;
            jsonRead(settingsFlashJson, "pinSCL", pinSCL, false);
            jsonRead(settingsFlashJson, "pinSDA", pinSDA, false);
            jsonRead(settingsFlashJson, "i2cFreq", i2cFreq, false);
            if (!pinSCL || !pinSDA || !i2cFreq) {
                pinSCL = SCL;
                pinSDA = SDA;
                i2cFreq = 100000;
            }
            ahts[addr]->begin(pinSDA, pinSCL, i2cFreq);
       }
    
       if (subtype == F("AhtXXt")) {
           return new AhtXXt(ahts[addr], param);
       } else if (subtype == F("AhtXXh")) {
           return new AhtXXh(ahts[addr], param);
       }
    }

    return nullptr;
}
