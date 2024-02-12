#include "Global.h"
#include "classes/IoTItem.h"
#include <Arduino.h>
#include "BoilerHeader.h"

namespace _Boiler
{
    DynamicJsonDocument OpenThemData(JSON_BUFFER_SIZE / 2);

    IoTItem *_idPID = nullptr;
    IoTItem *_idTboiler = nullptr;
    IoTItem *_idTret = nullptr;
    IoTItem *_idToutside = nullptr;

    IoTItem *_idStateCH = nullptr;
    IoTItem *_idStateFlame = nullptr;
    IoTItem *_idModLevel = nullptr;
    IoTItem *_idCmdCH = nullptr;
    IoTItem *_idSetCH = nullptr;
    IoTItem *_idCtrlType = nullptr;

    int pid;
    IoTItem *rele[3];
    IoTItem *relePump;
    IoTItem *releDhw;
    IoTItem *tmp;

    // проверяем если пришедшее значение отличается от предыдущего регистрируем событие
    void publishNew(String widget, String value)
    {
        if (OpenThemData[widget] != value)
        {
            OpenThemData[widget] = value;

            tmp = findIoTItem(widget);
            if (tmp)
            {
                tmp->setValue(value, true);
            }
            else
            {
                if (_debug > 0)
                    SerialPrint("new", "SmartBoiler", widget + " = " + value);
            }
        }
    }

    void sendTelegramm(String msg)
    {
        if (_telegram == 1)
        {
            if (tlgrmItem)
                tlgrmItem->sendTelegramMsg(false, msg);
        }
    }

    /*
     * =========================================================================================
     *                    КЛАСС УПРАВЛЕНИЯ ГВС
     * =========================================================================================
     */

    IoTItem *_idTdhw = nullptr;
    IoTItem *_idStateDHW = nullptr;
    IoTItem *_idCmdDHW = nullptr;
    IoTItem *_idTDhw = nullptr;
    IoTItem *_idSetDHW = nullptr;

    class DHWControl : public IoTItem
    {
    private:
        unsigned long ts = 0;

    public:
        DHWControl(String parameters) : IoTItem(parameters)
        {
            SerialPrint("i", F("DHWControl"), " START...   ");

            String tmpID;
            jsonRead(parameters, "idReleDhw", tmpID);
            releDhw = findIoTItem(tmpID);

            jsonRead(parameters, "idSetDHW", tmpID);
            _idSetDHW = findIoTItem(tmpID);
            jsonRead(parameters, "idStateDHW", tmpID);
            _idStateDHW = findIoTItem(tmpID);
            jsonRead(parameters, "idCmdDHW", tmpID);
            _idCmdDHW = findIoTItem(tmpID);
            jsonRead(parameters, "idTDhw", tmpID);
            _idTDhw = findIoTItem(tmpID);

            jsonRead(parameters, "minDhw", conf.minDhw);
            jsonRead(parameters, "maxDhw", conf.maxDhw);

            jsonRead(parameters, "gistDhw", conf.gistDhw);

            // releDhw = findIoTItem("releDhw");
            if (releDhw)
            {
                SerialPrint("i", "DHWControl", "Инициализировано РЕЛЕ ГВС");
            }

            // dhw_ctrl = this;
        }

        // ============================== ЛОГИКА РАБОТЫ КОТЛА И ВКЛЮЧЕНИЯ ТЭНОВ ======================================

        // Работы котла и включения тэнов
        static void logicPowerDhw()
        {
            if (_idCmdDHW)
                set.cmd_dhwEnable = ::atoi(_idCmdDHW->getValue().c_str());

            if (_idTDhw)
                state.Tdhw = ::atof(_idTDhw->getValue().c_str());

            if (_idSetDHW)
                set.TSetDhw = ::atof(_idSetDHW->getValue().c_str());

            if (set.cmd_dhwEnable)
            {
                if (releDhw)
                {
                    if (_idTDhw)
                        state.Tdhw = ::atof(_idTDhw->getValue().c_str());

                    if (set.TSetDhw - state.Tdhw >= conf.gistDhw && state.Tdhw > -5 && state.Tdhw < 120)
                    {
                        // включаем ГВС
                        releDhw->setValue("1", true);
                        state.stateDHW = 1;
                        state.stateCH = 0;
                        for (uint8_t i = 0; i < conf.countRele; i++)
                        {
                            state.r[i] = true;
                        }
                        state.RelModLevel = 100;
                    }
                    else
                    {
                        releDhw->setValue("0", true);
                        state.stateDHW = 0;
                        state.RelModLevel = 0;
                    }
                }
            }
            else
            {
                if (releDhw)
                    releDhw->setValue("0", true);
            }
            // publishNew("StateDHW", String(state.stateDHW));
            IoTValue val;
            if (_idStateDHW)
            {
                val.valD = state.stateDHW;
                _idStateDHW->setValue(val, true);
            }
        }

