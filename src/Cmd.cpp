#include "Cmd.h"

#include "Module/Terminal.h"
#include "MqttClient.h"
#include "WebClient.h"
#include "Sensors.h"
#include "Objects/Buttons.h"
#include "Objects/Pwms.h"
#include "Objects/Servos.h"
#include "Bus/OneWireBus.h"

static const char *MODULE = "Cmd";

Terminal *term = nullptr;

#ifdef ESP8266
SoftwareSerial *mySerial = nullptr;
#else
HardwareSerial *mySerial = nullptr;
#endif

void cmd_init() {
    myButtons.setOnChangeState([](Button_t *btn, uint8_t num) {
        fireEvent("switch", String(num, DEC));
        jsonWriteInt(configLiveJson, "switch" + String(num, DEC), btn->state);
    });

    sCmd.addCommand("button", cmd_button);
    sCmd.addCommand("buttonSet", cmd_buttonSet);
    sCmd.addCommand("buttonChange", cmd_buttonChange);
    sCmd.addCommand("switch", cmd_switch);

    sCmd.addCommand("pinSet", cmd_pinSet);
    sCmd.addCommand("pinChange", cmd_pinChange);

    sCmd.addCommand("pwm", cmd_pwm);
    sCmd.addCommand("pwmSet", cmd_pwmSet);

#ifdef ANALOG_ENABLED
    sCmd.addCommand("analog", cmd_analog);
#endif
#ifdef LEVEL_ENABLED
    sCmd.addCommand("levelPr", cmd_levelPr);
    sCmd.addCommand("ultrasonicCm", cmd_ultrasonicCm);
#endif
#ifdef DALLAS_ENABLED
    sCmd.addCommand("dallas", cmd_dallas);
#endif

    sCmd.addCommand("dhtT", cmd_dhtT);
    sCmd.addCommand("dhtH", cmd_dhtH);
    sCmd.addCommand("dhtPerception", cmd_dhtPerception);
    sCmd.addCommand("dhtComfort", cmd_dhtComfort);
    sCmd.addCommand("dhtDewpoint", cmd_dhtDewpoint);

    sCmd.addCommand("bmp280T", cmd_bmp280T);
    sCmd.addCommand("bmp280P", cmd_bmp280P);

    sCmd.addCommand("bme280T", cmd_bme280T);
    sCmd.addCommand("bme280P", cmd_bme280P);
    sCmd.addCommand("bme280H", cmd_bme280H);
    sCmd.addCommand("bme280A", cmd_bme280A);

    sCmd.addCommand("stepper", cmd_stepper);
    sCmd.addCommand("stepperSet", cmd_stepperSet);

    sCmd.addCommand("servo", cmd_servo);
    sCmd.addCommand("servoSet", cmd_servoSet);

    sCmd.addCommand("serialBegin", cmd_serialBegin);
    sCmd.addCommand("serialWrite", cmd_serialWrite);
    sCmd.addCommand("getData", cmd_getData);

    sCmd.addCommand("logging", logging);

    sCmd.addCommand("inputDigit", cmd_inputDigit);
    sCmd.addCommand("digitSet", cmd_digitSet);

    sCmd.addCommand("inputTime", cmd_inputTime);
    sCmd.addCommand("timeSet", cmd_timeSet);

    sCmd.addCommand("timerStart", timerStart_);
    sCmd.addCommand("timerStop", timerStop_);

    sCmd.addCommand("text", cmd_text);
    sCmd.addCommand("textSet", cmd_textSet);

    sCmd.addCommand("mqtt", cmd_mqtt);
    sCmd.addCommand("http", cmd_http);

    sCmd.addCommand("push", pushControl);

    sCmd.addCommand("firmwareUpdate", cmd_firmwareUpdate);
    sCmd.addCommand("firmwareVersion", cmd_firmwareVersion);
}

