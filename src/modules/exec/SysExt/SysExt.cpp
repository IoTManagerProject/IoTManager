#include "Global.h"
#include "classes/IoTItem.h"

// Пример модуля расширения возможностей системы на примере добавления новых команд в сценарии
// При комбинации использования doByInterval() и execute() можно обеспечить интеграцию с внешними сервисами, такими как Telegram, например.
//

class SysExt : public IoTItem {
   public:
    SysExt(String parameters) : IoTItem(parameters) {
        // инициализация внутренних переменных и объектов для взаимодействия с внешними системами
        // jsonRead(parameters, "addr", addr);   // получаем параметры из настроек модуля. Наименования могут быть любыми.
    }

    void doByInterval() {
        // выполнение периодических проверок каждые Int секунд из настроек модуля

        // regEvent(Значение, Описание);   // регистрация найденного события после проверок для запуска сценариев и других реакций в системе
    }

    // void loop() {
    //  выполнение необходимых проверок в теле основного цикла программы.
    //  ВАЖНО: 1. при использовании loop() отключается doByInterval()
    //         2. любые заминки в данном цикле повлияют на общую работу системы
    //}

    IoTValue execute(String command, std::vector<IoTValue>& param) {
        // реакция на вызов команды модуля из сценария
        // String command - имя команды после ID. (ID.Команда())
        // param - вектор ("массив") значений параметров переданных вместе с командой: ID.Команда("пар1", 22, 33) -> param[0].ValS = "пар1", param[1].ValD = 22

        // if (command == "reboot") {  // выполняем код при вызове спец команды из сценария: ID.reboot();
        //     ESP.restart();
        // } else if (command == "digitalRead") {
        //     if (param.size()) {
        //         IoTgpio.pinMode(param[0].valD, INPUT);
        //         value.valD = IoTgpio.digitalRead(param[0].valD);
        //         return value;
        //     }
        // } else if (command == "analogRead") {
        //     if (param.size()) {
        //         IoTgpio.pinMode(param[0].valD, INPUT);
        //         value.valD = IoTgpio.analogRead(param[0].valD);
        //         return value;
        //     }
        // } else if (command == "digitalWrite") {
        //     if (param.size() == 2) {
        //         IoTgpio.pinMode(param[0].valD, OUTPUT);
        //         IoTgpio.digitalWrite(param[0].valD, param[1].valD);
        //         return {};
        //     }
        // } else if (command == "digitalInvert") {
        //     if (param.size()) {
        //         IoTgpio.pinMode(param[0].valD, OUTPUT);
        //         IoTgpio.digitalInvert(param[0].valD);
        //         return {};
        //     }
            //} else if (command == "getTime") {
            //    if (param.size()) {
            //        value.isDecimal = false;
            //        value.valS = watch->gettime(param[0].valS.c_str());
            //        return value;
            //    }
            //} else if (command == "getHours") {
            //    value.valD = watch->Hours;  //	Часы			0-23
            //    value.isDecimal = true;
            //    return value;
            //} else if (command == "getMinutes") {
            //    value.valD = watch->minutes;  //	Минуты			0-59
            //    value.isDecimal = true;
            //    return value;
            //} else if (command == "getSeconds") {
            //    value.valD = watch->seconds;  //	Секунды			0-59
            //    value.isDecimal = true;
            //    return value;
            //} else if (command == "getMonth") {
            //    value.valD = watch->month;  //	Месяц			1-12
            //    value.isDecimal = true;
            //    return value;
            //} else if (command == "getDay") {
            //    value.valD = watch->day;  //	День месяца		1-31
            //    value.isDecimal = true;
            //    return value;
//         } else if (command == "deepSleep") {
//             if (param.size()) {
//                 Serial.printf("Ушел спать на %d сек...", (int)param[0].valD);
// #ifdef ESP32
//                 esp_sleep_enable_timer_wakeup(param[0].valD * 1000000);
//                 delay(1000);
//                 esp_deep_sleep_start();
// #else
//                 ESP.deepSleep(param[0].valD * 1000000);
// #endif
//             }
        //    return {};
        //}
        // } else if (command == "ModemSleep") {
        //         Serial.printf("Выключил все радио...");
        //         #ifdef ESP32
        //             WiFi.setSleep(true);
        //             if (!setCpuFrequencyMhz(80)){
        //                 Serial2.println("Not valid frequency!");
        //             }
        //         #else
        //             //WiFi.disconnect();
        //             adc_power_off();
        //             WiFi.disconnect(true);  // Disconnect from the network
        //             WiFi.mode(WIFI_OFF);    // Switch WiFi off
        //         #endif
        //     return {};
        // } else if (command == "ModemWakeup") {
        //         Serial.printf("Включил все радио...");
        //         #ifdef ESP32
        //             setCpuFrequencyMhz(240);
        //         #else
        //             WiFi.forceSleepWake();
        //             delay(1);
        //             // восстанавливаем коннект тут

        //             wifi_set_sleep_type(NONE_SLEEP_T);
        //         #endif
        //     return {};
        // }
        return {};  // команда поддерживает возвращаемое значения. Т.е. по итогу выполнения команды или общения с внешней системой, можно вернуть значение в сценарий для дальнейшей обработки
    }

    ~SysExt(){};
};

void* getAPI_SysExt(String subtype, String param) {
    if (subtype == F("SysExt")) {
        return new SysExt(param);
    } else {
        return nullptr;
    }
}
