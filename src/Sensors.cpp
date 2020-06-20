#include "Global.h"
#include "StringUtils.h"

String perception(byte value);
void bmp280T_reading();
String get_comfort_status(ComfortState cf);

void sensors_init() {
    ts.add(
        SENSORS, 1000, [&](void *) {
            static int counter;
            counter++;

#ifdef level_enable
            if (sensors_reading_map[0] == 1)
                ultrasonic_reading();
#endif

            if (counter > 10) {
                counter = 0;

#ifdef analog_enable
                if (sensors_reading_map[1] == 1)
                    analog_reading1();
                if (sensors_reading_map[2] == 1)
                    analog_reading2();
#endif

#ifdef dallas_enable
                if (sensors_reading_map[3] == 1)
                    dallas_reading();
#endif

#ifdef dht_enable
                if (sensors_reading_map[4] == 1)
                    dhtT_reading();
                if (sensors_reading_map[5] == 1)
                    dhtH_reading();
                if (sensors_reading_map[6] == 1)
                    dhtP_reading();
                if (sensors_reading_map[7] == 1)
                    dhtC_reading();
                if (sensors_reading_map[8] == 1)
                    dhtD_reading();
#endif

#ifdef bmp_enable
                if (sensors_reading_map[9] == 1)
                    bmp280T_reading();
                if (sensors_reading_map[10] == 1)
                    bmp280P_reading();
#endif

#ifdef bme_enable
                if (sensors_reading_map[11] == 1)
                    bme280T_reading();
                if (sensors_reading_map[12] == 1)
                    bme280P_reading();
                if (sensors_reading_map[13] == 1)
                    bme280H_reading();
                if (sensors_reading_map[14] == 1)
                    bme280A_reading();
#endif
            }
        },
        nullptr, true);
}

//=========================================================================================================================================
//=========================================Модуль измерения уровня в баке==================================================================
#ifdef level_enable
//levelPr p 14 12 Вода#в#баке,#% Датчики fill-gauge 125 20 1
void levelPr() {
  String value_name = sCmd.next();
  String trig = sCmd.next();
  String echo = sCmd.next();
  String widget_name = sCmd.next();
  String page_name = sCmd.next();
  String type = sCmd.next();
  String empty_level = sCmd.next();
  String full_level = sCmd.next();
  String page_number = sCmd.next();
  levelPr_value_name = value_name;
  jsonWriteStr(configOptionJson, "e_lev", empty_level);
  jsonWriteStr(configOptionJson, "f_lev", full_level);
  jsonWriteStr(configOptionJson, "trig", trig);
  jsonWriteStr(configOptionJson, "echo", echo);
  pinMode(trig.toInt(), OUTPUT);
  pinMode(echo.toInt(), INPUT);
  choose_widget_and_create(widget_name, page_name, page_number, type, value_name);
  sensors_reading_map[0] = 1;
}
//ultrasonicCm cm 14 12 Дистанция,#см Датчики fill-gauge 1
void ultrasonicCm() {
  String value_name = sCmd.next();
  String trig = sCmd.next();
  String echo = sCmd.next();
  String widget_name = sCmd.next();
  String page_name = sCmd.next();
  String type = sCmd.next();
  String empty_level = sCmd.next();
  String full_level = sCmd.next();
  String page_number = sCmd.next();
  ultrasonicCm_value_name = value_name;
  jsonWriteStr(configOptionJson, "trig", trig);
  jsonWriteStr(configOptionJson, "echo", echo);
  pinMode(trig.toInt(), OUTPUT);
  pinMode(echo.toInt(), INPUT);
  choose_widget_and_create(widget_name, page_name, page_number, type, value_name);
  sensors_reading_map[0] = 1;
}

