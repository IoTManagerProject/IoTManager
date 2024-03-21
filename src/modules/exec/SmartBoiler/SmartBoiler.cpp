#include "Global.h"
#include "classes/IoTItem.h"
#include <Arduino.h>
#include "BoilerHeader.h"
#include <map>
#include <vector>
#include "OpenTherm.h"

#define SLAVE true
#define TIMEOUT_TRESHOLD 5

namespace _Boiler_v2
{
    struct StepPower
    {
        float pwr;
        std::vector<String> listIDRele;
    };

    // std::vector<IoTItem *> vectorRele;

    std::map<String, IoTItem *> mapRele;

    std::map<int, StepPower> stepMap;

    // DynamicJsonDocument OpenThemData(JSON_BUFFER_SIZE / 2);

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
    IoTItem *_relePump = nullptr;

    IoTItem *_idStateDHW = nullptr;
    IoTItem *_idCmdDHW = nullptr;
    IoTItem *_idTDhw = nullptr;
    IoTItem *_idSetDHW = nullptr;
    IoTItem *_releDhw = nullptr;

    float pid;
    // IoTItem *rele[3];
    //    IoTItem *_relePump;

    IoTItem *tmp;

    // проверяем если пришедшее значение отличается от предыдущего регистрируем событие
    void publishNew(String widget, String value)
    {
        tmp = findIoTItem(widget);
        if (tmp)
        {
            if (value != tmp->value.valS)
                tmp->setValue(value, true);
        }
        else
        {
            if (_debug > 0)
                SerialPrint("new", "SmartBoiler", widget + " = " + value);
        }
    }

