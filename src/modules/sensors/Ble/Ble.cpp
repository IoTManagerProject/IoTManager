#include "Global.h"
#include "classes/IoTItem.h"
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <decoder.h>
#include <vector>

// Создаем переменную для хранения данных с датчиков bluetooth
// StaticJsonDocument<JSON_BUFFER_SIZE * 4> BLEbuffer;
// DynamicJsonDocument extBLEdata(JSON_BUFFER_SIZE * 4);
// JsonObject extBLEdata = BLEbuffer.to<JsonObject>();
class BleSens;
std::vector<BleSens *> BleSensArray;

class BleSens : public IoTItem
{
private:
  // описание параметров передаваемых из настроек датчика из веба
  String _MAC;
  String _sensor;
  int timeRecv;
public:
  String whoIAm(/*String &mac, String &sens*/)
  {
    // mac = _MAC;
    // sens = _sensor;
    return _MAC;
  }

  void setBLEdata(JsonObject extBLEdata)
  {
    if (_sensor == "last")
    {
      timeRecv = extBLEdata[_sensor].as<int>();
      char *s;
      s = TimeToString(millis() / 1000 - timeRecv / 1000);
      value.isDecimal = 0;
      if (timeRecv > 0)
      {
        value.valS = s;
      }
      else
      {
        value.valS = "";
      }
      regEvent(value.valS, _id);
    }
    else
    {
      String valStr = extBLEdata[_sensor].as<String>();
      if (valStr != "null")
      {
        if (value.isDecimal == isDigitDotCommaStr(valStr))
        {
          value.isDecimal = 1;
          value.valD = valStr.toFloat();
          regEvent(value.valD, _id);
        }
        else
        {
          value.isDecimal = 0;
          value.valS = valStr;
          regEvent(value.valS, _id);
        }
      }
      else
      {
        value.isDecimal = 0;
        value.valS = "";
        regEvent(value.valS, _id);
      }
    }
  }
  char *TimeToString(unsigned long t)
  {
    static char str[12];
    long h = t / 3600;
    t = t % 3600;
    int m = t / 60;
    int s = t % 60;
    sprintf(str, "%02ld:%02d:%02d", h, m, s);
    return str;
  }

  void doByInterval()
  {
    if (_sensor == "last")
    {
      char *s;
      s = TimeToString(millis() / 1000 - timeRecv / 1000);
      value.isDecimal = 0;
      if (timeRecv > 0)
      {
        value.valS = s;
      }
      else
      {
        value.valS = "";
      }
      regEvent(value.valS, _id);
    }
  }

  BleSens(String parameters) : IoTItem(parameters)
  {
    _MAC = jsonReadStr(parameters, "MAC");
    _sensor = jsonReadStr(parameters, "sensor");
    BleSensArray.push_back(this);
  }

  ~BleSens(){};
};

//=======================================================================================================

/** Callback to process the results of the last scan or restart it */
void scanEndedCB(NimBLEScanResults results)
{
  int count = results.getCount();
  SerialPrint("i", F("BLE"), "Scan done! "); // +"Devices found: " + String(count));
  // pBLEScan->clearResults();
}

class BleScan : public IoTItem, BLEAdvertisedDeviceCallbacks
{
private:
  // описание параметров передаваемых из настроек датчика из веба
  int _scanDuration;
  String _filter;
  bool _debug;

  StaticJsonDocument<512> doc;
  BLEScan *pBLEScan;
  TheengsDecoder decoder;

public:
  std::string convertServiceData(std::string deviceServiceData)
  {
    int serviceDataLength = (int)deviceServiceData.length();
    char spr[2 * serviceDataLength + 1];
    for (int i = 0; i < serviceDataLength; i++)
      sprintf(spr + 2 * i, "%.2x", (unsigned char)deviceServiceData[i]);
    spr[2 * serviceDataLength] = 0;
    return spr;
  }

