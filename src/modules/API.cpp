#include "ESPConfiguration.h"

void* getAPI_Timer(String subtype, String params);
void* getAPI_Variable(String subtype, String params);
void* getAPI_VButton(String subtype, String params);
void* getAPI_Ds18b20(String subtype, String params);
void* getAPI_RCswitch(String subtype, String params);
void* getAPI_Sonar(String subtype, String params);
void* getAPI_ButtonIn(String subtype, String params);
void* getAPI_ButtonOut(String subtype, String params);
void* getAPI_Mcp23017(String subtype, String params);
void* getAPI_Pcf8574(String subtype, String params);
void* getAPI_Pwm8266(String subtype, String params);
void* getAPI_TelegramLT(String subtype, String params);
void* getAPI_Lcd2004(String subtype, String params);

void* getAPI(String subtype, String params) {
void* tmpAPI;
if ((tmpAPI = getAPI_Timer(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Variable(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_VButton(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Ds18b20(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_RCswitch(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Sonar(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_ButtonIn(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_ButtonOut(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Mcp23017(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Pcf8574(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Pwm8266(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_TelegramLT(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Lcd2004(subtype, params)) != nullptr) return tmpAPI;
return nullptr;
}