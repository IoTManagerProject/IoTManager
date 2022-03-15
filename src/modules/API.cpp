#include "ESPConfiguration.h"

void* getAPI_Mcp23017(String subtype, String params);
void* getAPI_SysExt(String subtype, String params);
void* getAPI_Variable(String subtype, String params);
void* getAPI_ButtonIn(String subtype, String params);
void* getAPI_ButtonOut(String subtype, String params);
void* getAPI_Ads1115(String subtype, String params);
void* getAPI_Aht20(String subtype, String params);
void* getAPI_AnalogAdc(String subtype, String params);
void* getAPI_Bme280(String subtype, String params);
void* getAPI_Bmp280(String subtype, String params);
void* getAPI_Dht1122(String subtype, String params);
void* getAPI_Ds18b20(String subtype, String params);
void* getAPI_GY21(String subtype, String params);
void* getAPI_Hdc1080(String subtype, String params);
void* getAPI_Max6675(String subtype, String params);
void* getAPI_Mhz19(String subtype, String params);
void* getAPI_Sds011(String subtype, String params);
void* getAPI_Sht20(String subtype, String params);
void* getAPI_Lcd2004(String subtype, String params);

void* getAPI(String subtype, String params) {
void* tmpAPI;
if ((tmpAPI = getAPI_Mcp23017(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_SysExt(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Variable(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_ButtonIn(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_ButtonOut(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Ads1115(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Aht20(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_AnalogAdc(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Bme280(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Bmp280(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Dht1122(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Ds18b20(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_GY21(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Hdc1080(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Max6675(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Mhz19(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Sds011(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Sht20(subtype, params)) != nullptr) return tmpAPI;
if ((tmpAPI = getAPI_Lcd2004(subtype, params)) != nullptr) return tmpAPI;return nullptr;
}