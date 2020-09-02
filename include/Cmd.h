#pragma once
extern void cmd_init();
#include <Arduino.h>

extern void sensorsInit();

extern void levelPr();
extern void ultrasonicCm();
extern void ultrasonic_reading();

extern void analog_reading1();
extern void analog_reading2();
extern void dallas_reading();
extern void dhtT_reading();

extern void dallas();

extern void bmp280T();
extern void bmp280P();
extern void bmp280T_reading();
extern void bmp280P_reading();

extern void bme280T();
extern void bme280P();
extern void bme280H();
extern void bme280A();

extern void bme280T_reading();
extern void bme280P_reading();
extern void bme280H_reading();
extern void bme280A_reading();

extern void dhtT();
extern void dhtH();
extern void dhtP();
extern void dhtC();
extern void dhtD();
extern void dhtH_reading();
extern void dhtP_reading();
extern void dhtC_reading();
extern void dhtD_reading();

extern void handle_time_init();
extern void stepper();
extern void stepperSet();
extern void servo_();
extern void servoSet();
extern void serialBegin();
extern void serialWrite();
extern void logging();

extern void button();
extern void timeSet();

extern void mqttOrderSend();
extern void httpOrderSend();
extern void firmwareVersion();
extern void firmwareUpdate();
extern void loadScenario();

extern void fileExecute(const String& filename);
extern void csvExecute(String& cmdStr);
extern void spaceExecute(String& cmdStr);