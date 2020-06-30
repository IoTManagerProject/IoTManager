#include "Sensor\DHTSensor.h"

DHTesp dht;
String dhtT_value_name;
String dhtH_value_name;

const String comfortStr(ComfortState value) {
    String res;
    switch (value) {
        case Comfort_OK:
            res = F("Отлично");
            break;
        case Comfort_TooHot:
            res = F("Очень жарко");
            break;
        case Comfort_TooCold:
            res = F("Очень холодно");
            break;
        case Comfort_TooDry:
            res = F("Очень сухо");
            break;
        case Comfort_TooHumid:
            res = F("Очень влажно");
            break;
        case Comfort_HotAndHumid:
            res = F("Жарко и влажно");
            break;
        case Comfort_HotAndDry:
            res = F("Жарко и сухо");
            break;
        case Comfort_ColdAndHumid:
            res = F("Холодно и влажно");
            break;
        case Comfort_ColdAndDry:
            res = F("Холодно и сухо");
            break;
        default:
            res = F("Неизвестно");
            break;
    };
    return res;
}

const String perceptionStr(byte value) {
    String res;
    switch (value) {
        case 0:
            res = F("Сухой воздух");
            break;
        case 1:
            res = F("Комфортно");
            break;
        case 2:
            res = F("Уютно");
            break;
        case 3:
            res = F("Хорошо");
            break;
        case 4:
            res = F("Неудобно");
            break;
        case 5:
            res = F("Довольно неудобно");
            break;
        case 6:
            res = F("Очень неудобно");
            break;
        case 7:
            res = F("Невыносимо");
        default:
            res = F("Unknown");
            break;
    }
    return res;
}