void cmd_button() {
    String name = sCmd.next();
    String assign = sCmd.next();
    String description = sCmd.next();
    String page = sCmd.next();
    String param = sCmd.next();
    String order = sCmd.next();

    jsonWriteStr(configLiveJson, "button" + name, param);

    if (isDigitStr(assign)) {
        myButtons.addButton(name, assign, param);
    }
    if (assign == "scen") {
        config.general()->enableScenario(param.toInt());
    } else if (assign.startsWith("line")) {
        String str = assign;
        while (str.length()) {
            if (str == "") {
                return;
            }
            //line1,
            String tmp = selectToMarker(str, ",");
            //1,
            String number = deleteBeforeDelimiter(tmp, "e");
            number.replace(",", "");
            int number_int = number.toInt();

            Scenario::enableBlock(number_int, param);

            str = deleteBeforeDelimiter(str, ",");
        }
    }

    createWidget(description, page, order, "toggle", "button" + name);
}

void cmd_buttonSet() {
    String name = sCmd.next();
    String state = sCmd.next();

    Button_t *btn = myButtons.get(name);

    if (isDigitStr(btn->assign)) {
        btn->set(state.toInt());
    } else if (btn->assign == "scen") {
        config.general()->enableScenario(state.toInt());
    } else if (btn->assign.startsWith("line")) {
        String str = btn->assign;
        while (str.length() != 0) {
            String tmp = selectToMarker(str, ",");            //line1,
            String number = deleteBeforeDelimiter(tmp, "e");  //1,
            number.replace(",", "");

            Scenario::enableBlock(number.toInt(), state.toInt());

            str = deleteBeforeDelimiter(str, ",");
        }
    }

    fireEvent("button", name);

    jsonWriteStr(configLiveJson, "button" + name, state);

    MqttClient::publishStatus("button" + name, state);
}

void cmd_buttonChange() {
    String name = sCmd.next();

    if (!isDigitStr(name)) {
        pm.error("wrong button " + name);
        return;
    }
    Button_t *btn = myButtons.get(name);

    String stateStr = btn->toggle() ? "1" : "0";

    jsonWriteStr(configLiveJson, "button" + name, stateStr);

    MqttClient::publishStatus("button" + name, stateStr);
}

void cmd_pinSet() {
    String pin_number = sCmd.next();
    String pin_state = sCmd.next();
    pinMode(pin_number.toInt(), OUTPUT);
    digitalWrite(pin_number.toInt(), pin_state.toInt());
}

void cmd_pinChange() {
    String pin_number = sCmd.next();
    pinMode(pin_number.toInt(), OUTPUT);
    digitalWrite(pin_number.toInt(), !digitalRead(pin_number.toInt()));
}
//==================================================================================================================
//==========================================Модуль управления ШИМ===================================================
void cmd_pwm() {
    String name = sCmd.next();
    String assign = sCmd.next();
    String description = sCmd.next();

    String page = sCmd.next();
    String state = sCmd.next();
    String order = sCmd.next();

    String style = sCmd.next();

    myPwms.add(name, assign, state);
    // jsonWriteStr(configOptionJson, "pwm_pin" + name, assign);
    // jsonWriteStr(configLiveJson, "pwm" + name, state);
    // pinMode(assign.toInt(), OUTPUT);
    // analogWrite(assign.toInt(), state.toInt());
    createWidget(description, page, order, "range", "pwm" + name);
}

void cmd_pwmSet() {
    String name = sCmd.next();
    String state = sCmd.next();
    myPwms.get(name)->setInt(state.toInt());
    fireEvent("pwm", name);
    // analogWrite(pin, state.toInt());
    // int pin = jsonReadInt(configOptionJson, "pwm_pin" + name);
    // jsonWriteStr(configLiveJson, "pwm" + name, state);
    MqttClient::publishStatus("pwm" + name, state);
}