void ultrasonic_reading() {
  long duration_;
  int distance_cm;
  int level;
  static int counter;
  int trig = jsonReadInt(configOptionJson, "trig");
  int echo = jsonReadInt(configOptionJson, "echo");
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  duration_ = pulseIn(echo, HIGH, 30000); // 3000 µs = 50cm // 30000 µs = 5 m
  distance_cm = duration_ / 29 / 2;
  distance_cm = medianFilter.filtered(distance_cm);//отсечение промахов медианным фильтром
  counter++;
  if (counter > tank_level_times_to_send) {
    counter = 0;
    level = map(distance_cm,
                jsonReadInt(configOptionJson, "e_lev"),
                jsonReadInt(configOptionJson, "f_lev"), 0, 100);
                
    jsonWriteInt(configLiveJson, levelPr_value_name, level);
    eventGen (levelPr_value_name, "");
    sendSTATUS(levelPr_value_name, String(level));
    Serial.println("[i] sensor '" + levelPr_value_name + "' data: " + String(level));
    
    jsonWriteInt(configLiveJson, ultrasonicCm_value_name, distance_cm);
    eventGen (ultrasonicCm_value_name, "");
    sendSTATUS(ultrasonicCm_value_name, String(distance_cm));
    Serial.println("[i] sensor '" + ultrasonicCm_value_name + "' data: " + String(distance_cm));
  }
}
#endif
//=========================================================================================================================================
//=========================================Модуль аналогового сенсора======================================================================
#ifdef analog_enable
//analog adc 0 Аналоговый#вход,#% Датчики any-data 1 1023 1 100 1
void analog() {
    String value_name = sCmd.next();
    String pin = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String analog_start = sCmd.next();
    String analog_end = sCmd.next();
    String analog_start_out = sCmd.next();
    String analog_end_out = sCmd.next();
    String page_number = sCmd.next();
    analog_value_names_list += value_name + ",";
    enter_to_analog_counter++;
    jsonWriteStr(configOptionJson, value_name + "_st", analog_start);
    jsonWriteStr(configOptionJson, value_name + "_end", analog_end);
    jsonWriteStr(configOptionJson, value_name + "_st_out", analog_start_out);
    jsonWriteStr(configOptionJson, value_name + "_end_out", analog_end_out);
    choose_widget_and_create(widget_name, page_name, page_number, type, value_name);
    if (enter_to_analog_counter == 1) {
        sensors_reading_map[1] = 1;
    }
    if (enter_to_analog_counter == 2) {
        sensors_reading_map[2] = 1;
    }
}

void analog_reading1() {
    String value_name = selectFromMarkerToMarker(analog_value_names_list, ",", 0);
#ifdef ESP32
    int analog_in = analogRead(34);
#endif
#ifdef ESP8266
    int analog_in = analogRead(A0);
#endif
    int analog = map(analog_in,
                     jsonReadInt(configOptionJson, value_name + "_st"),
                     jsonReadInt(configOptionJson, value_name + "_end"),
                     jsonReadInt(configOptionJson, value_name + "_st_out"),
                     jsonReadInt(configOptionJson, value_name + "_end_out"));
    jsonWriteInt(configLiveJson, value_name, analog);
    eventGen(value_name, "");
    sendSTATUS(value_name, String(analog));
    Serial.println("[i] sensor '" + value_name + "' data: " + String(analog));
}

void analog_reading2() {
    String value_name = selectFromMarkerToMarker(analog_value_names_list, ",", 1);
#ifdef ESP32
    int analog_in = analogRead(35);
#endif
#ifdef ESP8266
    int analog_in = analogRead(A0);
#endif
    int analog = map(analog_in,
                     jsonReadInt(configOptionJson, value_name + "_st"),
                     jsonReadInt(configOptionJson, value_name + "_end"),
                     jsonReadInt(configOptionJson, value_name + "_st_out"),
                     jsonReadInt(configOptionJson, value_name + "_end_out"));
    jsonWriteInt(configLiveJson, value_name, analog);
    eventGen(value_name, "");
    sendSTATUS(value_name, String(analog));
    Serial.println("[i] sensor '" + value_name + "' data: " + String(analog));
}
#endif
//=========================================================================================================================================
//=========================================Модуль температурного сенсора ds18b20===========================================================
#ifdef dallas_enable
void dallas() {
    //String value_name = sCmd.next();
    String pin = sCmd.next();
    //String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    oneWire = new OneWire((uint8_t)pin.toInt());
    sensors.setOneWire(oneWire);
    sensors.begin();
    sensors.setResolution(12);
    choose_widget_and_create(widget_name, page_name, page_number, type, "dallas");
    sensors_reading_map[3] = 1;
}

