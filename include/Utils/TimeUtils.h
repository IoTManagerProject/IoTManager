#pragma once

#include <Arduino.h>
#ifdef ESP8266
#include <TZ.h>
#endif

void Time_Init();

void time_check();

void reconfigTime();

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
* Результат выполнения 
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