void cmd_switch() {
    String name = sCmd.next();
    String assign = sCmd.next();
    String debounce = sCmd.next();

    myButtons.addSwitch(name, assign, debounce);
}

void loop_serial() {
    if (term) {
        term->loop();
    }
}

void loop_button() {
    myButtons.loop();
}

//=====================================================================================================================================
//=========================================Добавление окна ввода цифры=================================================================
void cmd_inputDigit() {
    String value_name = sCmd.next();
    String number = value_name.substring(5);
    String widget_name = sCmd.next();
    widget_name.replace("#", " ");
    String page_name = sCmd.next();
    page_name.replace("#", " ");
    String start_state = sCmd.next();
    String page_number = sCmd.next();

    jsonWriteStr(configLiveJson, "digit" + number, start_state);
    createWidget(widget_name, page_name, page_number, "inputNum", "digit" + number);
}

void cmd_digitSet() {
    String number = sCmd.next();
    String value = sCmd.next();

    jsonWriteStr(configLiveJson, "digit" + number, value);
    MqttClient::publishStatus("digit" + number, value);
}

//=====================================================================================================================================
//=========================================Добавление окна ввода времени===============================================================
void cmd_inputTime() {
    String value_name = sCmd.next();
    String number = value_name.substring(4);
    String widget_name = sCmd.next();
    widget_name.replace("#", " ");
    String page_name = sCmd.next();
    page_name.replace("#", " ");
    String start_state = sCmd.next();
    String page_number = sCmd.next();

    jsonWriteStr(configLiveJson, "time" + number, start_state);
    createWidget(widget_name, page_name, page_number, "inputTime", "time" + number);
}

void cmd_timeSet() {
    String number = sCmd.next();
    String value = sCmd.next();

    jsonWriteStr(configLiveJson, "time" + number, value);
    MqttClient::publishStatus("time" + number, value);
}

void cmd_text() {
    String number = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String page_number = sCmd.next();

    createWidget(widget_name, page_name, page_number, "anydata", "text" + number);
}

void cmd_textSet() {
    String number = sCmd.next();
    String text = sCmd.next();
    text.replace("_", " ");

    if (text.indexOf("-time") >= 0) {
        text.replace("-time", "");
        text.replace("#", " ");
        text = text + " " + timeNow->getDateTimeDotFormated();
    }

    jsonWriteStr(configLiveJson, "text" + number, text);
    MqttClient::publishStatus("text" + number, text);
}
//=====================================================================================================================================
//=========================================Модуль шагового мотора======================================================================

//stepper 1 12 13
void cmd_stepper() {
    String stepper_number = sCmd.next();
    String pin_step = sCmd.next();
    String pin_dir = sCmd.next();

    jsonWriteStr(configOptionJson, "stepper" + stepper_number, pin_step + " " + pin_dir);
    pinMode(pin_step.toInt(), OUTPUT);
    pinMode(pin_dir.toInt(), OUTPUT);
}

