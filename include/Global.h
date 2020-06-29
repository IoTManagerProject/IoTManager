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
#include "GyverFilters.h"
#include "Upgrade.h"
#include "Clock.h"

#include "MqttClient.h"
#include "Utils\FileUtils.h"
#include "Utils\JsonUtils.h"
#include "Utils\StringUtils.h"
#include "Utils\SysUtils.h"
#include "Utils\PrintMessage.h"
#include "Utils\WiFiUtils.h"
#include "TickerScheduler/TickerScheduler.h"

//=========ПОДКЛЮЧЕНИЕ ОБЩИХ БИБЛИОТЕК===============
#include <ArduinoOTA.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include <Bounce2.h>
#include <DHTesp.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <PubSubClient.h>
#include <StringCommand.h>
#include <Wire.h>
#include <time.h>

#ifdef WEBSOCKET_ENABLED
extern AsyncWebSocket ws;
//extern AsyncEventSource events;
#endif

extern Clock* timeNow;

extern TickerScheduler ts;

extern WiFiClient espClient;

extern PubSubClient mqtt;

extern StringCommand sCmd;

extern AsyncWebServer server;

extern DallasTemperature* sensors;

/*
* Global vars
*/

extern boolean just_load;

extern String configSetupJson;   //все настройки
extern String configLiveJson;    //все данные с датчиков (связан с mqtt)
extern String configOptionJson;  //для трансфера

extern String chipId;
extern String prex;
extern String all_widgets;
extern String scenario;
extern String order_loop;

extern String analog_value_names_list;
extern int enter_to_analog_counter;

extern String dallas_value_name;
extern int enter_to_dallas_counter;

extern String levelPr_value_name;
extern String ultrasonicCm_value_name;

extern String dhtT_value_name;
extern String dhtH_value_name;

extern String bmp280T_value_name;
extern String bmp280P_value_name;

extern String bme280T_value_name;
extern String bme280P_value_name;
extern String bme280H_value_name;
extern String bme280A_value_name;

extern String logging_value_names_list;
extern int enter_to_logging_counter;

extern int scenario_line_status[40];

extern String lastVersion;

extern boolean checkUpdatesFlag;
extern boolean updateFlag;
extern boolean mqttParamsChanged;
extern boolean udp_data_parse;
extern boolean mqtt_send_settings_to_udp;

/*
* Запрос на скарнирование шины
*/
extern boolean busScanFlag;
/*
* Запрос на сканирование шины, указание какую
*/
extern BusScanner_t busToScan;

extern boolean fsCheckFlag;

extern int sensors_reading_map[15];

/*
* Global functions
*/

// Cmd
extern void cmd_init();
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
// Init
extern void loadConfig();
extern void all_init();
extern void statistics_init();
extern void loadScenario();
extern void Device_init();
extern void prsets_init();

// Logging
extern void logging();
extern void deleteOldDate(String filename, size_t max_lines, String date_to_add);
extern void clean_log_date();
extern void choose_log_date_and_send();

// Main
extern void setChipId();
extern void saveConfig();
extern void setConfigParam(const char* param, const String& value);

extern String getURL(const String& urls);
extern void do_fscheck();
extern void do_scan_bus();
extern void servo_();
extern void clock_init();

extern void setLedStatus(LedStatus_t);

//Scenario
extern void eventGen(String event_name, String number);
extern String add_set(String param_name);

//Sensors
extern void sensors_init();

extern void levelPr();
extern void ultrasonicCm();
extern void ultrasonic_reading();

extern void analog();
extern void analog_reading1();
extern void analog_reading2();
extern void dallas();
extern void dallas_reading();
extern void dhtT_reading();

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

//Timers
extern void Timer_countdown_init();
extern void timerStart_();
extern void addTimer(String number, String time);
extern void timerStop_();
extern void delTimer(String number);
extern int readTimer(int number);

extern void initUpdater();

// Widget
extern void createWidget(String widget_name, String page_name, String page_number, String file, String topic, String name1 = "", String param1 = "", String name2 = "", String param2 = "", String name3 = "", String param3 = "");
extern void createChart(String widget_name, String page_name, String page_number, String file, String topic, String maxCount);

// PushingBox
extern void pushControl();

// UDP
extern void udp_init();
extern void do_udp_data_parse();
extern void do_mqtt_send_settings_to_udp();

extern void addCommandLoop(const String& cmdStr);
extern void loop_serial();
extern void loopCmd();
extern void loop_button();
extern void loopScenario();
extern void loopUdp();

extern void do_update();

// Init
extern void uptime_init();

// Web
extern void web_init();
extern void telemetry_init();
