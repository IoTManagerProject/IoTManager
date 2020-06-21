#pragma once

#include <Arduino.h>

#include "FS.h"

#ifdef ESP32
#include "LITTLEFS.h"
#define LittleFS LITTLEFS
#endif
#ifdef ESP8266
#include <LittleFS.h>
#endif
/*
* Инициализация ФС
*/
bool fileSystemInit();

/*
* Удалить файл
*/
void removeFile(const String filename);

/*
* Открыть файл на позиции
*/
File seekFile(const String filename, size_t position = 0);

/*
* Чтение строки из файла
* возвращает стоку из файла в которой есть искомое слово found
*/
String readFileString(const String filename, const String to_find);

/* 
* Добовление строки в файл
*/
String addFile(const String filename, const String str);

/*
* Запись строки в файл
*/
String writeFile(const String filename, const String str);

/*
*  Чтение файла в строку
*/
String readFile(const String filename, size_t max_size);

/*
*  Размер файла
*/
String getFileSize(const String filename);