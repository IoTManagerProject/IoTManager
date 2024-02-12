#pragma once

#define SLAVE true
#define TIMEOUT_TRESHOLD 5

namespace _Boiler
{   
    // команды/установки от термостата
    struct SetpointBoiler
    {
        uint8_t cmd_chEnable = 0;
        uint8_t cmd_dhwEnable = 0;
        float TSetCH = 0;
        float TSetDhw = 0;
    } set;

    struct failCode
    {
        bool service_required = 0;
        bool lockout_reset = 0;
        bool low_water_pressure = 0;
        bool gas_fault = 0;
        bool air_fault = 0;
        bool water_overtemp = 0;
        uint8_t fault_code = 0;
    };

    // текущее реальное состояние котла
    struct StateBoiler
    {
        uint8_t stateCH = 0;
        uint8_t stateDHW = 0;
        uint8_t fl_flame = 0;
        uint8_t currentRele = 0;
        uint8_t fl_fail = 0;
        failCode fCode;
        float RelModLevel = 0;
        float Tboiler = -40;
        float Tret = 0;
        float Tdhw = 0;
        float Toutside = 0;
        bool r[3] = {0, 0, 0};
    } state;

    // конфигурация котла
    struct ConfigBoiler
    {
        int antiFreez;
        bool pump = false;              // 1- наличие реле насоса СО, 0 - мы не управляем насосом СО (в протоколе ОТ нет)
        bool changeRele = false; 
    //    bool dhw = false;               // 1- есть реле(трехходовой) ГВС
        bool ctrlType = false;          // 0 - модуляция, 1- вкл/выкл
        bool confDhw = false;           // 1 - бак, 0 - проточная //TODO ПОКА НЕ ЗНАЮ ЧТО ДЕЛАТЬ
        bool pumpControlMaster = false; // в протоколе ОТ: мастер управляет насосом ????????????????????? //TODO Команды кправления насосом от мастера не помню

        int minDhw;
        int maxDhw;
        int minCH;
        int maxCH;

        int gistDhw;
        int gistCH;

        int countRele = 0;
        int relePwr[3]={0,0,0};
        int prcOnekWt = 0; // процент одного киловата из общей мощности всех тэнев, расчитывается для модуляции
        //    int rele2Pwr = 0;
        //    int rele3Pwr = 0;
    } conf;



    unsigned long timeout_count = 0;

    int _debug = 0;
    bool _telegram = false;
    unsigned long ot_response = 0;
}