void dallas_reading() {
    float temp = 0;
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
    jsonWriteStr(configLiveJson, "dallas", String(temp));
    eventGen("dallas", "");
    sendSTATUS("dallas", String(temp));
    Serial.println("[i] sensor 'dallas' send date " + String(temp));
}
#endif
//=========================================================================================================================================
//=========================================Модуль сенсоров DHT=============================================================================
#ifdef dht_enable
//dhtT t 2 dht11 Температура#DHT,#t°C Датчики any-data 1
void dhtT() {
    String value_name = sCmd.next();
    String pin = sCmd.next();
    String sensor_type = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    dhtT_value_name = value_name;
    if (sensor_type == "dht11") {
        dht.setup(pin.toInt(), DHTesp::DHT11);
    }
    if (sensor_type == "dht22") {
        dht.setup(pin.toInt(), DHTesp::DHT22);
    }
    choose_widget_and_create(widget_name, page_name, page_number, type, value_name);
    sensors_reading_map[4] = 1;
}

void dhtT_reading() {
    float value = 0;
    static int counter;
    if (dht.getStatus() != 0 && counter < 5) {
        sendSTATUS(dhtT_value_name, String(dht.getStatusString()));
        counter++;
    } else {
        counter = 0;
        value = dht.getTemperature();
        if (String(value) != "nan") {
            eventGen(dhtT_value_name, "");
            jsonWriteStr(configLiveJson, dhtT_value_name, String(value));
            sendSTATUS(dhtT_value_name, String(value));
            Serial.println("[i] sensor '" + dhtT_value_name + "' data: " + String(value));
        }
    }
}

//dhtH h 2 dht11 Влажность#DHT,#t°C Датчики any-data 1
void dhtH() {
    String value_name = sCmd.next();
    String pin = sCmd.next();
    String sensor_type = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    dhtH_value_name = value_name;
    if (sensor_type == "dht11") {
        dht.setup(pin.toInt(), DHTesp::DHT11);
    }
    if (sensor_type == "dht22") {
        dht.setup(pin.toInt(), DHTesp::DHT22);
    }
    choose_widget_and_create(widget_name, page_name, page_number, type, value_name);
    sensors_reading_map[5] = 1;
}

void dhtH_reading() {
    float value = 0;
    static int counter;
    if (dht.getStatus() != 0 && counter < 5) {
        sendSTATUS(dhtH_value_name, String(dht.getStatusString()));
        counter++;
    } else {
        counter = 0;
        value = dht.getHumidity();
        if (String(value) != "nan") {
            eventGen(dhtH_value_name, "");
            jsonWriteStr(configLiveJson, dhtH_value_name, String(value));
            sendSTATUS(dhtH_value_name, String(value));
            Serial.println("[i] sensor '" + dhtH_value_name + "' data: " + String(value));
        }
    }
}

//dhtPerception Восприятие: Датчики 4
void dhtP() {
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String page_number = sCmd.next();
    choose_widget_and_create(widget_name, page_name, page_number, "any-data", "dhtPerception");
    sensors_reading_map[6] = 1;
}

void dhtP_reading() {
    byte value;
    if (dht.getStatus() != 0) {
        sendSTATUS("dhtPerception", String(dht.getStatusString()));
    } else {
        value = dht.computePerception(jsonReadStr(configLiveJson, dhtT_value_name).toFloat(), jsonReadStr(configLiveJson, dhtH_value_name).toFloat(), false);
        String final_line = perception(value);
        jsonWriteStr(configLiveJson, "dhtPerception", final_line);
        eventGen("dhtPerception", "");
        sendSTATUS("dhtPerception", final_line);
        if (client_mqtt.connected()) {
            Serial.println("[i] sensor 'dhtPerception' data: " + final_line);
        }
    }
}

