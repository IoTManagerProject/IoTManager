#pragma once
/*
* Main consts
*/
#define FIRMWARE_VERSION "2.3.4"
#define NUM_BUTTONS 6
#define LED_PIN 2
#define FLASH_4MB true
#define MQTT_RECONNECT_INTERVAL 20000
// 1000 * 60 * 60 * 2
#define TELEMETRY_UPDATE_INTERVAL 0

/* 
* Optional
*/
//#define OTA_UPDATES_ENABLED
//#define MDNS_ENABLED
//#define WEBSOCKET_ENABLED
//#define LAYOUT_IN_RAM
#define UDP_ENABLED

/* 
* Sensor 
*/
#define TANK_LEVEL_SAMPLES 10
#define LEVEL_ENABLED
#define ANALOG_ENABLED
#define DALLAS_ENABLED
#define DHT_ENABLED
#define BMP_ENABLED
#define BME_ENABLED

/* 
* Gears 
*/
#define STEPPER_ENABLED
#define SERVO_ENABLED

/* 
* Other
*/
#define LOGGING_ENABLED
#define SERIAL_ENABLED
#define PUSH_ENABLED
