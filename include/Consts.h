#pragma once
//Здесь хранятся все настройки прошивки

#define firmware_version "2.3.3"

/*
* Firmware settings
*/
#define mb_4_of_memory 1
#define wifi_mqtt_reconnecting 20000
#define blink_pin 2
#define tank_level_times_to_send 10  //после скольки выстрелов делать отправку данных
#define statistics_update 1000 * 60 * 60 * 2

//#define OTA_enable
//#define MDNS_enable
//#define WS_enable
//#define layout_in_ram
#define UDP_enable

/* 
* Sensor 
*/
#define level_enable
#define analog_enable
#define dallas_enable
#define dht_enable
#define bmp_enable
#define bme_enable

/*
* Logging
*/
#define logging_enable

/* 
* Gears 
*/
#define stepper_enable
#define servo_enable

/* 
* Other
*/
#define serial_enable
#define push_enable
