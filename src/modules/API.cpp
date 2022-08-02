#include "ESPConfiguration.h"

void* getAPI_Lcd2004(String subtype, String params);
void* getAPI_ButtonIn(String subtype, String params);
void* getAPI_ButtonOut(String subtype, String params);
void* getAPI_IoTServo(String subtype, String params);
void* getAPI_Mp3(String subtype, String params);
void* getAPI_Telegram(String subtype, String params);
void* getAPI_Timer(String subtype, String params);
void* getAPI_IarduinoRTC(String subtype, String params);
void* getAPI_Mcp23017(String subtype, String params);
void* getAPI_SysExt(String subtype, String params);
void* getAPI_Variable(String subtype, String params);

void* getAPI(String subtype, String params) {
void* tmpAPI;
if ((tmpAPI = getAPI_Lcd2004(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_ButtonIn(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_ButtonOut(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_IoTServo(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Mp3(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Telegram(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Timer(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_IarduinoRTC(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Mcp23017(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_SysExt(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Variable(subtype, params)) != nullptr) return tmpAPI;
return nullptr;
}