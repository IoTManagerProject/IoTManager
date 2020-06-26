#pragma once

#include <Arduino.h>

#include "CommonTypes.h"

#define ONE_MINUTE_s 60
#define ONE_HOUR_m 60
#define ONE_HOUR_s 60 * ONE_MINUTE_s
#define LEAP_YEAR(Y) (((1970 + Y) > 0) && !((1970 + Y) % 4) && (((1970 + Y) % 100) || !((1970 + Y) % 400)))
#define MIN_DATETIME 1575158400
#define ONE_SECOND_ms 1000

/*
* Время (мс) прошедщее с @since
*/
unsigned long millis_since(unsigned long sinse);

/*
* Интерввал времени (мс) между @start и @finish 
*/
unsigned long millis_passed(unsigned long start, unsigned long finish);

/*
* Форматиронное время интервала (мс)
* "чч:мм:cc", 
* "дд чч:мм", если > 24 часов
*/
const String prettyMillis(unsigned long time_ms = millis());

/*
* Форматиронное время интервала (c)
* "чч:мм:cc", 
* "дд чч:мм", если > 24 часов
*/
const String prettySeconds(unsigned long time_s);

/*
* Тайм зона в секундах
*/
int getOffsetInSeconds(int timezone);

/*
* Тайм зона в минутах
*/
int getOffsetInMinutes(int timezone);

/*
* Разбивает время на составляющие
*/
void breakEpochToTime(unsigned long epoch, Time_t& tm);