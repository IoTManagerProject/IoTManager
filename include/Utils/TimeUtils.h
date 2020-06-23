#pragma once

#include <Arduino.h>
#ifdef ESP8266
#include <TZ.h>
#endif

void Time_Init();

/*
* Получение текущего времени
*/
String getTime();
/*
* Получаем время в формате linux gmt
*/
String getTimeUnix();

/*
* Параметр время
* @result результат
*/
boolean getUnixTimeStr(String&);

String getTimeWOsec();

/*
* Получение даты
*/
String getDate();

String getDateDigitalFormated();

int timeToMin(String Time);

const String prettyMillis(unsigned long time_ms = millis());

int timeZoneInSeconds(const byte timeZone);

bool hasTimeSynced();

int getBiasInSeconds();

/*
* Время (мс) прошедщее с @simce
*/
unsigned long millis_since(unsigned long sinse);

/*
* Интерввал времени (мс) между @start и @fimish 
*/
unsigned long millis_passed(unsigned long start, unsigned long finish);