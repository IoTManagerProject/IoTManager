#pragma once

/*
* Libraries
*/
#include <Arduino.h>
#include <ArduinoJson.h>

#include "ESP32.h"
#include "ESP8266.h"
//
#include "Consts.h"
#include "Errors.h"
#include "Config.h"
#include "Upgrade.h"
#include "Clock.h"
#include "Scenario.h"
#include "Sensors.h"
#include "WiFiManager.h"

#include "Objects/EventQueue.h"

#include "Utils/FileUtils.h"
#include "Utils/JsonUtils.h"
#include "Utils/StringUtils.h"
#include "Utils/SysUtils.h"
#include "Utils/PrintMessage.h"
#include "TickerScheduler/TickerScheduler.h"

#include <ArduinoOTA.h>
#include <StringCommand.h>

#include <time.h>

#ifdef WEBSOCKET_ENABLED
extern AsyncWebSocket ws;
//extern AsyncEventSource events;
#endif

extern Clock* timeNow;
extern TickerScheduler ts;
extern WiFiClient espClient;
extern StringCommand sCmd;
extern AsyncWebServer server;

/*
* Global vars
*/
extern boolean just_load;
// все настройки
extern String configSetupJson;
// все данные с датчиков (связан с mqtt)
extern String configLiveJson;
// для трансфера
extern String configOptionJson;

extern String chipId;
extern String prex;
extern String all_widgets;
extern String order_loop;

extern String logging_value_names_list;
extern int enter_to_logging_counter;

extern String lastVersion;

extern boolean perform_updates_check;
extern boolean perform_upgrade;
extern boolean mqttParamsChangedFlag;
extern boolean udp_data_parse;

extern void fireEvent(String name);
extern void fireEvent(String name, String param);

extern const String readLiveData(const String& obj);
extern const String writeLiveData(const String& obj, const String& value);

/*
* Запрос на сканирование шины
*/
extern void perform_bus_scanning(BusScanner_t bus);
/*
* Произвести перезагрузку
*/
extern void perform_system_restart();
/*
* Широковещательная расслыка настроек mqtt
*/
extern void broadcast_mqtt_settings();

/*
* Global functions
*/
extern void fileExecute(const String filename);

extern void stringExecute(String& cmdStr);
// Init
extern void loadConfig();
extern void statistics_init();
extern void device_init();
extern void cmd_init();
extern void init_mod();
// Logging
extern void logging();
extern void deleteOldDate(String filename, size_t max_lines, String date_to_add);
extern void clean_log_date();
extern void choose_log_date_and_send();

// Main
extern void setChipId();
extern void saveConfig();
extern void setPreset(size_t num);
extern void setConfigParam(const char* param, const String& value);
extern void setLedStatus(LedStatus_t);

//Scenario
extern void fireEvent(String event_name, String number);
extern String add_set(String param_name);

//Timers
extern void Timer_countdown_init();
extern void timerStart_();
extern void addTimer(String number, String time);
extern void timerStop_();
extern void delTimer(String number);
extern int readTimer(int number);

extern void update_init();

// Widget
extern void createWidget(String widget_name, String page_name, String page_number, String file, String topic, String name1 = "", String param1 = "", String name2 = "", String param2 = "", String name3 = "", String param3 = "");
extern void createChart(String widget_name, String page_name, String page_number, String file, String topic, String maxCount);
extern void clearWidgets();

// PushingBox
extern void pushControl();

extern void addCommandLoop(const String& cmdStr);
extern void loop_serial();
extern void loopCmd();
extern void loop_button();
extern void loop_scenario();

extern void do_update();

// Init
extern void uptime_init();

// Web
extern void web_init();
extern void telemetry_init();
