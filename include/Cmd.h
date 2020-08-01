#pragma once

#include <Arduino.h>

extern void cmd_init();
extern void itemInit();

extern void buttonOut();
extern void buttonOutSet();

extern void pwmOut();
extern void pwmOutSet();

extern void buttonIn();
extern void buttonInSet();

extern void inputDigit();
extern void inputDigitSet();

extern void handle_time_init();
extern void stepper();
extern void stepperSet();
extern void servo_();
extern void servoSet();
extern void serialBegin();
extern void serialWrite();
extern void logging();

extern void inputTime();
extern void button();
extern void timeSet();
extern void text();
extern void textSet();
extern void mqttOrderSend();
extern void httpOrderSend();
extern void firmwareVersion();
extern void firmwareUpdate();
extern void loadScenario();

extern void fileExecute(const String& filename);
extern void stringExecute(String& cmdStr);