        void doByInterval()
        {
        }

        // Комманды из сценария
        IoTValue execute(String command, std::vector<IoTValue> &param)
        {
            IoTValue val;
            if (command == "SetDHW")
            {
                set.TSetDhw = param[0].valD;
                set.TSetDhw = constrain(set.TSetDhw, conf.minDhw, conf.maxDhw);
                // publishNew("SetDHW", String(set.TSetDhw));
                val.valD = set.TSetDhw;
                if (_idSetDHW)
                    _idSetDHW->setValue(val, true);
                SerialPrint("i", "DHWControl", "Scenario DHWSet ");
            }
            else if (command == "DHWEnable")
            {
                set.cmd_dhwEnable = param[0].valD;
                val.valD = set.cmd_dhwEnable;
                if (_idCmdDHW)
                    _idCmdDHW->setValue(val, true);
                SerialPrint("i", "DHWControl", "Scenario DHWEnable ");
            }

            return {};
        }

        ~DHWControl()
        {
        }
    };

    /*
     * =========================================================================================
     *                    КЛАСС УПРАВЛЕНИЯ КОТЛОМ
     * =========================================================================================
     */
    class BoilerControl : public IoTItem
    {
    private:
        unsigned long ts = 0;

    public:
        BoilerControl(String parameters) : IoTItem(parameters)
        {
            SerialPrint("i", F("BoilerControl"), " START...   ");

            jsonRead(parameters, "LogLevel", _debug);
            jsonRead(parameters, "telegram", _telegram);
            String tmpID;
            jsonRead(parameters, "idPID", tmpID);
            _idPID = findIoTItem(tmpID);
            jsonRead(parameters, "idTboiler", tmpID);
            _idTboiler = findIoTItem(tmpID);
            jsonRead(parameters, "idTret", tmpID);
            _idTret = findIoTItem(tmpID);
            jsonRead(parameters, "idToutside", tmpID);
            _idToutside = findIoTItem(tmpID);

            jsonRead(parameters, "idStateCH", tmpID);
            _idStateCH = findIoTItem(tmpID);
            jsonRead(parameters, "idStateFlame", tmpID);
            _idStateFlame = findIoTItem(tmpID);
            jsonRead(parameters, "idModLevel", tmpID);
            _idModLevel = findIoTItem(tmpID);
            jsonRead(parameters, "idCmdCH", tmpID);
            _idCmdCH = findIoTItem(tmpID);
            jsonRead(parameters, "idSetCH", tmpID);
            _idSetCH = findIoTItem(tmpID);
            jsonRead(parameters, "idCtrlType", tmpID);
            _idCtrlType = findIoTItem(tmpID);

            jsonRead(parameters, "rele1_Pwr", conf.relePwr[0]);
            jsonRead(parameters, "rele2_Pwr", conf.relePwr[1]);
            jsonRead(parameters, "rele3_Pwr", conf.relePwr[2]);
            jsonRead(parameters, "Pump", conf.pump);
            jsonRead(parameters, "changeRele", conf.changeRele);

            jsonRead(parameters, "minCH", conf.minCH);
            jsonRead(parameters, "maxCH", conf.maxCH);
            jsonRead(parameters, "gistCH", conf.gistCH);
            jsonRead(parameters, "antiFreez", conf.antiFreez);

            configuration();
        }

        // ============================== ЛОГИКА РАБОТЫ КОТЛА И ВКЛЮЧЕНИЯ ТЭНОВ ======================================

