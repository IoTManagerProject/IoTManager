#pragma once

#include <Arduino.h>

//#include "FS.h"

#ifdef ESP32
#include "LITTLEFS.h"
#define LittleFS LITTLEFS
#endif
#ifdef ESP8266
#include <LittleFS.h>
#endif

class FileHelper {
   public:
    FileHelper(const String filename);
    /*
    * Проверить существование
    */
    void exists();
    /*
    * Удалить файл
    */
    void remove();
    /*
    * Открыть файл установить позицию @position
    */
    File seek(size_t position = 0);

    /*
    * Чтение строки с содержащей @substr
    */
    String readFileString(const String substr);

    /* 
    * Добовление строки @str в файл 
    */
    String appendStr(const String str);

    /*
    * Запись строки
    */
    String writeStr(const String);

    /*
    *  Чтение в строку
    */
    String readStr(size_t);

    /*
    *  Размер в байтах
    */
    size_t getSize();
};