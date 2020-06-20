#pragma once

#include <Arduino.h>
#include <FS.h>

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