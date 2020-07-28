#pragma once

#include <Arduino.h>

extern void cmd_init();
extern void itemInit();
extern void button();
extern void buttonSet();
extern void buttonChange();
extern void pinSet();
extern void pinChange();
extern void handle_time_init();
extern void pwm();
extern void switch_();
extern void pwmSet();
extern void stepper();
extern void stepperSet();
extern void servo_();
extern void servoSet();
extern void serialBegin();
extern void serialWrite();
extern void logging();
extern void inputDigit();
extern void digitSet();
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