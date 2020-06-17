#pragma once

#include <Arduino.h>

extern void Timer_countdown_init();
extern void addTimer(String number, String time);
extern int readTimer(int number);
extern void delTimer(String number);
extern void timerStop();
extern void delTimer(String number);