String perception(byte value) {
    String res;
    switch (value) {
        case 0:
            res = F("Сухой воздух");
            break;
        case 1:
            res = F("Комфортно");
            break;
        case 2:
            res = F("Уютно");
            break;
        case 3:
            res = F("Хорошо");
            break;
        case 4:
            res = F("Неудобно");
            break;
        case 5:
            res = F("Довольно неудобно");
            break;
        case 6:
            res = F("Очень неудобно");
            break;
        case 7:
            res = F("Сильно неудобно, полный звиздец");
        default:
            res = F("Unknown");
            break;
    }
    return res;
}

//dhtComfort Степень#комфорта: Датчики 3
void dhtC() {
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String page_number = sCmd.next();
    choose_widget_and_create(widget_name, page_name, page_number, "any-data", "dhtComfort");
    sensors_reading_map[7] = 1;
}

void dhtC_reading() {  
    ComfortState cf;
    if (dht.getStatus() != 0) {
        sendSTATUS("dhtComfort", String(dht.getStatusString()));
    } else {
        dht.getComfortRatio(cf, jsonReadStr(configLiveJson, dhtT_value_name).toFloat(), jsonReadStr(configLiveJson, dhtH_value_name).toFloat(), false);
        String final_line = get_comfort_status(cf);
        jsonWriteStr(configLiveJson, "dhtComfort", final_line);
        eventGen("dhtComfort", "");
        sendSTATUS("dhtComfort", final_line);
        Serial.println("[i] sensor 'dhtComfort' send date " + final_line);
    }
}

String get_comfort_status(ComfortState cf) {
    String comfortStatus;
    switch (cf) {
        case Comfort_OK:
            comfortStatus = "Отлично";
            break;
        case Comfort_TooHot:
            comfortStatus = "Очень жарко";
            break;
        case Comfort_TooCold:
            comfortStatus = "Очень холодно";
            break;
        case Comfort_TooDry:
            comfortStatus = "Очень сухо";
            break;
        case Comfort_TooHumid:
            comfortStatus = "Очень влажно";
            break;
        case Comfort_HotAndHumid:
            comfortStatus = "Жарко и влажно";
            break;
        case Comfort_HotAndDry:
            comfortStatus = "Жарко и сухо";
            break;
        case Comfort_ColdAndHumid:
            comfortStatus = "Холодно и влажно";
            break;
        case Comfort_ColdAndDry:
            comfortStatus = "Холодно и сухо";
            break;
        default:
            comfortStatus = "Неизвестно";
            break;
    };
    return comfortStatus;
}

//dhtDewpoint Точка#росы: Датчики 5
void dhtD() {
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String page_number = sCmd.next();
    choose_widget_and_create(widget_name, page_name, page_number, "any-data", "dhtDewpoint");
    sensors_reading_map[8] = 1;
}

void dhtD_reading() {
    float value;
    if (dht.getStatus() != 0) {
        sendSTATUS("dhtDewpoint", String(dht.getStatusString()));
    } else {
        value = dht.computeDewPoint(jsonReadStr(configLiveJson, dhtT_value_name).toFloat(), jsonReadStr(configLiveJson, dhtH_value_name).toFloat(), false);
        jsonWriteInt(configLiveJson, "dhtDewpoint", value);
        eventGen("dhtDewpoint", "");
        sendSTATUS("dhtDewpoint", String(value));
        Serial.println("[i] sensor 'dhtDewpoint' data: " + String(value));
    }
}
#endif
//=========================================i2c bus esp8266 scl-4 sda-5 ====================================================================
//=========================================================================================================================================
//=========================================Модуль сенсоров bmp280==========================================================================

//bmp280T temp1 0x76 Температура#bmp280 Датчики any-data 1
void bmp280T() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    bmp280T_value_name = value_name;
    choose_widget_and_create(widget_name, page_name, page_number, type, value_name);
    bmp.begin(hexStringToUint8(address));
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
    //bmp_temp->printSensorDetails();
    sensors_reading_map[9] = 1;
}

