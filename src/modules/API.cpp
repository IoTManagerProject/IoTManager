#include "ESPConfiguration.h"

//============================================================================================
//здесь скопируйте строку и вставьте ниже, заменив имя AnalogAdc на название вашего сенсора
ModuleInfo getAPI_AnalogAdc(String subtype, String params);
ModuleInfo getAPI_ds18b20(String subtype, String params);
//============================================================================================

ModuleInfo getAPI(String subtype, String params) {
    ModuleInfo tmpMi;
    String defConfig = "";
    //===============================================================================================================
        //здесь нужно скопировать строку еще раз и вставить ее ниже, переименовав AnalogAdc на название вашего сенсора
    if ((tmpMi = getAPI_AnalogAdc(subtype, params)).apiToComponent != nullptr) return tmpMi; else defConfig += tmpMi.defConfig;
    //================================================================================================================

    if ((tmpMi = getAPI_ds18b20(subtype, params)).apiToComponent != nullptr) return tmpMi; else defConfig += tmpMi.defConfig;
     
    
    
    tmpMi.defConfig = defConfig;
    return tmpMi;
}