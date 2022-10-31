#include "Global.h"
#include "classes/IoTItem.h"
#include <Arduino.h>
#ifdef ESP32
#include <NimBLEDevice.h>
#include <decoder.h>

// Создаем переменную для хранения данных с датчиков bluetooth
StaticJsonDocument<JSON_BUFFER_SIZE * 4> BLEbuffer;
JsonObject extBLEdata = BLEbuffer.to<JsonObject>();

BLEScan *pBLEScan;
TheengsDecoder decoder;
StaticJsonDocument<512> doc;

class BleScan : public IoTItem, BLEAdvertisedDeviceCallbacks
{
private:
  //описание параметров передаваемых из настроек датчика из веба
  int _scanDuration;
  String _filter;

public:
  //=======================================================================================================
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

      String mac_address = BLEdata["id"].as<const char *>();
      mac_address.replace(":", "");

      if (_filter != "")
      {
        if (BLEdata[_filter])
        {
          for (JsonPair kv : BLEdata)
          {
            extBLEdata[mac_address][kv.key()] = BLEdata[kv.key()];
          }

          // дописываем время прихода пакета данных
          extBLEdata[mac_address]["last"] = millis();
        }
      }
      else
      {
        for (JsonPair kv : BLEdata)
        {
          extBLEdata[mac_address][kv.key()] = BLEdata[kv.key()];
        }
        // дописываем время прихода пакета данных
        extBLEdata[mac_address]["last"] = millis();
      }
    };
  }

  BleScan(String parameters) : IoTItem(parameters)
  {
    _scanDuration = jsonReadInt(parameters, "scanDuration");
    _filter = jsonReadStr(parameters, "filter");

    if (pBLEScan->isScanning() == false)
    {
      SerialPrint("i", F("BLE"), "Start Scanning...");
      BLEDevice::init("");
      pBLEScan = BLEDevice::getScan(); // create new scan
      pBLEScan->setAdvertisedDeviceCallbacks(this);
      pBLEScan->setActiveScan(false); // active scan uses more power, but get results faster
      pBLEScan->setInterval(100);
      pBLEScan->setWindow(99); // less or equal setInterval value
    }
  }

  //=======================================================================================================

  // doByInterval()
  void doByInterval()
  {

    if (_scanDuration > 0)
    {
      BLEScanResults foundDevices = pBLEScan->start(_scanDuration, true);
      int count = foundDevices.getCount();
      SerialPrint("i", F("BLE"), "Devices found: " + String(count));
      SerialPrint("i", F("BLE"), "Scan done!");
      pBLEScan->clearResults();
    }
    for (JsonPair kv : extBLEdata)
    {
      String val = extBLEdata[kv.key()].as<String>();
      SerialPrint("i", F("BLE"), _id + " " + kv.key().c_str() + " " + val);
    }
  }

  //=======================================================================================================

  ~BleScan(){};
};

class BleSens : public IoTItem
{
private:
  //описание параметров передаваемых из настроек датчика из веба
  String _MAC;
  String _sensor;

public:
  //=======================================================================================================
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

  BleSens(String parameters) : IoTItem(parameters)
  {
    _MAC = jsonReadStr(parameters, "MAC");
    _sensor = jsonReadStr(parameters, "sensor");
  }

  //=======================================================================================================

  // doByInterval()
  void doByInterval()
  {
    if (_sensor == "last")
    {
      int valInt = extBLEdata[_MAC][_sensor].as<int>();
      char *s;
      s = TimeToString(millis() / 1000 - valInt / 1000);
      value.isDecimal = 0;
      if (valInt > 0)
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
      String valStr = extBLEdata[_MAC][_sensor].as<String>();
      if (valStr != "null")
      {
        if (value.isDecimal = isDigitDotCommaStr(valStr))
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
  //=======================================================================================================

  ~BleSens(){};
};
#endif

// Заглушка для ESP8266
#ifdef ESP8266
class Ble : public IoTItem
{
private:
public:
  Ble(String parameters) : IoTItem(parameters) {}
};
#endif

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
