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
  // описание параметров передаваемых из настроек датчика из веба
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
          int tm = int(unixTime);
          if (tm > 1)
          {
            extBLEdata[mac_address]["last"] = tm - 5;
          }
          extBLEdata[mac_address]["last_milis"] = millis();
        }
      }
      else
      {
        for (JsonPair kv : BLEdata)
        {
          extBLEdata[mac_address][kv.key()] = BLEdata[kv.key()];
        }
        // дописываем время прихода пакета данных

        int tm = int(unixTime);
        if (tm > 1)
        {
          extBLEdata[mac_address]["last"] = tm - 5;
        }
        else
        {
          extBLEdata[mac_address]["last_milis"] = millis();
        }
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
  // описание параметров передаваемых из настроек датчика из веба
  String _MAC;
  String _sensor;
  int orange = 0;
  int red = 0;
  int offline = 0;
  int _minutesPassed = 0;
  String json = "{}";
  bool dataFromNode = false;

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
    jsonRead(parameters, F("orange"), orange);
    jsonRead(parameters, F("red"), red);
    jsonRead(parameters, F("offline"), offline);
  }

  void setNewWidgetAttributes()
  {

    jsonWriteStr(json, F("info"), prettyMinutsTimeout(_minutesPassed));
    if (dataFromNode)
    {
      if (orange != 0 && red != 0 && offline != 0)
      {
        if (_minutesPassed < orange)
        {
          jsonWriteStr(json, F("color"), "");
        }
        if (_minutesPassed >= orange && _minutesPassed < red)
        {
          jsonWriteStr(json, F("color"), F("orange")); // сделаем виджет оранжевым
        }
        if (_minutesPassed >= red && _minutesPassed < offline)
        {
          jsonWriteStr(json, F("color"), F("red")); // сделаем виджет красным
        }
        if (_minutesPassed >= offline)
        {
          jsonWriteStr(json, F("info"), F("offline"));
        }
      }
    }
    else
    {
      jsonWriteStr(json, F("info"), F("awaiting"));
    }
    sendSubWidgetsValues(_id, json);
  }
  //=======================================================================================================
  void onMqttWsAppConnectEvent()
  {
    setNewWidgetAttributes();
  }

  // doByInterval()
  void doByInterval()
  {
    _minutesPassed++;

    if (_sensor == "last")
    {
      //  SerialPrint("i", F("BLE"), "LAST: " + extBLEdata[_MAC][_sensor].as<String>());
      //  SerialPrint("i", F("BLE"), "LAST_millis: " + extBLEdata[_MAC]["last_milis"].as<String>());
      //  SerialPrint("i", F("BLE"), "_minutesPassed: " + String(_minutesPassed));

      String valStr = extBLEdata[_MAC][_sensor].as<String>();
      if (valStr != "null")
      {
        setValue(valStr);
      }
      else
      {
        int valINT = int(extBLEdata[_MAC]["last_milis"]);
        if (valINT > 0)
        {
          setValue(TimeToString(millis() / 1000 - valINT / 1000));
        }
      }
    }
    else
    {

      String valStr = extBLEdata[_MAC][_sensor].as<String>();
      if (valStr != "null")
      {
        if (valStr != "")
        {
          setValue(valStr);
          extBLEdata[_MAC][_sensor] = "";
          _minutesPassed = 0;
          dataFromNode = true;
        }
      }
      else
      {
        value.isDecimal = 0;
        value.valS = "";
        regEvent(value.valS, _id);
        jsonWriteStr(json, F("info"), F("awaiting"));
        sendSubWidgetsValues(_id, json);
      }
    }
    setNewWidgetAttributes();
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