//stepperSet 1 100 5
void cmd_stepperSet() {
    String stepper_number = sCmd.next();
    String steps = sCmd.next();
    jsonWriteStr(configOptionJson, "steps" + stepper_number, steps);
    String stepper_speed = sCmd.next();
    String pin_step = selectToMarker(jsonReadStr(configOptionJson, "stepper" + stepper_number), " ");
    String pin_dir = deleteBeforeDelimiter(jsonReadStr(configOptionJson, "stepper" + stepper_number), " ");
    Serial.println(pin_step);
    Serial.println(pin_dir);
    if (steps.toInt() > 0) digitalWrite(pin_dir.toInt(), HIGH);
    if (steps.toInt() < 0) digitalWrite(pin_dir.toInt(), LOW);
    if (stepper_number == "1") {
        ts.add(
            STEPPER1, stepper_speed.toInt(), [&](void *) {
                int steps_int = abs(jsonReadInt(configOptionJson, "steps1") * 2);
                static int count;
                count++;
                String pin_step = selectToMarker(jsonReadStr(configOptionJson, "stepper1"), " ");
                digitalWrite(pin_step.toInt(), !digitalRead(pin_step.toInt()));
                yield();
                if (count > steps_int) {
                    digitalWrite(pin_step.toInt(), LOW);
                    ts.remove(STEPPER1);
                    count = 0;
                }
            },
            nullptr, true);
    }
    if (stepper_number == "2") {
        ts.add(
            STEPPER2, stepper_speed.toInt(), [&](void *) {
                int steps_int = abs(jsonReadInt(configOptionJson, "steps2") * 2);
                static int count;
                count++;
                String pin_step = selectToMarker(jsonReadStr(configOptionJson, "stepper2"), " ");
                digitalWrite(pin_step.toInt(), !digitalRead(pin_step.toInt()));
                yield();
                if (count > steps_int) {
                    digitalWrite(pin_step.toInt(), LOW);
                    ts.remove(STEPPER2);
                    count = 0;
                }
            },
            nullptr, true);
    }
}

//servo 1 13 50 Мой#сервопривод Сервоприводы 0 100 0 180 2
void cmd_servo() {
    String number = sCmd.next();
    uint8_t pin = String(sCmd.next()).toInt();
    int value = String(sCmd.next()).toInt();

    String widget = sCmd.next();
    String page = sCmd.next();

    int min_value = String(sCmd.next()).toInt();
    int max_value = String(sCmd.next()).toInt();
    int min_deg = String(sCmd.next()).toInt();
    int max_deg = String(sCmd.next()).toInt();

    String pageNumber = sCmd.next();

    jsonWriteStr(configOptionJson, "servo_pin" + number, String(pin, DEC));

    value = map(value, min_value, max_value, min_deg, max_deg);

    Servo *servo = myServo.create(number.toInt(), pin);
    servo->write(value);

    jsonWriteInt(configOptionJson, "s_min_val" + number, min_value);
    jsonWriteInt(configOptionJson, "s_max_val" + number, max_value);
    jsonWriteInt(configOptionJson, "s_min_deg" + number, min_deg);
    jsonWriteInt(configOptionJson, "s_max_deg" + number, max_deg);

    jsonWriteInt(configLiveJson, "servo" + number, value);

    createWidget(widget, page, pageNumber, "range", "servo" + number, "min", String(min_value), "max", String(max_value), "k", "1");
}

void cmd_servoSet() {
    String number = sCmd.next();
    int value = String(sCmd.next()).toInt();

    value = map(value,
                jsonReadInt(configOptionJson, "s_min_val" + number),
                jsonReadInt(configOptionJson, "s_max_val" + number),
                jsonReadInt(configOptionJson, "s_min_deg" + number),
                jsonReadInt(configOptionJson, "s_max_deg" + number));

    Servo *servo = myServo.get(number.toInt());
    if (servo) {
        servo->write(value);
    }

    fireEvent("servo", number);
    jsonWriteInt(configLiveJson, "servo" + number, value);
    MqttClient::publishStatus("servo" + number, String(value, DEC));
}

void cmd_serialBegin() {
    String s_speed = sCmd.next();
    String rxPin = sCmd.next();
    String txPin = sCmd.next();

    if (mySerial) {
        delete mySerial;
    }

#ifdef ESP8266
    mySerial = new SoftwareSerial(rxPin.toInt(), txPin.toInt());
    mySerial->begin(s_speed.toInt());
#else
    mySerial = new HardwareSerial(2);
    mySerial->begin(rxPin.toInt(), txPin.toInt());
#endif

    term = new Terminal(mySerial);
    term->setEOL(LF);
    term->enableColors(false);
    term->enableControlCodes(false);
    term->enableEcho(false);
    term->setOnReadLine([](const char *str) {
        String line = String(str);
        addCommandLoop(line);
    });
}

