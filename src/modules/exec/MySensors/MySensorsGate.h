#pragma once
#include "Const.h"
#define MYSENSORS

#ifdef MYSENSORS

/*
    * DESCRIPTION
 * The ESP32 gateway sends data received from sensors to the WiFi link.
 * The gateway also accepts input on ethernet interface, which is then sent out to the radio network.
 * ----------- PINOUT --------------
 * | IO   | RF24 | RFM69 | RFM95 |
  |------|------|-------|-------|
  | MOSI | 23   | 23    | 23    |
  | MISO | 19   | 19    | 19    |
  | SCK  | 18   | 18    | 18    |
  | CSN  | 5    | 5     | 5     |
  | CE   | 17   | -     | -     |
  | RST  | -    | 17    | 17    |
  | IRQ  | 16*  | 16    | 16    |
*/

// Enable debug prints to serial monitor
//#define MY_DEBUG

//#define MY_RF24_CE_PIN 26  // Двигать пин CE на D4 чтобы освободить I2C
//#define MY_RF24_CS_PIN  9

// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
#define MY_BAUD_RATE 115200

// Enables and select radio type (if attached)
#define MY_RADIO_RF24
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

//#define MY_RF24_DATARATE RF24_1MBPS // Для платы KeyWish скорость 1 мегабит

// How many clients should be able to connect to this gateway (default 1)
#define MY_GATEWAY_MAX_CLIENTS 10

// Set LOW transmit power level as default, if you have an amplified NRF-module and
// power your radio separately with a good regulator you can turn up PA level.
#define MY_RF24_PA_LEVEL RF24_PA_MAX

// используем гейт в режиме serial хотя нам этот режим не нужен, поэтому в библиотеки отключаем MY_SERIALDEVICE.print
// в файле MyGatewayTransportSerial.cpp в строчке 35
#define MY_GATEWAY_SERIAL

//#define CHILD_ID 1

#include <MySensors.h>

extern String parseToString(const MyMessage& message);

#endif