        // Работы котла и включения тэнов
        static void logicPowerOn()
        {
            // TODO ВКЛЮЧЕНИЕ РЕЛЕ ПО ИХ МОЩНОСТИ (СДЕЛАТЬ ШАГИ НАГРЕВА И КОМБИНАЦИИ ВКЛБЧЕНИЯ ТЕНОВ С РАЗНОЙ МОЩНОСТЬЮ)
            for (uint8_t i = 0; i < conf.countRele; i++)
            {
                state.r[i] = false;
            }
            //  сейчас включаются по порядку
            state.RelModLevel = 0;
            pid = 0;
            state.fl_flame = false;
            if (_idPID)
                pid = ::atoi(_idPID->getValue().c_str());
            // обнуляем ГВС
            state.stateDHW = 0;

            if (state.Tboiler < conf.maxCH)
            {
                // if (dhw_ctrl)
                //{
                //  если есть модуль ГВС, то вызываем его логику включения тэнов
                DHWControl::logicPowerDhw();
                //}
                if (!state.stateDHW) // Если уже включено ГВС, то нечего смотреть на отопление
                {
                    if (set.cmd_chEnable)
                    {
                        publishNew("status", "Штатный режим");
                        // включаем отопление
                        state.stateCH = 1;
                        if (state.Tboiler < (set.TSetCH + conf.gistCH) && /*set.TSetCH - state.Tboiler >= conf.gistCH &&*/ state.Tboiler > -5 && state.Tboiler < 120)
                        {
                            if (conf.ctrlType == 0)
                            { // режим модуляции, это когда есть модуль ПИД и более одного реле

                                if (pid > 0)
                                {
                                    state.r[state.currentRele] = true;
                                    state.RelModLevel = conf.prcOnekWt * conf.relePwr[state.currentRele];
                                }
                                if (pid > 30 && pid <= 60)
                                {
                                    uint8_t next = state.currentRele + 1;
                                    if (next >= conf.countRele)
                                        next = 0;
                                    state.r[next] = true;
                                    state.RelModLevel = (conf.prcOnekWt * conf.relePwr[state.currentRele]) + (conf.prcOnekWt * conf.relePwr[next]);
                                }
                                if (pid > 60)
                                {
                                    for (uint8_t i = 0; i < conf.countRele; i++)
                                    {
                                        state.r[i] = true;
                                    }
                                    state.RelModLevel = 100;
                                }
                            }
                            else
                            { // у нас релейный режим без ПИДа или с одним реле(тэном)
                                // TODO СЕЙЧАС ВКЛЮЧАЕМ ВСЕ РЕЛЕ, НАДО ЧТО ТО ПРИДУМАТЬ УМНОЕ
                                for (uint8_t i = 0; i < conf.countRele; i++)
                                {
                                    state.r[i] = true;
                                }
                                state.RelModLevel = 100;
                            }
                        }
                        else
                        {
                            state.RelModLevel = 0;
                        }
                    }
                    else
                    {
                        publishNew("status", "Выкл отопление");
                        state.stateCH = 0;
                    }
                }
            }
            if (conf.antiFreez > 0 && state.Tboiler < (conf.antiFreez + 4))
            {
                state.r[state.currentRele] = true;
                state.RelModLevel = conf.prcOnekWt * conf.relePwr[state.currentRele];
                // setValue("Анти-Заморозка");
                publishNew("status", "Режим анти-заморозка");
                SerialPrint("i", "BoilerControl", "Режим анти-заморозка");
                sendTelegramm("Режим анти-заморозка");
            }

            static bool prev_flame = false;

            if (state.RelModLevel > 0)
                state.fl_flame = true; // если хоть одно реле включено, то выставляем флаг горелки

            if (state.fl_flame && prev_flame != state.fl_flame)
            {
                if (conf.changeRele)
                {
                    state.currentRele++;
                    if (state.currentRele >= conf.countRele)
                        state.currentRele = 0;
                }
            }
            prev_flame = state.fl_flame;

            bool fl_pump = false;

            // переключаем реле в соответсии с их статусами
            for (uint8_t i = 0; i < conf.countRele; i++)
            {
                if (rele[i])
                {
                    rele[i]->setValue(state.r[i] ? "1" : "0", true);
                }
                if (!fl_pump)                            // что бы не обнулить если выстиавили true
                    fl_pump = state.r[i] ? true : false; // если хоть одно реле включено, то включаем насос
            }
            if (fl_pump)
            { // если хоть одно реле включено, то включаем насос
                if (relePump)
                    relePump->setValue("1", true);
            }
            else
            {
                if (relePump)
                {                                                       // если все реле выключены
                    if (state.Tboiler > conf.minCH && set.cmd_chEnable) // НО температура ещё горячая и при этом отопление включено, то включаем насос
                        relePump->setValue("1", true);
                    else // Если температура в котле уже остыла, или отопление нам не нужно (летом нагрели воду-пусть котел сам остывает без насоса), то выключаем насос
                        relePump->setValue("0", true);
                }
            }
            IoTValue val;
            if (_idModLevel)
            {
                val.valD = state.RelModLevel;
                _idModLevel->setValue(val, true);
            }
            if (_idStateCH)
            {
                val.valD = state.stateCH;
                _idStateCH->setValue(val, true);
            }
            if (_idStateFlame)
            {
                val.valD = state.fl_flame;
                _idStateFlame->setValue(val, true);
            }

            //  publishNew("ModLevel", String(state.RelModLevel));
            //  publishNew("stateCH", String(state.stateCH));
            //  publishNew("controlType", String(conf.ctrlType));
            //  publishNew("StateFlame", String(state.fl_flame));
        }

        //============================== ОБЕСПЕЧЕНИЕ РАБОТЫ IoTMANAGER =====================================

