#pragma once
#include <Arduino.h>

#include "Consts.h"
#include "FileSystem.h"

/*
* Инициализация ФС
*/
bool fileSystemInit();

/*
* Удалить файл
*/
void removeFile(const String& filename);

/*
* Открыть файл на позиции
*/
File seekFile(const String& filename, size_t position = 0);

/*
* Чтение строки из файла
* возвращает стоку из файла в которой есть искомое слово found
*/
const String readFileString(const String& filename, const String& to_find);

/* 
* Добовление строки в файл
*/
const String addFileLn(const String& filename, const String& str);

/* 
* Добовление строки в файл
*/
const String addFile(const String& filename, const String& str);

/*
* Запись строки в файл
*/
const String writeFile(const String& filename, const String& str);

/*
*  Чтение файла в строку
*/
const String readFile(const String& filename, size_t max_size);

/*
*  Чтение файла в строку с записью его размера
*/
const String readFileSz(const String& filename, size_t max_size, size_t& size);

/*
*  Размер файла
*/
const String getFileSize(const String& filename);

bool copyFile(const String& src, const String& dst, bool overwrite = true);

const String getFSSizeInfo();

const String getConfigFile(uint8_t preset, ConfigType_t type);