  void onResult(BLEAdvertisedDevice *advertisedDevice)
  {
    JsonObject BLEdata = doc.to<JsonObject>();
    String mac_adress_ = advertisedDevice->getAddress().toString().c_str();
    mac_adress_.toUpperCase();
    BLEdata["id"] = (char *)mac_adress_.c_str();

    if (advertisedDevice->haveName())
    {
      BLEdata["name"] = (char *)advertisedDevice->getName().c_str();
    }
    if (advertisedDevice->haveManufacturerData())
    {
      char *manufacturerdata = BLEUtils::buildHexData(NULL, (uint8_t *)advertisedDevice->getManufacturerData().data(), advertisedDevice->getManufacturerData().length());
      BLEdata["manufacturerdata"] = manufacturerdata;
      free(manufacturerdata);
    }
    if (advertisedDevice->haveRSSI())
      BLEdata["rssi"] = (int)advertisedDevice->getRSSI();
    if (advertisedDevice->haveTXPower())
      BLEdata["txpower"] = (int8_t)advertisedDevice->getTXPower();
    if (advertisedDevice->haveServiceData())
    {
      int serviceDataCount = advertisedDevice->getServiceDataCount();
      for (int j = 0; j < serviceDataCount; j++)
      {
        std::string service_data = convertServiceData(advertisedDevice->getServiceData(j));
        BLEdata["servicedata"] = (char *)service_data.c_str();
        std::string serviceDatauuid = advertisedDevice->getServiceDataUUID(j).toString();
        BLEdata["servicedatauuid"] = (char *)serviceDatauuid.c_str();
      }
    }

    if (decoder.decodeBLEJson(BLEdata))
    {
      BLEdata.remove("manufacturerdata");
      BLEdata.remove("servicedata");
      BLEdata.remove("type");
      BLEdata.remove("cidc");
      BLEdata.remove("acts");
      BLEdata.remove("cont");
      BLEdata.remove("track");

      String mac_address = BLEdata["id"].as<const char *>();
      mac_address.replace(":", "");
      // дописываем время прихода пакета данных
      BLEdata["last"] = millis();
      if (_debug)
      {
        if ((_filter != "" && BLEdata[_filter]) || _filter == "")
        {
          //  for (JsonPair kv : BLEdata)
          // {
          // String val = BLEdata.as<String>();
          String output;
          serializeJson(BLEdata, output);
          SerialPrint("i", F("BLE"), _id + " " + output);
          //}
        }
      }
      BLEdata.remove("servicedatauuid");
      SerialPrint("i", F("BLE"), "found: " + mac_address);
      // Перебираем все зарегистрированные сенсоры BleSens
      for (std::vector<BleSens *>::iterator it = BleSensArray.begin();
           it != BleSensArray.end(); ++it)
      {
        // Если это данные для нужного сенсора (по его МАКУ)
        if ((*it)->whoIAm() == mac_address)
          // то передаем ему json, дальше он сам разберется
          (*it)->setBLEdata(BLEdata);
      }
    }
  }

  BleScan(String parameters) : IoTItem(parameters)
  {
    _scanDuration = jsonReadInt(parameters, "scanDuration");
    _filter = jsonReadStr(parameters, "filter");
    jsonRead(parameters, "debug", _debug);

    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); // create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(this);
    pBLEScan->setActiveScan(false); // active scan uses more power, but get results faster
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);    // less or equal setInterval value
    pBLEScan->setMaxResults(0); // do not store the scan results, use callback only.
  }

  // doByInterval()
  void doByInterval()
  {
    if (pBLEScan->isScanning() == false)
    {
      if (_scanDuration > 0)
      {
        SerialPrint("i", F("BLE"), "Start Scanning...");
        pBLEScan->start(_scanDuration, scanEndedCB, false);
      }
    }
  }

  ~BleScan(){
    BleSensArray.clear();
  };
};

//=======================================================================================================

void *getAPI_Ble(String subtype, String param)
{
  if (subtype == F("BleScan"))
  {
    return new BleScan(param);
  }
  else if (subtype == F("BleSens"))
  {
    return new BleSens(param);
  }
  else
  {
    return nullptr;
  }
}
