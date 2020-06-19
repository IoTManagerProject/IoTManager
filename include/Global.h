#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Bounce2.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <PubSubClient.h>
#include <SPIFFSEditor.h>
#include <StringCommand.h>
#include <TickerScheduler.h>
#include <time.h>

#include "Consts.h"

/* 
* ESP8266
*/
#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
ESP8266HTTPUpdateServer httpUpdater;
#include <WiFiUdp.h>
WiFiUDP Udp;
#include <Servo.h>
#ifdef MDNS_enable
#include <ESP8266mDNS.h>
#endif
#endif

/* 
* ESP32
*/
#ifdef ESP32
#include <AsyncTCP.h>
#include <AsyncUDP.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <analogWrite.h>
extern AsyncUDP udp;
#include <ESP32Servo.h>
#ifdef MDNS_enable
#include <ESPmDNS.h>
#endif
#endif

extern Servo myServo1;
extern Servo myServo2;

#ifdef OTA_enable
#include <ArduinoOTA.h>
#endif

extern AsyncWebServer server;

// Global vars
extern boolean just_load;
extern const char *hostName;

extern String configSetupJson;  //все настройки
extern String configLiveJson;   //все данные с датчиков (связан с mqtt)
extern String configOptionJson; //для трансфера

extern String chipID;
extern String prex;
extern String all_widgets;
extern String scenario;
extern String order_loop;

extern String analog_value_names_list;
extern int enter_to_analog_counter;

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

extern String current_time;

extern int scenario_line_status[40];

extern int wifi_lost_error;
extern int mqtt_lost_error;

extern String last_version;

extern boolean upgrade_url;
extern boolean upgrade;
extern boolean mqtt_connection;
extern boolean udp_data_parse;
extern boolean mqtt_send_settings_to_udp;
extern boolean i2c_scanning;

#ifdef WS_enable
extern AsyncWebSocket ws;
#endif

extern AsyncEventSource events;

extern int sensors_reading_map[15];

enum {
    ROUTER_SEARCHING,
    WIFI_MQTT_CONNECTION_CHECK,
    SENSORS,
    STEPPER1,
    STEPPER2,
    LOG1,
    LOG2,
    LOG3,
    LOG4,
    LOG5,
    TIMER_COUNTDOWN,
    TIME,
    TIME_SYNC,
    STATISTICS,
    UDP,
    UDP_DB,
    TEST
};

extern TickerScheduler ts;

extern WiFiClient espClient;
extern PubSubClient client_mqtt;
extern StringCommand sCmd;

#define NUM_BUTTONS 6
extern boolean but[NUM_BUTTONS];
extern Bounce *buttons;

extern boolean udp_busy;
extern unsigned int udp_port;
extern IPAddress udp_multicastIP;
extern String received_ip;
extern String received_udp_line;
extern int udp_period;

#ifdef level_enable
#include "GyverFilters.h"
extern GMedian<10, int> medianFilter;
#endif

#include <DallasTemperature.h>
#include <OneWire.h>
extern OneWire *oneWire;
extern DallasTemperature sensors;

#include <DHTesp.h>
extern DHTesp dht;

#include <Wire.h>

#ifdef bmp_enable
#include <Adafruit_BMP280.h>
extern Adafruit_BMP280 bmp;
extern Adafruit_Sensor *bmp_temp;
extern Adafruit_Sensor *bmp_pressure;
#endif

#ifdef bme_enable
#include <Adafruit_BME280.h>
extern Adafruit_BME280 bme;
extern Adafruit_Sensor *bme_temp;
extern Adafruit_Sensor *bme_pressure;
extern Adafruit_Sensor *bme_humidity;
#endif

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(14, 12);

// StringUtils
extern uint8_t hexStringToUint8(String hex);
extern uint16_t hexStringToUint16(String hex);
extern String selectToMarkerLast(String str, String found);
extern String selectToMarker(String str, String found);
extern String deleteAfterDelimiter(String str, String found);
extern String deleteBeforeDelimiter(String str, String found);
extern String deleteBeforeDelimiterTo(String str, String found);
extern String selectFromMarkerToMarker(String str, String found, int number);

// JsonUtils
extern String jsonReadStr(String &json, String name);
extern int jsonReadInt(String &json, String name);
extern String jsonWriteInt(String &json, String name, int volume);
extern String jsonWriteStr(String &json, String name, String volume);
extern String jsonWriteFloat(String &json, String name, float volume);

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
extern void firmwareVersion();
extern void firmwareUpdate();
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
extern void saveConfig();
extern String getURL(const String &urls);

extern String writeFile(String fileName, String strings);
extern String readFile(String fileName, size_t len);
extern String addFile(String fileName, String strings);

extern void servo_();
extern boolean isDigitStr(String str);
extern String jsonWriteStr(String &json, String name, String volume);
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
// И как раз тут хорошо просто в Sensors.h это пихать - а не в один здоровенный ФАЙЛ
extern void sensors_init();

extern void levelPr();
extern void ultrasonicCm();
extern void level_reading();


extern void analog();
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