    // Обновит состояние реле, только если оно поменялось, что бы было меньше событий
    void updateReleState(IoTItem *rele, int val)
    {
        if (rele)
            if (val != ::atoi(rele->getValue().c_str()))
            {
                rele->setValue(String(val), true);
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

    OpenTherm *ot_driver = nullptr;
    OpenTherm *instance_OTdriver(int _RX_pin, int _TX_pin)
    {
        if (!ot_driver)
        {
            ot_driver = new OpenTherm(_RX_pin, _TX_pin, SLAVE);
            // ot_driver->begin();
        }
        return ot_driver;
    }

    // Обработчик прерываний от ОТ
    void IRAM_ATTR handleInterruptSlave()
    {
        if (ot_driver != nullptr)
            ot_driver->handleInterrupt();
    }

    // unsigned long ot_response = 0;
    //  int SlaveMemberIDcode = 0;

    /*
     * =========================================================================================
     *                             КЛАСС РАБОТЫ ПО ПРОТОКОЛУ OPENTHERM
     * =========================================================================================
     */
    class OpenThermSlave : public IoTItem
    {
    private:
        //    unsigned long ts = 0;

    public:
        OpenThermSlave(String parameters) : IoTItem(parameters)
        {
            int _RX_pin = 16;
            int _TX_pin = 4;
            SerialPrint("i", F("OpenThermSlave"), " START...   ");

            jsonRead(parameters, "RX_pin", _RX_pin);
            jsonRead(parameters, "TX_pin", _TX_pin);
            jsonRead(parameters, "MemberID", SlaveMemberIDcode);

            instance_OTdriver(_RX_pin, _TX_pin);
            ot_driver->begin(handleInterruptSlave, processRequest); // responseCallback
                                                                    //            ot_boiler = this;
        }

        void configured()
        {
            static bool isFirstOT = true;
            if (isFirstOT)
            {
                isFirstOT = false;

                bool haveBoiler = false;
                for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it)
                {
                    if ((*it)->getSubtype() == "BoilerControl")
                        haveBoiler = true;
                }
                if (!haveBoiler)
                {
                    SerialPrint("E", "OpenThermSlave", "Warning: BoilerControl не найден! работаем автономно по ID модулей");
                    _idTboiler = findIoTItem("idTboiler");
                    _idTret = findIoTItem("idTret");
                    _idToutside = findIoTItem("idToutside");
                    _idStateCH = findIoTItem("idStateCH");
                    _idStateDHW = findIoTItem("idStateDHW");
                    _idStateFlame = findIoTItem("idStateFlame");
                    _idModLevel = findIoTItem("idModLevel");
                    _idTDhw = findIoTItem("idTDhw");
                    _idCmdCH = findIoTItem("idCmdCH");
                    _idCmdDHW = findIoTItem("idCmdDHW");
                    _idCtrlType = findIoTItem("idCtrlType");
                    _idSetCH = findIoTItem("idSetCH");
                    _idSetDHW = findIoTItem("idSetDHW");
                }
            }
        }

        void doByInterval()
        {
            configured();
        }

        // Основной цикл программы
        void loop()
        {
            ot_driver->process();
            IoTItem::loop();
        }

        // Комманды из сценария
        IoTValue execute(String command, std::vector<IoTValue> &param)
        {
            return {};
        }

        // Обработка управления и отправка статуса
        static void processStatus(unsigned int &data)
        {

            uint8_t statusRequest = data >> 8;      // забрали старший байт с командами мастера
            set.cmd_chEnable = statusRequest & 0x1; // забрали 0 бит из этого байта - включение СО (маска 01)

            set.cmd_dhwEnable = statusRequest & 0x2; // забрали 1 бит из этого байта - включение СО (маска 10)
            IoTValue val;
            val.valD = set.cmd_chEnable;
            if (_idCmdCH)
                _idCmdCH->setValue(val, true);
            val.valD = set.cmd_dhwEnable;
            if (_idCmdDHW)
                _idCmdDHW->setValue(val, true);
            data &= 0xFF00; // старший бит не трогаем, а младший обнулили, что бы его заполнить состоянием котла и вернуть data термостату

            /*
                        //   if (_idFail)
                        //      state.fl_fail = ::atof(_idFail->getValue().c_str());
                        if (_idStateCH)
                            state.stateCH = ::atoi(_idStateCH->getValue().c_str());
                        if (_idStateDHW)
                            state.stateDHW = ::atoi(_idStateDHW->getValue().c_str());
                        if (_idStateFlame)
                            state.fl_flame = ::atoi(_idStateFlame->getValue().c_str());
            */
            if (state.fl_fail)
                data |= 0x01; // fault indication
            if (state.stateCH)
                data |= 0x02; // CH active
            if (state.stateDHW)
                data |= 0x04; // DHW active
            if (state.fl_flame)
                data |= 0x08; // flame on
                              // data |= 0x10; //cooling active
                              // data |= 0x20; //CH2 active
                              // data |= 0x40; //diagnostic/service event
                              // data |= 0x80; //electricity production on
        }

        // обработка сброса ошибок
        static void processCommand(unsigned int &data)
        {
            uint8_t command = data >> 8; // забрали старший байт с командами мастера
            if (command == 1)
            {
                state.fl_fail = 0;
                data |= 128; // ответ 128-255: команда выполнена
            }
        }

        //=================================== Обработка входящих сообщение ОТ ======================================
        static void processRequest(unsigned long request, OpenThermResponseStatus status)
        {
            switch (status)
            {
            case OpenThermResponseStatus::NONE:
                if (_debug > 0)
                {
                    SerialPrint("E", "OpenThermSlave", "Error: OpenTherm не инициализирован");
                }
                break;
            case OpenThermResponseStatus::INVALID:
                if (_debug > 0)
                {
                    SerialPrint("E", "OpenThermSlave", "ID:" + String(ot_driver->getDataID(request)) + " / Error: Ошибка разбора команды: " + String(request, HEX));
                    // build UNKNOWN-DATAID response
                    unsigned long response = ot_driver->buildResponse(OpenThermMessageType::DATA_INVALID, ot_driver->getDataID(request), 0);

                    // send response
                    ot_driver->sendResponse(response);
                }
                break;
            case OpenThermResponseStatus::TIMEOUT:
                if (_debug > 0)
                {
                    SerialPrint("E", "OpenThermSlave", " ID: " + String(ot_driver->getDataID(request)) + " / Error: Таймаут команд от управляющего устройства (термостата)");
                }
                timeout_count++;
                if (timeout_count > TIMEOUT_TRESHOLD)
                {
                    // publishNew("boilerslave", "❌");
                    //  publishNew("status", "не подключен");
                    //                    setValue("ОТ не подключен", true);
                    timeout_count = TIMEOUT_TRESHOLD;
                    if (_debug > 0)
                    {
                        SerialPrint("E", "OpenThermSlave", "OpenTherm: потеря связи с управляющим устройством (термостатом) ❌");
                    }
                    sendTelegramm(("OpenTherm: потеря связи с управляющим устройством (термостатом) ❌"));
                }
                break;
            case OpenThermResponseStatus::SUCCESS:
                timeout_count = 0;
                // publishNew("boilerslave", "✅");
                //  publishNew("status", "подключен");
                //                setValue("ОТ подключен", true);
                //    sendTelegramm(("OpenTherm: котёл подключен ✅"));
                //   respondense_flag = true;
                //   ts_ = new_ts_;
                HandleRequest(request);
                break;
            default:
                break;
            }
        }

        // Парсинг запросов
        static void HandleRequest(unsigned long request)
        {
            /*
                        if (_idCtrlType)
                        {
                            if (_idCtrlType->getValue() == "Модуляция")
                                conf.ctrlType = 0;
                            else
                                conf.ctrlType = 1;
                        }
            */
            // conf.ctrlType = ::atoi(_idCtrlType->getValue().c_str());
            //	unsigned long response;
            unsigned int data = ot_driver->getUInt(request);
            OpenThermMessageType msgType;
            byte ID;
            OpenThermMessageID id = ot_driver->getDataID(request);
            uint8_t flags;
            if (_debug > 1)
            {
                SerialPrint("i", "OpenThermSlave <-", String(millis()) + " ID: " + String(id) + " / requestHEX: " + String(request, HEX) + " / request: " + String(request));
            }
            switch (id)
            {
                /*----------------------------Инициализация и конфигурация----------------------------*/
            case OpenThermMessageID::SConfigSMemberIDcode: // запрос Конфигурации Котла и SlaveMemberID
                msgType = OpenThermMessageType::READ_ACK;
                data = conf.dhw | (conf.ctrlType << 1) | (false << 2) | (conf.confDhw << 3) | (conf.pumpControlMaster << 4) | (false << 5); // 2-cooling, 5-CH2
                data <<= 8;
                data |= SlaveMemberIDcode;
                // data = (int)SlaveMemberIDcode;
                break;
                //        case OpenThermMessageID::MConfigMMemberIDcode: // Получили Master Member ID
                //            msgType = OpenThermMessageType::WRITE_ACK;
                //            break;
                //        case OpenThermMessageID::SlaveVersion: // TODO вернуть версию модуля
                //            msgType = OpenThermMessageType::READ_ACK;
                //            data = (int)1;
                //            break;
                //        case OpenThermMessageID::MasterVersion:
                //            msgType = OpenThermMessageType::WRITE_ACK;
                //            break;
                //        case OpenThermMessageID::OpenThermVersionSlave:
                //            msgType = OpenThermMessageType::READ_ACK;
                //            break;
                /*----------------------------Управление (уставки и команды)----------------------------*/
            case OpenThermMessageID::TdhwSetUBTdhwSetLB: // границы уставки ГВС
                msgType = OpenThermMessageType::READ_ACK;
                data |= (uint8_t)conf.minDhw;
                data |= (uint8_t)conf.maxDhw << 8;
                break;
            case OpenThermMessageID::MaxTSetUBMaxTSetLB: // границы уставки СО
                msgType = OpenThermMessageType::READ_ACK;
                data |= (uint8_t)conf.minCH;
                data |= (uint8_t)conf.maxCH << 8;
                break;
            case OpenThermMessageID::Command: // Сброс ошибок/сброс блокировки котла. Ответ: команды (не)выполнена
                msgType = OpenThermMessageType::READ_ACK;
                processCommand(data);
                break;
            case OpenThermMessageID::TdhwSet: // TODO Получили температуру ГВС
                if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                {
                    msgType = OpenThermMessageType::READ_ACK;
                    //  if (_idSetDHW)
                    //     set.TSetDhw = ::atof(_idSetDHW->getValue().c_str());
                    data = ot_driver->temperatureToData(set.TSetDhw);
                }
                else
                {
                    msgType = OpenThermMessageType::WRITE_ACK;
                    // processDHWSet(ot_driver->getFloat(data));
                    set.TSetDhw = ot_driver->getFloat(data);
                    set.TSetDhw = constrain(set.TSetDhw, conf.minDhw, conf.maxDhw);
                    // publishNew("TDHWSet", String(set.TSetDhw));
                    IoTValue val;
                    val.valD = set.TSetDhw;
                    if (_idSetDHW)
                        _idSetDHW->setValue(val, true);
                }
                break;
            case OpenThermMessageID::TSet: // TODO Получили температуру СО
                if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                {
                    msgType = OpenThermMessageType::READ_ACK;
                    //  if (_idSetCH)
                    //      set.TSetCH = ::atof(_idSetCH->getValue().c_str());
                    data = ot_driver->temperatureToData(set.TSetCH);
                }
                else
                {
                    msgType = OpenThermMessageType::WRITE_ACK;
                    // processCHSet(ot_driver->getFloat(data));
                    set.TSetCH = ot_driver->getFloat(data);
                    set.TSetCH = constrain(set.TSetCH, conf.minCH, conf.maxCH);
                    // publishNew("TCHSet", String(set.TSetCH));
                    IoTValue val;
                    val.valD = set.TSetCH;
                    if (_idSetCH)
                        _idSetCH->setValue(val, true);
                }
                break;
                /*        case OpenThermMessageID::MaxTSet: // максимальная уставка ГВС ??????
                            if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                                msgType = OpenThermMessageType::READ_ACK;
                            else
                                msgType = OpenThermMessageType::WRITE_ACK;
                            break;
                        case OpenThermMessageID::Hcratio: // Коэффециент тепловой кривой
                            if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                                msgType = OpenThermMessageType::READ_ACK;
                            else
                                msgType = OpenThermMessageType::WRITE_ACK;
                            break;
                */
                /*----------------------------Состояние и статусы----------------------------*/
            case OpenThermMessageID::Status: // TODO Вернуть Статус котла
                msgType = OpenThermMessageType::READ_ACK;
                processStatus(data);
                break;
            case OpenThermMessageID::RelModLevel: // запрос модуляции
                msgType = OpenThermMessageType::READ_ACK;
                //  if (_idModLevel)
                //     state.RelModLevel = ::atoi(_idModLevel->getValue().c_str());
                data = ot_driver->temperatureToData(state.RelModLevel);
                break;
            case OpenThermMessageID::Tboiler: // запрос температуры котла
                msgType = OpenThermMessageType::READ_ACK;
                //   if (_idTboiler)
                //      state.Tboiler = ::atof(_idTboiler->getValue().c_str());
                data = ot_driver->temperatureToData(state.Tboiler);
                break;
            case OpenThermMessageID::Tdhw: // запрос температуры ГВС
                msgType = OpenThermMessageType::READ_ACK;
                if (_idTDhw)
                {
                    state.Tdhw = ::atof(_idTDhw->getValue().c_str());
                    data = ot_driver->temperatureToData(state.Tdhw);
                }
                else
                {
                    msgType = OpenThermMessageType::UNKNOWN_DATA_ID;
                }
                break;
            case OpenThermMessageID::Toutside: // запрос внешней температуры
                msgType = OpenThermMessageType::READ_ACK;
                if (_idToutside)
                {
                    state.Toutside = ::atof(_idToutside->getValue().c_str());
                    data = ot_driver->temperatureToData(state.Toutside);
                }
                else
                {
                    msgType = OpenThermMessageType::UNKNOWN_DATA_ID;
                }
                break;
            case OpenThermMessageID::ASFflags: // запрос ошибок
                msgType = OpenThermMessageType::READ_ACK;
                data = 0;
                if (state.fl_fail)
                {
                    data = state.fCode.service_required | (state.fCode.lockout_reset << 1) | (state.fCode.low_water_pressure << 2) | (state.fCode.gas_fault << 3) | (state.fCode.air_fault << 4) | (state.fCode.water_overtemp << 5);
                    data |= (uint8_t)state.fCode.fault_code << 8;
                }
                break;
            case OpenThermMessageID::Tret: // температура обратки
                msgType = OpenThermMessageType::READ_ACK;
                if (_idTret)
                {
                    state.Tret = ::atof(_idTret->getValue().c_str());
                    data = ot_driver->temperatureToData(state.Tret);
                }
                else
                {
                    msgType = OpenThermMessageType::UNKNOWN_DATA_ID;
                }
                break;
                //        case OpenThermMessageID::OEMDiagnosticCode:
                //            msgType = OpenThermMessageType::READ_ACK;
                //            break;
                //        case OpenThermMessageID::ElectricBurnerFlame: // Ток работы горелки ?????
                //            msgType = OpenThermMessageType::READ_ACK;
                //            break;
                //        case OpenThermMessageID::MaxCapacityMinModLevel: // максимальная мощность котла кВт и минимальная модуляция %
                //            msgType = OpenThermMessageType::READ_ACK;
                //            break;

                /*----------------------------Двусторонние информационные сообщения----------------------------*/
                /*        case OpenThermMessageID::DayTime:
                            if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                                msgType = OpenThermMessageType::READ_ACK;
                            else
                                msgType = OpenThermMessageType::WRITE_ACK;
                            break;
                        case OpenThermMessageID::Date:
                            if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                                msgType = OpenThermMessageType::READ_ACK;
                            else
                                msgType = OpenThermMessageType::WRITE_ACK;
                            break;
                        case OpenThermMessageID::Year:
                            if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                                msgType = OpenThermMessageType::READ_ACK;
                            else
                                msgType = OpenThermMessageType::WRITE_ACK;
                            break;
                        // ========>>>>>>>>>>>   СБРОС КОЛИЧЕСТВА 0 от мастера
                        case OpenThermMessageID::BurnerStarts: // Количество стартов горелки
                            if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                                msgType = OpenThermMessageType::READ_ACK;
                            else
                                msgType = OpenThermMessageType::WRITE_ACK;
                            break;
                        case OpenThermMessageID::CHPumpStarts: // Количество стартов насоса СО
                            if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                                msgType = OpenThermMessageType::READ_ACK;
                            else
                                msgType = OpenThermMessageType::WRITE_ACK;
                            break;
                        case OpenThermMessageID::DHWPumpValveStarts: // Количество стартов насоса/клапана ГВС
                            if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                                msgType = OpenThermMessageType::READ_ACK;
                            else
                                msgType = OpenThermMessageType::WRITE_ACK;
                            break;
                        case OpenThermMessageID::DHWBurnerStarts: // Количество стартов горелки ГВС
                            if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                                msgType = OpenThermMessageType::READ_ACK;
                            else
                                msgType = OpenThermMessageType::WRITE_ACK;
                            break;
                        case OpenThermMessageID::BurnerOperationHours: // часы работы горелки
                            if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                                msgType = OpenThermMessageType::READ_ACK;
                            else
                                msgType = OpenThermMessageType::WRITE_ACK;
                            break;
                        case OpenThermMessageID::CHPumpOperationHours: // часы работы горелки СО
                            if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                                msgType = OpenThermMessageType::READ_ACK;
                            else
                                msgType = OpenThermMessageType::WRITE_ACK;
                            break;
                        case OpenThermMessageID::DHWPumpValveOperationHours: // часы работы насоса/клапана ГВС
                            if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                                msgType = OpenThermMessageType::READ_ACK;
                            else
                                msgType = OpenThermMessageType::WRITE_ACK;
                            break;
                        case OpenThermMessageID::DHWBurnerOperationHours: // часы работы горелки ГВС
                            if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                                msgType = OpenThermMessageType::READ_ACK;
                            else
                                msgType = OpenThermMessageType::WRITE_ACK;
                            break;
                */
                /*------------------------------------ ВСЁ ------------------------------------*/

            default:
                msgType = OpenThermMessageType::UNKNOWN_DATA_ID;
                break;
            }
            ot_response = ot_driver->buildResponse(msgType, id, data);
            ot_driver->sendResponse(ot_response);
            if (_debug > 1)
            {
                SerialPrint("i", "OpenThermSlave ->", String(millis()) + " ID: " + String(id) + " / responseHEX: " + String(ot_response, HEX) + " / response: " + String(ot_response));
            }
        }

        ~OpenThermSlave()
        {
            delete ot_driver;
            ot_driver = nullptr;
        }
    };

    /*
     * =========================================================================================
     *                    КЛАСС УПРАВЛЕНИЯ ГВС
     * =========================================================================================
     */

    class DHWControl : public IoTItem
    {
    private:
        //     unsigned long ts = 0;

    public:
        DHWControl(String parameters) : IoTItem(parameters)
        {
            SerialPrint("i", F("DHWControl"), " START...   ");

            jsonRead(parameters, "minDhw", conf.minDhw);
            jsonRead(parameters, "maxDhw", conf.maxDhw);

            jsonRead(parameters, "gistDhw", conf.gistDhw);

            jsonRead(parameters, "numStepDhw", conf.numStepDhw);

            String tmpID;
            if (jsonRead(parameters, "idSetDHW", tmpID))
                _idSetDHW = findIoTItem(tmpID);
            if (jsonRead(parameters, "idStateDHW", tmpID))
                _idStateDHW = findIoTItem(tmpID);
            if (jsonRead(parameters, "idCmdDHW", tmpID))
                _idCmdDHW = findIoTItem(tmpID);
            if (jsonRead(parameters, "idTDhw", tmpID))
                _idTDhw = findIoTItem(tmpID);
            if (jsonRead(parameters, "idReleDhw", tmpID))
                _releDhw = findIoTItem(tmpID);
            if (_releDhw)
            {
                SerialPrint("i", "DHWControl", "Инициализировано РЕЛЕ ГВС");
                conf.dhw = true;
            }
        }
        /*
                void configured()
                {
                    static bool isFirstDhW = true;
                    if (isFirstDhW)
                    {
                        isFirstDhW = false;

                        String tmpID;
                        if (jsonRead(parameters, "idSetDHW", tmpID))
                            _idSetDHW = findIoTItem(tmpID);
                        if (jsonRead(parameters, "idStateDHW", tmpID))
                            _idStateDHW = findIoTItem(tmpID);
                        if (jsonRead(parameters, "idCmdDHW", tmpID))
                            _idCmdDHW = findIoTItem(tmpID);
                        if (jsonRead(parameters, "idTDhw", tmpID))
                            _idTDhw = findIoTItem(tmpID);
                        if (jsonRead(parameters, "idReleDhw", tmpID))
                            _releDhw = findIoTItem(tmpID);
                        if (_releDhw){
                            SerialPrint("i", "DHWControl", "Инициализировано РЕЛЕ ГВС");
                            conf.dhw = true;
                        }
                    }
                }
                */
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
                if (_releDhw)
                {
                    if (_idTDhw)
                        state.Tdhw = ::atof(_idTDhw->getValue().c_str());

                    if (set.TSetDhw - state.Tdhw >= conf.gistDhw && state.Tdhw > -5 && state.Tdhw < 120)
                    {
                        // включаем ГВС
                        //_releDhw->setValue("1", true);
                        updateReleState(_releDhw, 1);
                        state.stateDHW = 1;
                        state.stateCH = 0;
                        state.numStepOn = conf.numStepDhw;
                        if (stepMap.find(conf.numStepDhw) != stepMap.end())
                            state.RelModLevel = conf.prcOnekWt * stepMap[conf.numStepDhw].pwr;
                        else
                            state.RelModLevel = 0;
                    }
                    else
                    {
                        //_releDhw->setValue("0", true);
                        updateReleState(_releDhw, 0);
                        state.stateDHW = 0;
                        state.RelModLevel = 0;
                    }
                }
            }
            else
            {
                if (_releDhw)
                    //_releDhw->setValue("0", true);
                    updateReleState(_releDhw, 0);
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
            //    configured();
        }

        // Комманды из сценария
        IoTValue execute(String command, std::vector<IoTValue> &param)
        {
            IoTValue val;
            if (command == "SetDHW")
            {
                if (param.size())
                {
                    set.TSetDhw = param[0].valD;
                    set.TSetDhw = constrain(set.TSetDhw, conf.minDhw, conf.maxDhw);
                    // publishNew("SetDHW", String(set.TSetDhw));
                    val.valD = set.TSetDhw;
                    if (_idSetDHW)
                        _idSetDHW->setValue(val, true);
                    SerialPrint("i", "DHWControl", "Scenario DHWSet ");
                }
            }
            else if (command == "DHWEnable")
            {
                if (param.size())
                {
                    set.cmd_dhwEnable = param[0].valD;
                    val.valD = set.cmd_dhwEnable;
                    if (_idCmdDHW)
                        _idCmdDHW->setValue(val, true);
                    SerialPrint("i", "DHWControl", "Scenario DHWEnable ");
                }
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
        //      unsigned long ts = 0;

    public:
        BoilerControl(String parameters) : IoTItem(parameters)
        {
            SerialPrint("i", F("BoilerControl"), " START...   ");

            jsonRead(parameters, "debug", _debug);
            jsonRead(parameters, "telegram", _telegram);

            //            jsonRead(parameters, "Pump", conf.pump);
            jsonRead(parameters, "changeRele", conf.changeRele);

            jsonRead(parameters, "minCH", conf.minCH);
            jsonRead(parameters, "maxCH", conf.maxCH);
            jsonRead(parameters, "gistCH", conf.gistCH);
            jsonRead(parameters, "antiFreez", conf.antiFreez);

            jsonRead(parameters, "maxKW", conf.maxKW);
            // configuration();
            state.fl_flame = state.stateCH = 0;
            conf.countRele = conf.prcOnekWt = 0;

            String tmpID;
            if (jsonRead(parameters, "idPID", tmpID))
                _idPID = findIoTItem(tmpID);
            if (jsonRead(parameters, "idTboiler", tmpID))
                _idTboiler = findIoTItem(tmpID);
            if (jsonRead(parameters, "idTret", tmpID))
                _idTret = findIoTItem(tmpID);
            if (jsonRead(parameters, "idToutside", tmpID))
                _idToutside = findIoTItem(tmpID);

            if (jsonRead(parameters, "idStateCH", tmpID))
                _idStateCH = findIoTItem(tmpID);
            if (jsonRead(parameters, "idStateFlame", tmpID))
                _idStateFlame = findIoTItem(tmpID);
            if (jsonRead(parameters, "idModLevel", tmpID))
                _idModLevel = findIoTItem(tmpID);
            if (jsonRead(parameters, "idCmdCH", tmpID))
                _idCmdCH = findIoTItem(tmpID);
            if (jsonRead(parameters, "idSetCH", tmpID))
                _idSetCH = findIoTItem(tmpID);
            if (jsonRead(parameters, "idCtrlType", tmpID))
                _idCtrlType = findIoTItem(tmpID);
            if (jsonRead(parameters, "idRelePump", tmpID))
                _relePump = findIoTItem(tmpID);
            if (_relePump)
                SerialPrint("i", "BoilerControl", "Initialized relay pump");

            updateStateboiler();
        }
        /*
                void configured()
                {
                    static bool isFirst = true;
                    if (isFirst)
                    {
                        isFirst = true;

                        String tmpID;
                        if (jsonRead(parameters, "idPID", tmpID))
                            _idPID = findIoTItem(tmpID);
                        if (jsonRead(parameters, "idTboiler", tmpID))
                            _idTboiler = findIoTItem(tmpID);
                        if (jsonRead(parameters, "idTret", tmpID))
                            _idTret = findIoTItem(tmpID);
                        if (jsonRead(parameters, "idToutside", tmpID))
                            _idToutside = findIoTItem(tmpID);

                        if (jsonRead(parameters, "idStateCH", tmpID))
                            _idStateCH = findIoTItem(tmpID);
                        if (jsonRead(parameters, "idStateFlame", tmpID))
                            _idStateFlame = findIoTItem(tmpID);
                        if (jsonRead(parameters, "idModLevel", tmpID))
                            _idModLevel = findIoTItem(tmpID);
                        if (jsonRead(parameters, "idCmdCH", tmpID))
                            _idCmdCH = findIoTItem(tmpID);
                        if (jsonRead(parameters, "idSetCH", tmpID))
                            _idSetCH = findIoTItem(tmpID);
                        if (jsonRead(parameters, "idCtrlType", tmpID))
                            _idCtrlType = findIoTItem(tmpID);
                        if (jsonRead(parameters, "idRelePump", tmpID))
                            _relePump = findIoTItem(tmpID);
                        if (_relePump)
                            SerialPrint("i", "BoilerControl", "Initialized relay pump");
                    }
                }
        */
        // ============================== ЛОГИКА РАБОТЫ КОТЛА И ВКЛЮЧЕНИЯ ТЭНОВ ======================================

        // Работы котла и включения тэнов
        static void
        logicPowerOn()
        {
            // TODO ВКЛЮЧЕНИЕ РЕЛЕ ПО ИХ МОЩНОСТИ (СДЕЛАТЬ ШАГИ НАГРЕВА И КОМБИНАЦИИ ВКЛБЧЕНИЯ ТЕНОВ С РАЗНОЙ МОЩНОСТЬЮ

            // state.numStepOn = 0;
            state.RelModLevel = 0;
            pid = 0;
            state.fl_flame = false;
            // обнуляем ГВС
            state.stateDHW = 0;

            conf.countRele = mapRele.size(); // vectorRele.size();
            if (_debug > 0)
                SerialPrint("I", "SmartBoiler", " countRele = " + String(conf.countRele));
            if (conf.maxKW)
            {
                // conf.prcOnekWt /= conf.countRele;
                conf.prcOnekWt = 100 / conf.maxKW;
                // SerialPrint("i", "BoilerControl", "Процент одного кВт = " + String (conf.prcOnekWt) + "%");
            }
            if (conf.countRele == 1 || _idPID == nullptr)
                conf.ctrlType = true;

            // IoTValue val;
            // val.valD = conf.ctrlType;
            if (_idCtrlType)
            {
                if (conf.ctrlType == 0)
                    _idCtrlType->setValue("Модуляция", true);
                else
                    _idCtrlType->setValue("Вкл/Выкл", true);
            }
            if (_idPID)
                pid = ::atof(_idPID->getValue().c_str());

            if (conf.antiFreez > 0 && state.Tboiler < (conf.antiFreez + 5))
            {
                state.antiFreezOn = true;
                state.numStepOn = 1;
                state.RelModLevel = conf.prcOnekWt * stepMap[1].pwr;
                publishNew("status", "Анти-заморозка");
                SerialPrint("i", "BoilerControl", "Режим анти-заморозка");
                sendTelegramm("Режим анти-заморозка");
            }
            else
            {
                if (state.antiFreezOn)
                {
                    state.numStepOn = 0;
                    state.RelModLevel = 0;
                    state.antiFreezOn = false;
                }
            }

            if (state.Tboiler < conf.maxCH)
            {
                if (!state.antiFreezOn)
                {
                    if (conf.autoPower)
                    {
                        // if (dhw_ctrl)
                        //{
                        //  если есть модуль ГВС, то вызываем его логику включения тэнов
                        DHWControl::logicPowerDhw();
                        //}
                        if (!state.stateDHW) // Если уже включено ГВС, то нечего смотреть на отопление
                        {
                            if (set.cmd_chEnable && stepMap.size() > 0)
                            {
                                SerialPrint("I", "SmartBoiler", " stepMap.size = " + String(stepMap.size()));
                                publishNew("status", "Штатный режим");
                                // включаем отопление
                                state.stateCH = 1;

                                float TMAXcompare;
                                if (state.numStepOn == 0)
                                { // сейчас нагрев выключен
                                    TMAXcompare = set.TSetCH - conf.gistCH;
                                }
                                else
                                { // сейчас нагрев работает
                                    TMAXcompare = set.TSetCH;
                                }

                                if (state.Tboiler <= TMAXcompare && state.Tboiler > -5 && state.Tboiler < 120)
                                {
                                    if (conf.ctrlType == 0)
                                    { // режим модуляции, это когда есть модуль ПИД и более одного реле
                                        float pidStep = 100 / stepMap.size();
                                        if (_debug > 0)
                                            SerialPrint("I", "SmartBoiler", " pidStep = " + String(pidStep));
                                        for (size_t i = 0; i < stepMap.size(); i++)
                                        {
                                            if (pid > 1 && pid > i * pidStep && pid <= pidStep * (i + 1))
                                            {
                                                if (_debug > 0)
                                                {
                                                    SerialPrint("I", "SmartBoiler", " Modulation, stepON = " + String(i + 1));
                                                    SerialPrint("I", "SmartBoiler", " Modulation, pid > " + String(i * pidStep) + "; pid <= " + String(pidStep * (i + 1)));
                                                }
                                                state.numStepOn = i + 1;
                                                if (stepMap.find(state.numStepOn) != stepMap.end())
                                                    state.RelModLevel = conf.prcOnekWt * stepMap[state.numStepOn].pwr;
                                                else
                                                {
                                                    state.RelModLevel = 0;
                                                    SerialPrint("E", "SmartBoiler", "НЕТ ТАКОГО Шага: " + String(state.numStepOn));
                                                }
                                            }
                                        }
                                        if (pid >= 100)
                                        {
                                            if (_debug > 0)
                                            {
                                                SerialPrint("I", "SmartBoiler", " Modulation, stepON = " + String(stepMap.size()));
                                                SerialPrint("I", "SmartBoiler", " Modulation, pid 100% ");
                                            }
                                            state.numStepOn = stepMap.size();
                                            if (stepMap.find(state.numStepOn) != stepMap.end())
                                                state.RelModLevel = 100;
                                            else
                                            {
                                                state.RelModLevel = 0;
                                                SerialPrint("E", "SmartBoiler", "НЕТ ТАКОГО Шага: " + String(state.numStepOn));
                                            }
                                        }
                                    }
                                    else
                                    { // у нас релейный режим без ПИДа или с одним реле(тэном)
                                      // TODO СЕЙЧАС ВКЛЮЧАЕМ ПЕРВЫЙ ШАГ НАГРЕВА, НАДО ЧТО ТО ПРИДУМАТЬ УМНОЕ
                                        state.numStepOn = 1;
                                        state.RelModLevel = conf.prcOnekWt * stepMap[1].pwr;
                                        if (_debug > 0)
                                            SerialPrint("I", "SmartBoiler", " On|Off, stepON = " + String(state.numStepOn));
                                    }
                                }
                                else
                                {
                                    state.numStepOn = 0;
                                    state.RelModLevel = 0;
                                }
                            }
                            else
                            {
                                publishNew("status", "Выкл отопление");
                                state.numStepOn = 0;
                                state.RelModLevel = 0;
                                state.stateCH = 0;
                                if (_debug > 0)
                                    SerialPrint("I", "SmartBoiler", " Выкл отопление ");
                            }
                        }
                    }
                    else
                    {
                        publishNew("status", "Ручной режим");
                        if (stepMap.find(state.numStepOn) != stepMap.end())
                            state.RelModLevel = conf.prcOnekWt * stepMap[state.numStepOn].pwr;
                        else
                        {
                            SerialPrint("E", "SmartBoiler", "НЕТ ТАКОГО Шага: " + String(state.numStepOn));
                            set.cmd_chEnable = 0;
                        }

                        if (!set.cmd_chEnable)
                        {
                            state.numStepOn = 0;
                            state.RelModLevel = 0;
                            state.stateCH = 0;
                        }
                        if (_debug > 0)
                            SerialPrint("I", "SmartBoiler", " Ручной режим, stepON = " + String(state.numStepOn));
                    }
                }
            }
            else
            {
                state.numStepOn = 0;
                state.RelModLevel = 0;
                state.stateCH = 0;
                SerialPrint("i", "BoilerControl", "Выключились по макс. температуре, Tboiler = " + String(state.Tboiler));
            }

            static bool prev_flame;

            if (state.numStepOn > 0)
            {
                state.fl_flame = true; // если хоть одно реле включено, то выставляем флаг горелки
                // если хоть одно реле включено, то включаем насос
                if (_relePump)
                    //_relePump->setValue("1", true);
                    updateReleState(_relePump, 1);
            }
            else
            {
                // если все реле выключены
                if (_relePump)
                {
                    if (state.Tboiler > conf.minCH && set.cmd_chEnable) // НО температура ещё горячая и при этом отопление включено, то включаем насос
                        //_relePump->setValue("1", true);
                        updateReleState(_relePump, 1);
                    else // Если температура в котле уже остыла, или отопление нам не нужно (летом нагрели воду-пусть котел сам остывает без насоса), то выключаем насос
                        //_relePump->setValue("0", true);
                        updateReleState(_relePump, 0);
                }
            }

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

            // onStepPower(state.numStepOn);
            if (state.numStepOn > 0)
            {
                if (stepMap.find(state.numStepOn) == stepMap.end())
                {
                    SerialPrint("E", "SmartBoiler", "НЕТ ТАКОГО Шага: " + String(state.numStepOn));
                    state.numStepOn = 1;
                }
                bool releon;
                if (conf.changeRele)
                {
                    // количество реле в текущем шаге
                    int countOn = stepMap[state.numStepOn].listIDRele.size();
                    bool crOn = false;
                    // перебираем все реле
                    int i = 0;
                    for (auto it = mapRele.begin(); it != mapRele.end(); it++)
                    {
                        crOn = false;
                        // перебираем сколько реле нам нужно включить в текущем шаге
                        for (size_t c = 0; c < countOn; c++)
                        {
                            // state.currentRele - с какого реле нужно начать включение
                            int cr = state.currentRele + c;
                            if (cr >= mapRele.size())
                                cr = 0;
                            if (i == cr)
                                crOn = true;
                        }
                        if (crOn)
                        {
                            updateReleState(it->second, 1);
                        }
                        else
                        {
                            updateReleState(it->second, 0);
                        }
                        i++;
                    }
                }
                else
                {
                    for (auto it = mapRele.begin(); it != mapRele.end(); it++)
                    {
                        releon = false;
                        for (size_t r = 0; r < stepMap[state.numStepOn].listIDRele.size(); r++)
                        {
                            if (stepMap[state.numStepOn].listIDRele[r] == it->first)
                                releon = true;
                        }
                        if (releon)
                            updateReleState(it->second, 1);
                        else
                            updateReleState(it->second, 0);
                    }
                }
            }
            else
            {
                for (auto it = mapRele.begin(); it != mapRele.end(); it++)
                {
                    updateReleState(it->second, 0);
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

            publishNew("autoPower", String(conf.autoPower));

            // setValue(String(stepMap[state.numStepOn].pwr));
            //   publishNew("ModLevel", String(state.RelModLevel));
            //   publishNew("StateCH", String(state.stateCH));
            //   publishNew("controlType", String(conf.ctrlType));
            //   publishNew("StateFlame", String(state.fl_flame));
        }

        //============================== ОБЕСПЕЧЕНИЕ РАБОТЫ IoTMANAGER =====================================
        /*
                // конфигурирование котла в зависимости от настроек
                void configuration()
                {
                    state.fl_flame = state.stateCH = 0;
                    conf.countRele = conf.prcOnekWt = 0;

                    if (conf.pump)
                    {
                        _relePump = findIoTItem("_relePump");
                        if (_relePump)
                        {
                            SerialPrint("i", "BoilerControl", "Initialized relay pump");
                        }
                    }

                    updateStateboiler();
                }
        */
        void addStepPower(std::vector<IoTValue> &param)
        {
            // int step = param[0].valD;
            // float pwr = param[1].valD;
            SerialPrint("i", "BoilerControl", "Add Step Power - " + param[0].valS + " , power - " + param[1].valS);
            StepPower step; // = new StepPower;
            step.pwr = param[1].valD;
            for (size_t i = 2; i < param.size(); i++)
            {
                step.listIDRele.push_back(param[i].valS);
                tmp = findIoTItem(param[i].valS);
                if (tmp)
                {
                    // vectorRele.push_back(tmp);
                    mapRele[tmp->getID()] = tmp;

                    SerialPrint("i", "BoilerControl", "initialized relay - " + tmp->getID());
                }
                else
                {
                    SerialPrint("E", "BoilerControl", "Error initialized relay - " + param[i].valS);
                }
            }
            stepMap[(int)param[0].valD] = step;
        }

        void doByInterval()
        {
            // configured();
            updateStateboiler();
            logicPowerOn();
            if (stepMap.find(state.numStepOn) != stepMap.end())
                setValue(String(stepMap[state.numStepOn].pwr));
            else
                setValue("0");
        }

        /*
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
        */
        // Комманды из сценария
        IoTValue execute(String command, std::vector<IoTValue> &param)
        {
            IoTValue val;
            if (command == "CHSet")
            {
                if (param.size())
                {
                    set.TSetCH = param[0].valD;
                    set.TSetCH = constrain(set.TSetCH, conf.minCH, conf.maxCH);
                    val.valD = set.TSetCH;
                    if (_idSetCH)
                        _idSetCH->setValue(val, true);
                    // publishNew("SetCH", String(set.TSetCH));
                    SerialPrint("i", "BoilerControl", "Scenario CHSet ");
                }
            }
            else if (command == "CHEnable")
            {
                if (param.size())
                {
                    set.cmd_chEnable = param[0].valD;
                    val.valD = set.cmd_chEnable;
                    if (_idCmdCH)
                        _idCmdCH->setValue(val, true);
                    SerialPrint("i", "BoilerControl", "Scenario CHEnable ");
                }
            }
            else if (command == "addStepPower")
            {
                if (param.size() > 2)
                {
                    // addstep(step, power, param);
                    addStepPower(param);
                    // SerialPrint("i", "BoilerControl", "Scenario addStep ");
                }
            }
            else if (command == "onStepPower")
            {
                if (param.size())
                {
                    conf.autoPower = false;
                    // step[param[0].valD] = true;
                    // onStepPower(param[0].valD);
                    state.numStepOn = param[0].valD;
                    set.cmd_chEnable = true;
                    val.valD = set.cmd_chEnable;

                    if (_idCmdCH)
                        _idCmdCH->setValue(val, true);
                    SerialPrint("i", "BoilerControl", "Scenario onStep, Hand Power On ");
                }
            }
            else if (command == "autoPower")
            {

                conf.autoPower = true;
                SerialPrint("i", "BoilerControl", "Auto Power On");
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
            stepMap.clear();
            // vectorRele.clear();
            mapRele.clear();
        }
    };
}

void *getAPI_SmartBoiler(String subtype, String param)
{
    if (subtype == F("BoilerControl"))
    {
        return new _Boiler_v2::BoilerControl(param);
    }
    else if (subtype == F("DHWControl"))
    {
        return new _Boiler_v2::DHWControl(param);
    }
    else if (subtype == F("OpenThermSlave"))
    {
        return new _Boiler_v2::OpenThermSlave(param);
    }

    else
    {
        return nullptr;
    }
}
