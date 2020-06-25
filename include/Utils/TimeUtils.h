#pragma once
#ifdef ESP8266
#include <time.h>
#endif
#include <Arduino.h>

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

/*
* Время (мс) прошедщее с @since
*/
unsigned long millis_since(unsigned long sinse);

/*
* Интерввал времени (мс) между @start и @finish 
*/
unsigned long millis_passed(unsigned long start, unsigned long finish);

int getOffsetInSeconds(int timezone);

int getOffsetInMinutes(int timezone);
