#pragma once

#include <Arduino.h>
#include <FS.h>

/*
* Чтение строки из файла
* возвращает стоку из файла в которой есть искомое слово found
*/
String readFileString(const String& filename, const String& str_to_found);

/* 
* Добовление строки в файл
*/
String addFile(const String& fileName, const String& str);

/*
* Запись строки в файл
*/
String writeFile(const String& fileName, const String& str);

/*
*  Чтение файла в строку
*/
String readFile(const String& fileName, size_t len);

/*
*  Размер файла
*/
String sizeFile(const String& fileName);