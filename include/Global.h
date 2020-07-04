#pragma once

/*
* Libraries
*/
#include <Arduino.h>
#include <ArduinoJson.h>

#include "ESP32.h"
#include "ESP8266.h"
//

#include "Base/KeyValueStore.h"
#include "Base/StringQueue.h"

#include "Consts.h"
#include "Errors.h"
#include "Config.h"
#include "Upgrade.h"
#include "Clock.h"
#include "Sensors.h"
#include "Scenario.h"
#include "MqttClient.h"
#include "WiFiManager.h"

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

extern KeyValueStore options;
extern KeyValueStore liveData;
extern KeyValueFile runtime;

extern Clock* timeNow;
extern TickerScheduler ts;
extern WiFiClient espClient;
extern StringCommand sCmd;
extern AsyncWebServer server;

/*
* Global vars
*/
extern boolean just_load;
extern String prex;
extern String all_widgets;
extern String order_loop;

extern String lastVersion;

extern boolean perform_updates_check_flag;
extern boolean perform_upgrade_flag;
extern boolean mqtt_restart_flag;
extern boolean udp_data_parse;

/*
* Запрос на проверку обновлений
*/
extern void perform_updates_check();
/*
* Запрос на аггрейд
*/
extern void perform_upgrade();
/*
* Запрос на очистук логов
*/
extern void perform_logger_clear();
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
* Запуск комманд из файла
*/
extern void fileExecute(const String filename);
/*
* Запуск комманд из строки
*/
extern void stringExecute(String& cmdStr);

extern void load_runtime();
extern void load_config();

extern void statistics_init();
extern void device_init();
extern void cmd_init();
extern void init_mod();

extern void save_runtime();
extern void save_config();
extern void setPreset(size_t num);
extern void setLedStatus(LedStatus_t);

//Scenario
extern String add_set(String param_name);

//Timers
extern void timer_countdown_init();
extern void cmd_timerStart();
extern void addTimer(String number, String time);
extern void cmd_timerStop();
extern void delTimer(String number);
extern int readTimer(int number);

// Widget
extern void createWidget(String widget_name, String page_name, String page_number, String file, String topic, String name1 = "", String param1 = "", String name2 = "", String param2 = "", String name3 = "", String param3 = "");
extern void createChart(String widget_name, String page_name, String page_number, String file, String topic, String maxCount);
extern void clearWidgets();

// PushingBox
extern void pushControl();

extern void addCommandLoop(const String& cmdStr);

extern void loop_serial();
extern void loop_cmd();
extern void loop_button();
extern void loop_scenario();

// Init
extern void uptime_task_init();

extern void config_init();
extern void web_init();
extern void telemetry_init();

extern int readOptionInt(const String& name);