#include "Utils\StringUtils.h"

String selectToMarkerLast(String str, String found) {
    int p = str.lastIndexOf(found);
    return str.substring(p + found.length());
}

String selectToMarker(String str, String found) {
    int p = str.indexOf(found);
    return str.substring(0, p);
}

String deleteAfterDelimiter(String str, String found) {
    int p = str.indexOf(found);
    return str.substring(0, p);
}

String deleteBeforeDelimiter(String str, String found) {
    int p = str.indexOf(found) + found.length();
    return str.substring(p);
}

String deleteBeforeDelimiterTo(String str, String found) {
    int p = str.indexOf(found);
    return str.substring(p);
}

String deleteToMarkerLast(String str, String found) {
    int p = str.lastIndexOf(found);
    return str.substring(0, p);
}

String selectToMarkerPlus(String str, String found, int plus) {
    int p = str.indexOf(found);
    return str.substring(0, p + plus);
}

String selectFromMarkerToMarker(String str, String found, int number) {
    if (str.indexOf(found) == -1) return "not found";  // если строки поиск нет сразу выход
    str += found;                                      // добавим для корректного поиска
    uint8_t i = 0;                                     // Индекс перебора
    do {
        if (i == number) return selectToMarker(str, found);  // если индекс совпал с позицией законцим вернем резултат
        str = deleteBeforeDelimiter(str, found);             // отбросим проверенный блок до разделителя
        i++;                                                 // увеличим индекс
    } while (str.length() != 0);                             // повторим пока строка не пустая
    return "not found";                                      // Достигли пустой строки и ничего не нашли
}

uint8_t hexStringToUint8(String hex) {
    uint8_t tmp = strtol(hex.c_str(), NULL, 0);
    if (tmp >= 0x00 && tmp <= 0xFF) {
        return tmp;
    }
}

uint16_t hexStringToUint16(String hex) {
    uint16_t tmp = strtol(hex.c_str(), NULL, 0);
    if (tmp >= 0x0000 && tmp <= 0xFFFF) {
        return tmp;
    }
}
