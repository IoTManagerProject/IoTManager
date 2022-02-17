#include "ESPConfiguration.h"

//============================================================================================
//здесь скопируйте строку и вставьте ниже, заменив имя AnalogAdc на название вашего сенсора
void* getAPI_AnalogAdc(String subtype, String params);
void* getAPI_Ds18b20(String subtype, String params);
void* getAPI_Sht20(String subtype, String params);
void* getAPI_Dht1122(String subtype, String params);
void* getAPI_Bmp280(String subtype, String params);
void* getAPI_Bme280(String subtype, String params);
void* getAPI_Aht20(String subtype, String params);
void* getAPI_Hdc1080(String subtype, String params);
void* getAPI_GY21(String subtype, String params);
void* getAPI_Lcd2004(String subtype, String params);
//============================================================================================

void* getAPI(String subtype, String params) {
    void* tmpAPI;
    //===============================================================================================================
    //здесь нужно скопировать строку еще раз и вставить ее ниже, переименовав AnalogAdc на название вашего сенсора
    if ((tmpAPI = getAPI_AnalogAdc(subtype, params)) != nullptr) return tmpAPI;
    if ((tmpAPI = getAPI_Ds18b20(subtype, params)) != nullptr) return tmpAPI;
    if ((tmpAPI = getAPI_Sht20(subtype, params)) != nullptr) return tmpAPI;
    if ((tmpAPI = getAPI_Dht1122(subtype, params)) != nullptr) return tmpAPI;
    if ((tmpAPI = getAPI_Bmp280(subtype, params)) != nullptr) return tmpAPI;
    if ((tmpAPI = getAPI_Bme280(subtype, params)) != nullptr) return tmpAPI;
    if ((tmpAPI = getAPI_Aht20(subtype, params)) != nullptr) return tmpAPI;
    if ((tmpAPI = getAPI_Hdc1080(subtype, params)) != nullptr) return tmpAPI;
    if ((tmpAPI = getAPI_GY21(subtype, params)) != nullptr) return tmpAPI;
    if ((tmpAPI = getAPI_Lcd2004(subtype, params)) != nullptr) return tmpAPI;
    //================================================================================================================

    return nullptr;
}