void cmd_getData() {
    String param = sCmd.next();
    String res = param.length() ? jsonReadStr(configLiveJson, param) : configLiveJson;
    if (term) {
        term->println(res.c_str());
    }
}

void cmd_serialWrite() {
    String payload = sCmd.next();
    if (term) {
        term->println(payload.c_str());
    }
}

void cmd_mqtt() {
    String topic = sCmd.next();
    String data = sCmd.next();

    MqttClient::publishOrder(topic, data);
}

void cmd_onewire() {
    uint8_t pin = (uint8_t)String(sCmd.next()).toInt();
    oneWireBus.set(pin);
    if (!oneWireBus.exists()) {
        pm.error("on create OneWire");
    }
}

//ultrasonicCm cm 14 12 Дистанция,#см Датчики fillgauge 1
void cmd_ultrasonicCm() {
    String measure_unit = sCmd.next();

    String trig = sCmd.next();
    String echo = sCmd.next();
    String widget = sCmd.next();
    String page = sCmd.next();
    String type = sCmd.next();
    String empty_level = sCmd.next();
    String full_level = sCmd.next();
    String page_number = sCmd.next();

    ultrasonicCm_value_name = measure_unit;

    jsonWriteStr(configOptionJson, "trig", trig);
    jsonWriteStr(configOptionJson, "echo", echo);
    pinMode(trig.toInt(), OUTPUT);
    pinMode(echo.toInt(), INPUT);

    createWidget(widget, page, page_number, type, measure_unit);

    Sensors::enable(0);
}

//dallas temp1 0x14 Температура Датчики anydata 1
//dallas temp2 0x15 Температура Датчики anydata 2
void cmd_dallas() {
    if (!oneWireBus.exists()) {
        pm.error("needs OneWire");
        return;
    }

    if (dallasSensors.count() == 0) {
        dallasTemperature = new DallasTemperature(oneWireBus.get());
        dallasTemperature->begin();
        dallasTemperature->setResolution(12);
    }

    String value = sCmd.next();

    // uint8_t *address = String(sCmd.next()).toInt();

    // dallasSensors.create(address, value);

    String widget = sCmd.next();
    String page = sCmd.next();
    String type = sCmd.next();
    String pageNumber = sCmd.next();

    // jsonWriteStr(configOptionJson, value + "_ds", String(address, DEC));

    dallas_value_name += value + ";";
    createWidget(widget, page, pageNumber, type, value);

    Sensors::enable(3);
}

//levelPr p 14 12 Вода#в#баке,#% Датчики fillgauge 125 20 1
void cmd_levelPr() {
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
    createWidget(widget_name, page_name, page_number, type, value_name);

    Sensors::enable(0);
}

//dhtH h 2 dht11 Влажность#DHT,#t°C Датчики any-data 1
void cmd_dhtH() {
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
    createWidget(widget_name, page_name, page_number, type, value_name);

    Sensors::enable(5);
}

// dhtT t 2 dht11 Температура#DHT,#t°C Датчики any-data 1
void cmd_dhtT() {
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

    createWidget(widget_name, page_name, page_number, type, value_name);

    Sensors::enable(4);
}

//dhtDewpoint Точка#росы: Датчики 5
void cmd_dhtDewpoint() {
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String page_number = sCmd.next();

    createWidget(widget_name, page_name, page_number, "anydata", "dhtDewpoint");

    Sensors::enable(8);
}

// dhtPerception Восприятие: Датчики 4
void cmd_dhtPerception() {
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String page_number = sCmd.next();

    createWidget(widget_name, page_name, page_number, "any-data", "dhtPerception");

    Sensors::enable(6);
}

// dhtComfort Степень#комфорта: Датчики 3
void cmd_dhtComfort() {
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String page_number = sCmd.next();

    createWidget(widget_name, page_name, page_number, "anydata", "dhtComfort");

    Sensors::enable(7);
}

