#include "Global.h"
#include "classes/IoTItem.h"

extern IoTGpio IoTgpio;

class ThermostatGIST : public IoTItem
{
private:
    String _set_id;         // заданная температура
    String _term_id;        // термометр
    String _term_rezerv_id; // резервный термометр
    String _rele;           // реле
    float pv_last = 0;      // предыдущая температура
    float _gist = 1;        // гистерис
    float sp, pv, pv2;
    String interim;
    int enable = 1;

public:
    ThermostatGIST(String parameters) : IoTItem(parameters)
    {
        jsonRead(parameters, "set_id", _set_id);
        jsonRead(parameters, "term_id", _term_id);
        jsonRead(parameters, "term_rezerv_id", _term_rezerv_id);
        jsonRead(parameters, "gist", _gist);
        jsonRead(parameters, "rele", _rele);
    }

    void doByInterval()
    {
        // заданная температура
        IoTItem *tmp = findIoTItem(_set_id);
        if (tmp)
        {
            interim = tmp->getValue();
            sp = ::atof(interim.c_str());
        }
        // термометр
        tmp = findIoTItem(_term_id);
        if (tmp)
        {
            interim = tmp->getValue();
            pv = ::atof(interim.c_str());
        }
        if (sp && _rele != "")
        {
            if (pv > -40 && pv < 120 && pv)
            {
                if (enable)
                {
                    setValue("штатный режим");
                }
                // работаем по основному датчику
                if (pv >= sp + _gist && enable)
                {
                    tmp = findIoTItem(_rele);
                    if (tmp)
                        tmp->setValue("0", true);
                }
                if (pv <= sp - _gist && enable)
                {
                    tmp = findIoTItem(_rele);
                    if (tmp)
                        tmp->setValue("1", true);
                }
            }
            else
            {
                // резервный термометр
                if (_term_rezerv_id != "")
                {
                    tmp = findIoTItem(_term_rezerv_id);
                    if (tmp)
                    {
                        interim = tmp->getValue();
                        pv2 = ::atof(interim.c_str());
                    }
                    // работаем по резервному датчику
                    if (pv2 > -40 && pv2 < 120 && pv2)
                    {
                        if (enable)
                        {
                            setValue("резервный датчик");
                        }
                        if (pv2 >= sp + _gist && enable)
                        {
                            tmp = findIoTItem(_rele);
                            if (tmp)
                                tmp->setValue("0", true);
                        }
                        if (pv2 <= sp - _gist && enable)
                        {
                            tmp = findIoTItem(_rele);
                            if (tmp)
                                tmp->setValue("1", true);
                        }
                    }
                    else
                    {
                        if (enable)
                        {
                            setValue("ошибка резервного датчика");
                        }
                    }
                }
                else
                {
                    if (enable)
                    {
                        setValue("ошибка датчика температуры");
                    }
                }
            }
        }
        else
        {
            // если не заполнены настройки термостата
            setValue("ошибка настройки термостата");
        }

        pv_last = pv;
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (param.size() == 1)
        {
            if (command == "enable")
            {
                if (param.size())
                {
                    enable = param[0].valD;
                    if (enable)
                    {
                        setValue("включен");
                    }
                    else
                    {
                        setValue("выключен");
                    }
                }
            }
        }
        return {};
    }

    ~ThermostatGIST(){};
};