        // конфигурирование котла в зависимости от настроек
        void configuration()
        {
            state.fl_flame = state.stateCH = 0;
            conf.countRele = conf.prcOnekWt = 0;
            if (conf.relePwr[0])
            {
                rele[0] = findIoTItem("rele1");
                if (rele[0])
                {
                    conf.countRele++;
                    SerialPrint("i", "BoilerControl", "Инициализировано РЕЛЕ 1-го тэна");
                }
            }

            if (conf.relePwr[1])
            {
                rele[1] = findIoTItem("rele2");
                if (rele[1])
                {
                    conf.countRele++;
                    SerialPrint("i", "BoilerControl", "Инициализировано РЕЛЕ 2-го тэна");
                }
            }

            if (conf.relePwr[2])
            {
                rele[2] = findIoTItem("rele3");
                if (rele[2])
                {
                    conf.countRele++;
                    SerialPrint("i", "BoilerControl", "Инициализировано РЕЛЕ 3-го тэна");
                }
            }

            for (int i = 0; i < conf.countRele; i++)
            {
                conf.prcOnekWt += conf.relePwr[i];
            }
            if (conf.countRele && conf.prcOnekWt)
            {
                // conf.prcOnekWt /= conf.countRele;
                conf.prcOnekWt = 100 / conf.prcOnekWt;
                // SerialPrint("i", "BoilerControl", "Процент одного кВт = " + String (conf.prcOnekWt) + "%");
            }

            if (conf.pump)
            {
                relePump = findIoTItem("relePump");
                if (relePump)
                {
                    SerialPrint("i", "BoilerControl", "Инициализировано РЕЛЕ Насоса");
                }
            }
            conf.ctrlType = false;
            if (conf.countRele == 1 || _idPID == nullptr)
                conf.ctrlType = true;

           // IoTValue val;
           // val.valD = conf.ctrlType;
            if (_idCtrlType)
            {
                if (conf.ctrlType)
                    _idCtrlType->setValue("Вкл/Выкл", true);
                else
                    _idCtrlType->setValue("Модуляция", true);
            }

            updateStateboiler();
        }

        void doByInterval()
        {
            // updateStateboiler();

            // Принудительно чистим данные, что бы обновился интерфейс
            OpenThemData.clear();

            if (_debug > 0)
            {
                SerialPrint("i", "BoilerControl", "Обновляем данные в web интерфейсе");
            }
            if (_debug > 0)
            {
                SerialPrint("i", "BoilerControl", "memoryUsage: " + String(OpenThemData.memoryUsage()));
            }
        }

        // Основной цикл программы
        void loop()
        {
            unsigned long new_ts = millis();
            int delay = 1000;
            if (new_ts - ts > delay)
            {
                ts = new_ts;
                updateStateboiler();
                logicPowerOn();
            }
            // для новых версий IoTManager
            IoTItem::loop();
        }

        // Комманды из сценария
        IoTValue execute(String command, std::vector<IoTValue> &param)
        {
            IoTValue val;
            if (command == "CHSet")
            {
                set.TSetCH = param[0].valD;
                set.TSetCH = constrain(set.TSetCH, conf.minCH, conf.maxCH);
                val.valD = set.TSetCH;
                if (_idSetCH)
                    _idSetCH->setValue(val, true);
                // publishNew("SetCH", String(set.TSetCH));
                SerialPrint("i", "BoilerControl", "Scenario CHSet ");
            }
            else if (command == "CHEnable")
            {
                set.cmd_chEnable = param[0].valD;
                val.valD = set.cmd_chEnable;
                if (_idCmdCH)
                    _idCmdCH->setValue(val, true);
                SerialPrint("i", "BoilerControl", "Scenario CHEnable ");
            }

            return {};
        }

        // обновление данных от датчиков
        void updateStateboiler()
        {
            if (_idTboiler)
                state.Tboiler = ::atof(_idTboiler->getValue().c_str());
            if (_idTret)
                state.Tret = ::atof(_idTret->getValue().c_str());
            if (_idToutside)
                state.Toutside = ::atof(_idToutside->getValue().c_str());

            //   if (_idStateCH)
            //       state.stateCH = ::atoi(_idStateCH->getValue().c_str());
            //   if (_idStateFlame)
            //       state.fl_flame = ::atoi(_idStateFlame->getValue().c_str());
            //    if (_idModLevel)
            //      state.RelModLevel = ::atof(_idModLevel->getValue().c_str());
            if (_idCmdCH)
                set.cmd_chEnable = ::atoi(_idCmdCH->getValue().c_str());
            if (_idSetCH)
                set.TSetCH = ::atof(_idSetCH->getValue().c_str());
        }

        ~BoilerControl()
        {
        }
    };

}

void *getAPI_SmartBoiler(String subtype, String param)
{
    if (subtype == F("BoilerControl"))
    {
        return new _Boiler::BoilerControl(param);
    }
    else if (subtype == F("DHWControl"))
    {
        return new _Boiler::DHWControl(param);
    }
    else
    {
        return nullptr;
    }
}
