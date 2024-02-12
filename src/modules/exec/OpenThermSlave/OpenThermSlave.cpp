#include "Global.h"
#include "classes/IoTItem.h"
#include <Arduino.h>
#include "OpenTherm.h"

#define SLAVE true
#define TIMEOUT_TRESHOLD 5

namespace _OpenThermSlave
{
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
        uint8_t fl_fail = 0;
        failCode fCode;
        float RelModLevel = 0;
        float Tboiler = 0;
        float Tret = 0;
        float Tdhw = 0;
        float Toutside = 0;
    } state;

    // конфигурация котла
    struct ConfigBoiler
    {
        bool dhw = false;               // 1- есть реле(трехходовой) ГВС
        bool ctrlType = false;          // 0 - модуляция, 1- вкл/выкл
        bool confDhw = true;            // 1 - бак, 0 - проточная //TODO ПОКА НЕ ЗНАЮ ЧТО ДЕЛАТЬ
        bool pumpControlMaster = false; // в протоколе ОТ: мастер управляет насосом ????????????????????? //TODO Команды кправления насосом от мастера не помню

        int minDhw;
        int maxDhw;
        int minCH;
        int maxCH;

    } conf;

   // DynamicJsonDocument OpenThemData(JSON_BUFFER_SIZE / 2);

    IoTItem *tmp;

        IoTItem *_idTboiler = nullptr;
        IoTItem *_idTret = nullptr;
        IoTItem *_idToutside = nullptr;
        IoTItem *_idStateCH = nullptr;
        IoTItem *_idStateDHW = nullptr;
        IoTItem *_idStateFlame = nullptr;
        IoTItem *_idModLevel = nullptr;
        IoTItem *_idTDhw = nullptr;
        IoTItem *_idCmdCH = nullptr;
        IoTItem *_idCmdDHW = nullptr;
        IoTItem *_idSetCH = nullptr;
        IoTItem *_idSetDHW = nullptr;
        IoTItem *_idCtrlType = nullptr;

    unsigned long timeout_count = 0;

    uint8_t _debug = 0;
    bool _telegram = false;
    unsigned long ot_response = 0;
    uint8_t SlaveMemberIDcode = 0;


    void publishNew(String widget, String value)
    {
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
            jsonRead(parameters, "MemberID", (int &)SlaveMemberIDcode);

            jsonRead(parameters, "LogLevel", (int &)_debug);
            jsonRead(parameters, "telegram", _telegram);

            String tmpID;
            jsonRead(parameters, "idTboiler", tmpID);
            _idTboiler = findIoTItem(tmpID);
            jsonRead(parameters, "idTret", tmpID);
            _idTret = findIoTItem(tmpID);
            jsonRead(parameters, "idToutside", tmpID);
            _idToutside = findIoTItem(tmpID);
            jsonRead(parameters, "idStateCH", tmpID);
            _idStateCH = findIoTItem(tmpID);
            jsonRead(parameters, "idStateDHW", tmpID);
            _idStateDHW = findIoTItem(tmpID);
            jsonRead(parameters, "idStateFlame", tmpID);
            _idStateFlame = findIoTItem(tmpID);
            jsonRead(parameters, "idModLevel", tmpID);
            _idModLevel = findIoTItem(tmpID);
            jsonRead(parameters, "idTDhw", tmpID);
            _idTDhw = findIoTItem(tmpID);
            jsonRead(parameters, "idCmdCH", tmpID);
            _idCmdCH = findIoTItem(tmpID);
            jsonRead(parameters, "idCmdDHW", tmpID);
            _idCmdDHW = findIoTItem(tmpID);
            jsonRead(parameters, "idCtrlType", tmpID);
            _idCtrlType = findIoTItem(tmpID);

            jsonRead(parameters, "idSetCH", tmpID);
            _idSetCH = findIoTItem(tmpID);
            jsonRead(parameters, "idSetDHW", tmpID);
            _idSetDHW = findIoTItem(tmpID);

            jsonRead(parameters, "minCH", conf.minCH);
            jsonRead(parameters, "maxCH", conf.maxCH);
            jsonRead(parameters, "minDhw", conf.minDhw);
            jsonRead(parameters, "maxDhw", conf.maxDhw);

            instance_OTdriver(_RX_pin, _TX_pin);
            ot_driver->begin(handleInterruptSlave, processRequest); // responseCallback
//            ot_boiler = this;
        }

        void doByInterval()
        {
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
            _idCmdCH->setValue(val, true);
            val.valD = set.cmd_dhwEnable;
            _idCmdDHW->setValue(val, true);
            data &= 0xFF00; // старший бит не трогаем, а младший обнулили, что бы его заполнить состоянием котла и вернуть data термостату

            //   if (_idFail)
            //      state.fl_fail = ::atof(_idFail->getValue().c_str());
            if (_idStateCH)
                state.stateCH = ::atoi(_idStateCH->getValue().c_str());
            if (_idStateDHW)
                state.stateDHW = ::atoi(_idStateDHW->getValue().c_str());
            if (_idStateFlame)
                state.fl_flame = ::atoi(_idStateFlame->getValue().c_str());

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
                    publishNew("boilerslave", "❌");
                    // publishNew("status", "не подключен");
                    timeout_count = TIMEOUT_TRESHOLD;
                    sendTelegramm(("OpenTherm: потеря связи с управляющим устройством (термостатом) ❌"));
                }
                break;
            case OpenThermResponseStatus::SUCCESS:
                timeout_count = 0;
                publishNew("boilerslave", "✅");
                // publishNew("status", "подключен");
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
            if (_idCtrlType)
                conf.ctrlType = ::atoi(_idCtrlType->getValue().c_str());
            //	unsigned long response;
            unsigned int data = ot_driver->getUInt(request);
            OpenThermMessageType msgType;
            byte ID;
            OpenThermMessageID id = ot_driver->getDataID(request);
            uint8_t flags;
            if (_debug > 2)
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
                    if (_idSetDHW)
                        set.TSetDhw = ::atof(_idSetDHW->getValue().c_str());
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
                    _idSetDHW->setValue(val, true);
                }
                break;
            case OpenThermMessageID::TSet: // TODO Получили температуру СО
                if (ot_driver->getMessageType(request) == OpenThermMessageType::READ_DATA)
                {
                    msgType = OpenThermMessageType::READ_ACK;
                    if (_idSetCH)
                        set.TSetCH = ::atof(_idSetCH->getValue().c_str());
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
                if (_idModLevel)
                    state.RelModLevel = ::atoi(_idModLevel->getValue().c_str());
                data = ot_driver->temperatureToData(state.RelModLevel);
                break;
            case OpenThermMessageID::Tboiler: // запрос температуры котла
                msgType = OpenThermMessageType::READ_ACK;
                if (_idTboiler)
                    state.Tboiler = ::atof(_idTboiler->getValue().c_str());
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
            if (_debug > 2)
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
}


void *getAPI_OpenThermSlave(String subtype, String param)
{
    if (subtype == F("OpenThermSlave"))
    {
        return new _OpenThermSlave::OpenThermSlave(param);
    }
    else
    {
        return nullptr;
    }
}