class ThermostatPID : public IoTItem
{
private:
    String _set_id;  // заданная температура
    String _term_id; // термометр
    float _int, _KP, _KI, _KD, sp, pv,
        pv_last = 0, // предыдущая температура
        ierr = 0,    // интегральная погрешность
        dt = 0;      // время между измерениями
    String _rele;    // реле
    String interim;
    int enable = 1;
    long interval;
    IoTItem *tmp;
    int releState = 0;

public:
    ThermostatPID(String parameters) : IoTItem(parameters)
    {
        jsonRead(parameters, "set_id", _set_id);
        jsonRead(parameters, "term_id", _term_id);
        jsonRead(parameters, "int", _int);
        jsonRead(parameters, "KP", _KP);
        jsonRead(parameters, "KI", _KI);
        jsonRead(parameters, "KD", _KD);
        jsonRead(parameters, F("int"), interval);
        interval = interval * 1000; // интервал проверки  в сек
        jsonRead(parameters, "rele", _rele);
    }

protected:
    //===============================================================
    // Вычисляем температуру контура отпления, коэффициенты ПИД регулятора
    //===============================================================
    float pid(float sp, float pv, float pv_last, float &ierr, float dt)
    {
        float Kc = _KP;   // K / %Heater 5
        float tauI = _KI; // sec 50
        float tauD = _KD; // sec 1
        // ПИД коэффициенты
        float KP = Kc; // 5
        if (tauI == 0)
        {
            tauI = 50;
        }
        float KI = Kc / tauI; // 0.1
        float KD = Kc * tauD; // 5
        // верхняя и нижняя границы уровня нагрева
        float ophi = 100;
        float oplo = 0;
        // вычислить ошибку
        float error = sp - pv; // 0
        // calculate the integral error
        ierr = ierr + KI * error * dt; // 0
        // вычислить производную измерения
        float dpv = (pv - pv_last) / dt; // 0
        // рассчитать выход ПИД регулятора
        float P = KP * error; // пропорциональная составляющая
        float I = ierr;       // интегральная составляющая
        float D = -KD * dpv;  // дифференциальная составляющая
        float op = P + I + D;
        // защита от сброса
        if ((op < oplo) || (op > ophi))
        {
            I = I - KI * error * dt;
            // выход регулятора, он же уставка для ID-1 (температура теплоносителя контура СО котла)
            op = constrain(op, oplo, ophi);
        }

        ierr = I;
        return op;
    }

    void
    doByInterval()
    {
        // заданная температура
        IoTItem *tmp = findIoTItem(_set_id);
        if (tmp)
        {
            interim = tmp->getValue();
            sp = ::atof(interim.c_str());
        }
        // термометр
        tmp = findIoTItem(_term_id);
        if (tmp)
        {
            interim = tmp->getValue();
            pv = ::atof(interim.c_str());
        }
        if (sp && pv)
        {
            value.valD = pid(sp, pv, pv_last, ierr, _int);
            value.valS = (String)(int)pid(sp, pv, pv_last, ierr, _int);
            regEvent(value.valS, "ThermostatPID", false, true);
                }
        pv_last = pv;
    }

    void loop()
    {
        if (enableDoByInt)
        {
            currentMillis = millis();
            difference = currentMillis - prevMillis;

            if (_rele != "" && enable && value.valD * interval / 100000 > difference / 1000 && releState == 0)
            {
                releState = 1;
                tmp = findIoTItem(_rele);
                if (tmp)
                    tmp->setValue("1", true);
            }
            if (_rele != "" && enable && value.valD * interval / 100000 < difference / 1000 && releState == 1)
            {
                releState = 0;
                tmp = findIoTItem(_rele);
                if (tmp)
                    tmp->setValue("0", true);
            }

            if (difference >= interval)
            {
                prevMillis = millis();
                this->doByInterval();
            }
        }
    }
    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (param.size() == 1)
        {
            if (command == "enable")
            {
                if (param.size())
                {
                    enable = param[0].valD;
                }
            }
            if (command == "KP")
            {
                if (param.size())
                {
                    _KP = param[0].valD;
                }
            }
            if (command == "KI")
            {
                if (param.size())
                {
                    _KI = param[0].valD;
                }
            }
            if (command == "KD")
            {
                if (param.size())
                {
                    _KD = param[0].valD;
                }
            }
        }
        return {};
    }
    ~ThermostatPID(){};
};

class ThermostatETK : public IoTItem
{
private:
    float pv, sp, outside_temp;
    float _iv_k;        // эквитермические кривые
    String _set_id;     // заданная температура
    String _term_id;    // термометр
    String _outside_id; // уличный термометр
    String interim;
    int enable = 1;

public:
    ThermostatETK(String parameters) : IoTItem(parameters)
    {
        jsonRead(parameters, "set_id", _set_id);
        jsonRead(parameters, "term_id", _term_id);
        jsonRead(parameters, "iv_k", _iv_k);
        jsonRead(parameters, "outside_id", _outside_id);
    }

protected:
    //===================================================================================================================
    //       Вычисляем температуру контура отпления, эквитермические кривые
    //===================================================================================================================
    float curve(float iv_k, float outside_temp)
    {
        float a = (-0.21 * iv_k) - 0.06;          // a = -0,21k — 0,06
        float b = (6.04 * iv_k) + 1.98;           // b = 6,04k + 1,98
        float c = (-5.06 * iv_k) + 18.06;         // с = -5,06k + 18,06
        float x = (-0.2 * outside_temp) + 5;      // x = -0.2*t1 + 5
        float temp_n = (a * x * x) + (b * x) + c; // Tn = ax2 + bx + c
        // Расчетная температура конура отопления
        float op = temp_n; // T = Tn
        // Ограничиваем температуру для ID-1
        op = constrain(op, 0, 100);
        return op;
    }

