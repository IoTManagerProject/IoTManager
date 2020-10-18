#include "Utils/SysUtils.h"

#include "Global.h"
#include "Utils/PrintMessage.h"

const String getUniqueId(const char* name) {
    return String(name) + getMacAddress();
}

uint32_t ESP_getChipId(void) {
#ifdef ESP32
    uint32_t id = 0;
    for (uint32_t i = 0; i < 17; i = i + 8) {
        id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    return id;
#else
    return ESP.getChipId();
#endif
}

uint32_t ESP_getFlashChipId(void) {
#ifdef ESP32
    // Нет аналогичной (без доп.кода) функций в 32
    // надо использовать другой id - варианты есть
    return ESP_getChipId();
#else
    return ESP.getFlashChipId();
#endif
}

const String getChipId() {
    return String(ESP_getChipId()) + "-" + String(ESP_getFlashChipId());
}

void setChipId() {
    chipId = getChipId();
    SerialPrint("I", "System", "id: " + chipId);
}

#ifdef ESP8266
static uint32_t total_memory = 52864;
#else
static uint32_t total_memory = ESP.getHeapSize();
#endif

const String printMemoryStatus() {
    uint32_t free = ESP.getFreeHeap();
    uint32_t used = total_memory - free;
    uint32_t memory_load = (used * 100) / total_memory;
    char buf[64];
    sprintf(buf, "used: %d%% free: %s", memory_load, getHeapStats().c_str());
    return String(buf);
}

#ifdef ESP8266
const String getHeapStats() {
    uint32_t free;
    uint16_t max;
    uint8_t frag;
    ESP.getHeapStats(&free, &max, &frag);
    String buf;
    buf += prettyBytes(free);
    buf += " frag: ";
    buf += frag;
    buf += '%';
    return buf;
}
#else
const String getHeapStats() {
    String buf;
    buf = prettyBytes(ESP.getFreeHeap());
    return buf;
}
#endif

const String getMacAddress() {
    uint8_t mac[6];
    char buf[13] = {0};
#if defined(ESP8266)
    WiFi.macAddress(mac);
    sprintf(buf, MACSTR, MAC2STR(mac));
#else
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    sprintf(buf, MACSTR, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
    return String(buf);
}

#ifdef ESP8266
void setLedStatus(LedStatus_t status) {
    if (jsonReadBool(configSetupJson, "blink") == 1) {
        pinMode(LED_PIN, OUTPUT);
        switch (status) {
            case LED_OFF:
                noTone(LED_PIN);
                digitalWrite(LED_PIN, HIGH);
                break;
            case LED_ON:
                noTone(LED_PIN);
                digitalWrite(LED_PIN, LOW);
                break;
            case LED_SLOW:
                tone(LED_PIN, 1);
                break;
            case LED_FAST:
                tone(LED_PIN, 20);
                break;
            default:
                break;
        }
    }
}
#else
void setLedStatus(LedStatus_t status) {
    if (jsonReadBool(configSetupJson, "blink") == 1) {
        pinMode(LED_PIN, OUTPUT);
        switch (status) {
            case LED_OFF:
                digitalWrite(LED_PIN, HIGH);
                break;
            case LED_ON:
                digitalWrite(LED_PIN, LOW);
                break;
            case LED_SLOW:
                break;
            case LED_FAST:
                break;
            default:
                break;
        }
    }
}
#endif
//===================================================================
/*
  void web_print (String text) {
  if (WiFi.status() == WL_CONNECTED) {
    jsonWriteStr(json, "test1",  jsonReadStr(json, "test2"));
    jsonWriteStr(json, "test2",  jsonReadStr(json, "test3"));
    jsonWriteStr(json, "test3",  jsonReadStr(json, "test4"));
    jsonWriteStr(json, "test4",  jsonReadStr(json, "test5"));
    jsonWriteStr(json, "test5",  jsonReadStr(json, "test6"));

    jsonWriteStr(json, "test6", GetTime() + " " + text);

    ws.textAll(json);
  }
  }
*/
//===================================================================
/*
  "socket": [
      "ws://{{ip}}/ws"
        ],
*/
//===================================================================
/*
  {
      "type": "h4",
      "title": "('{{build2}}'=='{{firmware_version}}'?'NEW':'OLD')"
    },
*/
//===================================================================
/*
    {
      "type": "button",
      "title": "Конфигурация устройства",
      "socket": "test2",
      "class": "btn btn-block btn-primary"
    },
  {
      "type": "hr"
    },
               {
      "type": "h6",
      "title": "{{test1}}"
    },
                {
      "type": "h6",
      "title": "{{test2}}"
    },
               {
      "type": "h6",
      "title": "{{test3}}"
    },
                {
      "type": "h6",
      "title": "{{test4}}"
    },
                {
      "type": "h6",
      "title": "{{test5}}"
    },
                {
      "type": "h6",
      "title": "{{test6}}"
    },
                {
      "type": "hr"
    },
*/
//===================================================================

/*
  String getResetReason(uint8_t core) {
  int reason = rtc_get_reset_reason(core);
  switch (reason) {
    case 1  : return "Power on"; break;                                  //Vbat power on reset
    case 3  : return "Software reset digital core"; break;               //Software reset digital core
    case 4  : return "Legacy watch dog reset digital core"; break;       //Legacy watch dog reset digital core
    case 5  : return "Deep Sleep reset digital core"; break;             //Deep Sleep reset digital core
    case 6  : return "Reset by SLC module, reset digital core"; break;   //Reset by SLC module, reset digital core
    case 7  : return "Timer Group0 Watch dog reset digital core"; break; //Timer Group0 Watch dog reset digital core
    case 8  : return "Timer Group1 Watch dog reset digital core"; break; //Timer Group1 Watch dog reset digital core
    case 9  : return "RTC Watch dog Reset digital core"; break;          //
    case 10 : return "Instrusion tested to reset CPU"; break;
    case 11 : return "Time Group reset CPU"; break;
    case 12 : return "Software reset CPU"; break;
    case 13 : return "RTC Watch dog Reset CPU"; break;
    case 14 : return "for APP CPU, reseted by PRO CPU"; break;
    case 15 : return "Reset when the vdd voltage is not stable"; break;
    case 16 : return "RTC Watch dog reset digital core and rtc module"; break;
    default : return "NO_MEAN";
  }
  }


  String EspClass::getResetReason(void) {
  char buff[32];
  if (resetInfo.reason == REASON_DEFAULT_RST) { // normal startup by power on
    strcpy_P(buff, PSTR("Power on"));
  } else if (resetInfo.reason == REASON_WDT_RST) { // hardware watch dog reset
    strcpy_P(buff, PSTR("Hardware Watchdog"));
  } else if (resetInfo.reason == REASON_EXCEPTION_RST) { // exception reset, GPIO status won’t change
    strcpy_P(buff, PSTR("Exception"));
  } else if (resetInfo.reason == REASON_SOFT_WDT_RST) { // software watch dog reset, GPIO status won’t change
    strcpy_P(buff, PSTR("Software Watchdog"));
  } else if (resetInfo.reason == REASON_SOFT_RESTART) { // software restart ,system_restart , GPIO status won’t change
    strcpy_P(buff, PSTR("Software/System restart"));
  } else if (resetInfo.reason == REASON_DEEP_SLEEP_AWAKE) { // wake up from deep-sleep
    strcpy_P(buff, PSTR("Deep-Sleep Wake"));
  } else if (resetInfo.reason == REASON_EXT_SYS_RST) { // external system reset
    strcpy_P(buff, PSTR("External System"));
  } else {
    strcpy_P(buff, PSTR("Unknown"));
  }
  return String(buff);
  }
*/
