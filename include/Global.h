#pragma once

#include <Arduino.h>

#include "Consts.h"

// Cmd
extern void CMD_init();
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
extern void firmware();
extern void update_firmware();
extern void Scenario_init();
extern void txtExecution(String file);
extern void stringExecution(String str);

// FileSystem
extern void File_system_init();

// i2c_bu
extern void do_i2c_scanning();
extern String i2c_scan();

// Init
extern void All_init();
extern void statistics_init();
extern void Scenario_init();
extern void Device_init();
extern void prsets_init();
extern void up_time();

// Logging
extern void logging();
extern void deleteOldDate(String file, int seted_number_of_lines, String date_to_add);
extern void clean_log_date();
extern void choose_log_date_and_send();

// Main
void getMemoryLoad(String text);

extern String jsonReadStr(String& json, String name);
extern int jsonReadInt(String& json, String name);
extern String jsonWriteInt(String& json, String name, int volume);
extern String jsonWriteStr(String& json, String name, String volume);
extern void saveConfig();
extern String jsonWriteFloat(String& json, String name, float volume);

extern String getURL(String urls);

extern String writeFile(String fileName, String strings);
extern String readFile(String fileName, size_t len);
extern String addFile(String fileName, String strings);

//STRING
extern String selectToMarkerLast(String str, String found);
extern String selectToMarker(String str, String found);
extern String deleteAfterDelimiter(String str, String found);
extern String deleteBeforeDelimiter(String str, String found);
extern String deleteBeforeDelimiterTo(String str, String found);

extern String selectFromMarkerToMarker(String str, String found, int number);

extern void servo_();
extern boolean isDigitStr(String str);
extern String jsonWriteStr(String& json, String name, String volume);
extern void led_blink(String satus);
extern int count(String str, String found);


// Mqtt
extern void MQTT_init();
extern boolean MQTT_Connecting();
extern boolean sendMQTT(String end_of_topik, String data);
extern boolean sendCHART(String topik, String data);
extern void sendSTATUS(String topik, String state);
extern void sendCONTROL(String id, String topik, String state);
extern void do_mqtt_connection();
extern void handleMQTT();
extern String selectFromMarkerToMarker(String str, String found, int number);

// WiFiUtils
extern void WIFI_init();
extern void All_init();
extern bool StartAPMode();
extern void ROUTER_Connecting();

//Scenario
extern void eventGen(String event_name, String number);
extern String add_set(String param_name);

//Sensors
extern void sensors_init();

//Timers
extern void Timer_countdown_init();
extern void timerStart_();
extern void addTimer(String number, String time);
extern void timerStop_();
extern void delTimer(String number);
extern int readTimer(int number);

//TimeUtils
extern void Time_Init();
extern int timeToMin(String Time);
extern String GetDataDigital();
extern String GetDate();
extern String GetTimeWOsec();
extern String GetTime();
extern String GetTimeUnix();
extern void reconfigTime();
extern void saveConfig();
extern String GetTimeUnix();
extern void time_check();

//Upgrade
extern void initUpgrade();

//widget
extern void createWidget(String widget_name, String page_name, String page_number, String file, String topic);
extern void createWidgetParam(String widget_name, String page_name, String page_number, String file, String topic, String name1, String param1, String name2, String param2, String name3, String param3);
extern void choose_widget_and_create(String widget_name, String page_name, String page_number, String type, String topik);
extern void createChart(String widget_name, String page_name, String page_number, String file, String topic, String maxCount);

// Push

extern void Push_init();

// UDP
extern void UDP_init();
extern void do_udp_data_parse();
extern void do_mqtt_send_settings_to_udp();
// WebServer
extern void Web_server_init();

//iot_firmware
extern void not_async_actions();
extern void handleCMD_loop();
extern void handleButton();
extern void handleScenario();
extern void handleUdp();
extern void do_upgrade_url();
extern void do_upgrade();