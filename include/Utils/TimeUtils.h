#pragma once

#include <Arduino.h>

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

String GetTimeWOsec();

String GetDate();

String GetDataDigital();

int timeToMin(String Time);

const String prettyMillis(unsigned long time_ms = millis());
