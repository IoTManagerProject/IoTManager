#include "ESPConfiguration.h"

//============================================================================================
//здесь скопируйте строку и вставьте ниже, заменив имя AnalogAdc на название вашего сенсора
void* getAPI_AnalogAdc(String subtype, String params);
void* getAPI_ds18b20(String subtype, String params);
//============================================================================================

void* getAPI(String subtype, String params) {
    void* tmpAPI;
    //===============================================================================================================
        //здесь нужно скопировать строку еще раз и вставить ее ниже, переименовав AnalogAdc на название вашего сенсора
    if ((tmpAPI = getAPI_AnalogAdc(subtype, params)) != nullptr) return tmpAPI;
    //================================================================================================================

    if ((tmpAPI = getAPI_ds18b20(subtype, params)) != nullptr) return tmpAPI;
     
    return nullptr;
}