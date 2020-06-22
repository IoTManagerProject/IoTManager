#include "Global.h"

/*
* Objects.cpp(с данными)
*/

#ifdef WS_enable
AsyncWebSocket ws;
//AsyncEventSource events;
#endif


TickerScheduler ts(TEST + 1);

WiFiClient espClient;

PubSubClient client_mqtt(espClient);

StringCommand sCmd;

AsyncWebServer server(80);

DallasTemperature sensors;

/*
* Global vars
*/

boolean just_load = true;

// Json
String configSetupJson = "{}";
String configLiveJson = "{}";
String configOptionJson = "{}";

// Mqtt
String chipId = "";
String prex = "";
String all_widgets = "";
String scenario = "";
String order_loop = "";

// Sensors
String analog_value_names_list;
int enter_to_analog_counter;

String levelPr_value_name;
String ultrasonicCm_value_name;

String dhtT_value_name;
String dhtH_value_name;

String bmp280T_value_name;
String bmp280P_value_name;

String bme280T_value_name;
String bme280P_value_name;
String bme280H_value_name;
String bme280A_value_name;

int sensors_reading_map[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Logging
String logging_value_names_list;
int enter_to_logging_counter;

// Scenario
int scenario_line_status[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

String last_version = "";

// Async actions
boolean upgrade_url = false;
boolean upgrade = false;
boolean mqtt_connection = false;
boolean udp_data_parse = false;
boolean mqtt_send_settings_to_udp = false;
boolean i2c_scanning = false;