// analog adc 0 Аналоговый#вход,#% Датчики any-data 1 1023 1 100 1
void cmd_analog() {
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

    createWidget(widget_name, page_name, page_number, type, value_name);

    if (enter_to_analog_counter == 1) {
        Sensors::enable(1);
    }
    if (enter_to_analog_counter == 2) {
        Sensors::enable(2);
    }
}

// bmp280T temp1 0x76 Температура#bmp280 Датчики any-data 1
void cmd_bmp280T() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    bmp280T_value_name = value_name;

    bmp.begin(hexStringToUint8(address));
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    createWidget(widget_name, page_name, page_number, type, value_name);

    Sensors::enable(9);
}

//bmp280P press1 0x76 Давление#bmp280 Датчики any-data 2
void cmd_bmp280P() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    bmp280P_value_name = value_name;

    bmp.begin(hexStringToUint8(address));
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    createWidget(widget_name, page_name, page_number, type, value_name);

    Sensors::enable(10);
}

//=========================================================================================================================================
//=============================================Модуль сенсоров bme280======================================================================
//bme280T temp1 0x76 Температура#bmp280 Датчики any-data 1
void cmd_bme280T() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    bme280T_value_name = value_name;

    bme.begin(hexStringToUint8(address));

    createWidget(widget_name, page_name, page_number, type, value_name);

    Sensors::enable(11);
}

//bme280P pres1 0x76 Давление#bmp280 Датчики any-data 1
void cmd_bme280P() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    bme280P_value_name = value_name;

    bme.begin(hexStringToUint8(address));

    createWidget(widget_name, page_name, page_number, type, value_name);

    Sensors::enable(12);
}

//bme280H hum1 0x76 Влажность#bmp280 Датчики any-data 1
void cmd_bme280H() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    bme280H_value_name = value_name;

    bme.begin(hexStringToUint8(address));

    createWidget(widget_name, page_name, page_number, type, value_name);

    Sensors::enable(13);
}

//bme280A altit1 0x76 Высота#bmp280 Датчики any-data 1
void cmd_bme280A() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    bme280A_value_name = value_name;

    bme.begin(hexStringToUint8(address));

    createWidget(widget_name, page_name, page_number, type, value_name);

    Sensors::enable(14);
}

void cmd_http() {
    String host = sCmd.next();
    String param = sCmd.next();
    param.replace("_", "%20");

    String url = "http://" + host + "/cmd?command=" + param;
    WebClient::get(url);
}

void cmd_firmwareUpdate() {
    perform_upgrade = true;
}

void cmd_firmwareVersion() {
    String widget = sCmd.next();
    String page = sCmd.next();
    String pageNumber = sCmd.next();

    jsonWriteStr(configLiveJson, "firmver", FIRMWARE_VERSION);
    createWidget(widget, page, pageNumber, "anydata", "firmver");
}

void addCommandLoop(const String &cmdStr) {
    order_loop += cmdStr;
    if (!cmdStr.endsWith(",")) {
        order_loop += ",";
    }
}

void fileExecute(const String filename) {
    String cmdStr = readFile(filename, 2048);
    stringExecute(cmdStr);
}

void stringExecute(String &cmdStr) {
    cmdStr += "\r\n";
    cmdStr.replace("\r\n", "\n");
    cmdStr.replace("\r", "\n");

    while (cmdStr.length()) {
        String buf = selectToMarker(cmdStr, "\n");
        sCmd.readStr(buf);
        cmdStr = deleteBeforeDelimiter(cmdStr, "\n");
    }
}

void loopCmd() {
    if (order_loop.length()) {
        //выделяем первую команду rel 5 1,
        String block = selectToMarker(order_loop, ",");
        pm.info("execute: " + block);
        //выполняем
        sCmd.readStr(block);
        //осекаем
        order_loop = deleteBeforeDelimiter(order_loop, ",");
    }
}