    void doByInterval()
    {
        //  уличный термометр
        IoTItem *tmp = findIoTItem(_outside_id);
        if (tmp)
        {
            interim = tmp->getValue();
            outside_temp = ::atof(interim.c_str());
        }
        if (_iv_k && outside_temp)
        {

            value.valD = curve(_iv_k, outside_temp);
            regEvent(value.valD, "ThermostatETK");
        }
    }

    ~ThermostatETK(){};
};

class ThermostatETK2 : public IoTItem
{
private:
    float pv, sp, outside_temp;
    float _iv_k;        // эквитермические кривые
    String _set_id;     // заданная температура
    String _term_id;    // термометр
    String _outside_id; // уличный термометр
    String interim;
    int enable = 1;

public:
    ThermostatETK2(String parameters) : IoTItem(parameters)
    {
        jsonRead(parameters, "set_id", _set_id);
        jsonRead(parameters, "term_id", _term_id);
        jsonRead(parameters, "iv_k", _iv_k);
        jsonRead(parameters, "outside_id", _outside_id);
    }

protected:
    //===================================================================================================================
    //       Вычисляем температуру контура отпления, эквитермические кривые с учётом влияния температуры в помещении
    //===================================================================================================================
    float curve2(float sp, float pv, float iv_k, float outside_temp)
    {
        // Расчет поправки (ошибки) термостата
        float error = sp - pv; // Tt = (Tu — T2) × 5
        float temp_t = error * 3.0;
        // Поправка на желаемую комнатную температуру
        // Температура контура отопления в зависимости от наружной температуры
        float a = (-0.21 * iv_k) - 0.06;          // a = -0,21k — 0,06
        float b = (6.04 * iv_k) + 1.98;           // b = 6,04k + 1,98
        float c = (-5.06 * iv_k) + 18.06;         // с = -5,06k + 18,06
        float x = (-0.2 * outside_temp) + 5;      // x = -0.2*t1 + 5
        float temp_n = (a * x * x) + (b * x) + c; // Tn = ax2 + bx + c
        // Расчетная температура конура отопления
        float op = temp_n + temp_t; // T = Tn + Tk + Tt
        // Ограничиваем температуру для ID-1
        op = constrain(op, 0, 100);
        return op;
    }

    void doByInterval()
    {
        // заданная температура
        IoTItem *tmp = findIoTItem(_set_id);
        if (tmp)
        {
            interim = tmp->getValue();
            sp = ::atof(interim.c_str());
        }
        // термометр
        tmp = findIoTItem(_term_id);
        if (tmp)
        {
            interim = tmp->getValue();
            pv = ::atof(interim.c_str());
        }
        //  уличный термометр
        tmp = findIoTItem(_outside_id);
        if (tmp)
        {
            interim = tmp->getValue();
            outside_temp = ::atof(interim.c_str());
        }
        if (sp && pv && _iv_k && outside_temp)
        {
            value.valD = curve2(sp, pv, _iv_k, outside_temp);
            regEvent(value.valD, "ThermostatETK2");
        }
    }

    ~ThermostatETK2(){};
};

void *getAPI_Thermostat(String subtype, String param)
{
    if (subtype == F("ThermostatGIST"))
    {
        return new ThermostatGIST(param);
    }
    else if (subtype == F("ThermostatPID"))
    {
        return new ThermostatPID(param);
    }
    else if (subtype == F("ThermostatETK"))
    {
        return new ThermostatETK(param);
    }
    else if (subtype == F("ThermostatETK2"))
    {
        return new ThermostatETK2(param);
    }
    //}

    return nullptr;
}