void bmp280T_reading() {
    float value = 0;
    sensors_event_t temp_event;
    bmp_temp->getEvent(&temp_event);
    value = temp_event.temperature;
    jsonWriteStr(configLiveJson, bmp280T_value_name, String(value));
    eventGen(bmp280T_value_name, "");
    sendSTATUS(bmp280T_value_name, String(value));
    Serial.println("[i] sensor '" + bmp280T_value_name + "' data: " + String(value));
}

//bmp280P press1 0x76 Давление#bmp280 Датчики any-data 2
void bmp280P() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    bmp280P_value_name = value_name;
    choose_widget_and_create(widget_name, page_name, page_number, type, value_name);
    bmp.begin(hexStringToUint8(address));
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
    //bmp_temp->printSensorDetails();
    sensors_reading_map[10] = 1;
}

void bmp280P_reading() {
    float value = 0;
    sensors_event_t pressure_event;
    bmp_pressure->getEvent(&pressure_event);
    value = pressure_event.pressure;
    value = value / 1.333224;
    jsonWriteStr(configLiveJson, bmp280P_value_name, String(value));
    eventGen(bmp280P_value_name, "");
    sendSTATUS(bmp280P_value_name, String(value));
    Serial.println("[i] sensor '" + bmp280P_value_name + "' data: " + String(value));
}

//=========================================================================================================================================
//=============================================Модуль сенсоров bme280======================================================================
//bme280T temp1 0x76 Температура#bmp280 Датчики any-data 1
void bme280T() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    bme280T_value_name = value_name;
    choose_widget_and_create(widget_name, page_name, page_number, type, value_name);
    bme.begin(hexStringToUint8(address));
    sensors_reading_map[11] = 1;
}

void bme280T_reading() {
    float value = 0;
    value = bme.readTemperature();
    jsonWriteStr(configLiveJson, bme280T_value_name, String(value));
    eventGen(bme280T_value_name, "");
    sendSTATUS(bme280T_value_name, String(value));
    Serial.println("[i] sensor '" + bme280T_value_name + "' data: " + String(value));
}

//bme280P pres1 0x76 Давление#bmp280 Датчики any-data 1
void bme280P() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    bme280P_value_name = value_name;
    choose_widget_and_create(widget_name, page_name, page_number, type, value_name);
    bme.begin(hexStringToUint8(address));
    sensors_reading_map[12] = 1;
}

void bme280P_reading() {
    float value = 0;
    value = bme.readPressure();
    value = value / 1.333224;
    jsonWriteStr(configLiveJson, bme280P_value_name, String(value));
    eventGen(bme280P_value_name, "");
    sendSTATUS(bme280P_value_name, String(value));
    Serial.println("[i] sensor '" + bme280P_value_name + "' data: " + String(value));
}

//bme280H hum1 0x76 Влажность#bmp280 Датчики any-data 1
void bme280H() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    bme280H_value_name = value_name;
    choose_widget_and_create(widget_name, page_name, page_number, type, value_name);
    bme.begin(hexStringToUint8(address));
    sensors_reading_map[13] = 1;
}

void bme280H_reading() {
    float value = 0;
    value = bme.readHumidity();
    jsonWriteStr(configLiveJson, bme280H_value_name, String(value));
    eventGen(bme280H_value_name, "");
    sendSTATUS(bme280H_value_name, String(value));
    Serial.println("[i] sensor '" + bme280H_value_name + "' data: " + String(value));
}

//bme280A altit1 0x76 Высота#bmp280 Датчики any-data 1
void bme280A() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    bme280A_value_name = value_name;
    choose_widget_and_create(widget_name, page_name, page_number, type, value_name);
    bme.begin(hexStringToUint8(address));
    sensors_reading_map[14] = 1;
}

void bme280A_reading() {
    float value = 0;
    value = bme.readAltitude(1013.25);
    jsonWriteStr(configLiveJson, bme280A_value_name, String(value));
    eventGen(bme280A_value_name, "");
    sendSTATUS(bme280A_value_name, String(value));
    Serial.println("[i] sensor '" + bme280A_value_name + "' data: " + String(value));
}
