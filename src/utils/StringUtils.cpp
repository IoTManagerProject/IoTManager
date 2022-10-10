#include "utils/StringUtils.h"

void writeUint8tToString(uint8_t* payload, size_t length, size_t headerLenth, String& payloadStr) {
    payloadStr = "";
    payloadStr.reserve(length + 1);
    for (size_t i = headerLenth; i < length; i++) {
        payloadStr += (char)payload[i];
    }
}

String selectToMarkerLast(String str, String found) {
    int p = str.lastIndexOf(found);
    return str.substring(p + found.length());
}

String selectToMarker(String str, String found) {
    int p = str.indexOf(found);
    return str.substring(0, p);
}

String extractInner(String str) {
    int p1 = str.indexOf("[");
    int p2 = str.indexOf("]");
    return str.substring(p1 + 1, p2);
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

String selectFromMarkerToMarker(String str, String tofind, int number) {
    if (str.indexOf(tofind) == -1) {
        return "not found";
    }
    str += tofind;  // добавим для корректного поиска
    uint8_t i = 0;  // Индекс перебора
    do {
        if (i == number) {
            // если индекс совпал с позицией
            return selectToMarker(str, tofind);
        }
        // отбросим проверенный блок до разделителя
        str = deleteBeforeDelimiter(str, tofind);
        i++;
    } while (str.length() != 0);

    return "not found";
}

//преобразовываем байтовый массив в человеческий вид HEX в строке
void hex2string(byte array[], unsigned int len, char buffer[]) {
    for (unsigned int i = 0; i < len; i++) {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i * 2 + 0] = nib1 < 0xA ? '0' + nib1 : 'A' + nib1 - 0xA;
        buffer[i * 2 + 1] = nib2 < 0xA ? '0' + nib2 : 'A' + nib2 - 0xA;
    }
    buffer[len * 2] = '\0';
}

inline unsigned char ChartoHex(char ch) {
    return ((ch >= 'A') ? (ch - 'A' + 0xA) : (ch - '0')) & 0x0F;
}

// str   - указатель на массив символов
// bytes - выходной буфер
// функция возвращает колл-во байт
//
int string2hex(const char* str, unsigned char* bytes) {
    unsigned char Hi, Lo;

    int i = 0;
    while ((Hi = *str++) && (Lo = *str++)) {
        bytes[i++] = (ChartoHex(Hi) << 4) | ChartoHex(Lo);
    }

    return i;
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

size_t itemsCount2(String str, const String& separator) {
    // если строки поиск нет сразу выход
    if (str.indexOf(separator) == -1) {
        return 0;
    }
    // добавим для корректного поиска
    str += separator;
    size_t cnt = 0;
    while (str.length()) {
        // отбросим проверенный блок до разделителя
        str = deleteBeforeDelimiter(str, separator);
        cnt++;
    }
    return cnt;
}

size_t itemsCount(String& str, const char* delim) {
    size_t cnt = 0;
    char* cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());
    char* token;
    while ((token = strtok_r(cstr, delim, &cstr))) {
        cnt++;
        // printf("%s\n", token);
    }
    delete[] cstr;
    return cnt;
}

char* stringToChar(String& str) {
    char* mychar = new char[str.length() + 1];
    strcpy(mychar, str.c_str());
    return mychar;
}

boolean isDigitStr(const String& str) {
    for (size_t i = 0; i < str.length(); i++) {
        if (!isDigit(str.charAt(i))) {
            return false;
        }
    }
    return str.length();
}

boolean isDigitDotCommaStr(const String& str) {
    bool dotCountWas = false;
    for (size_t i = 0; i < str.length(); i++) {
        char latter = str.charAt(i);
        //если символ не цифра и не точка и не тире
        if (!isDigit(latter) && latter != '.' && latter != '-') {
            return false;
        }
        if (latter == '.') {    // проверяем на две точки, чтоб исключить дату
            if (dotCountWas) return false;
            dotCountWas = true;
        }
    }
    return true;
}

String prettyBytes(size_t size) {
    if (size < 1024)
        return String(size) + "b";
    else if (size < (1024 * 1024))
        return String(size / 1024.0) + "kB";
    else if (size < (1024 * 1024 * 1024))
        return String(size / 1024.0 / 1024.0) + "MB";
    else
        return String(size / 1024.0 / 1024.0 / 1024.0) + "GB";
}

String uint64ToString(uint64_t input, uint8_t base) {
    String result = "";

    do {
        char c = input % base;
        input /= base;

        if (c < 10)
            c += '0';
        else
            c += 'A' - 10;
        result = c + result;
    } while (input);
    return result;
}
