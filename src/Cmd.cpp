#include "Cmd.h"

#include "Consts.h"
#include "Devices.h"
#include "Events.h"
#include "Logger.h"
#include "MqttClient.h"
#include "WebClient.h"
#include "Sensors.h"
#include "Scenario.h"
#include "Sensors/AnalogSensor.h"
#include "Sensors/DallasSensors.h"
#include "Sensors/OneWireBus.h"
#include "Timers.h"
#include "Objects/Switches.h"
#include "Objects/Buttons.h"
#include "Objects/PwmItems.h"
#include "Objects/ServoItems.h"
#include "Objects/Terminal.h"
#include "Objects/Telnet.h"
#include "Utils/PrintMessage.h"
#include "Base/StringCommand.h"

static const char *MODULE = "Cmd";

StringQueue _orders;

StringCommand sCmd;

Terminal *term = nullptr;
Telnet *telnet = nullptr;

#ifdef ESP8266
SoftwareSerial *mySerial = nullptr;
#else
HardwareSerial *mySerial = nullptr;
#endif

void cmd_init() {
    pm.info(TAG_INIT);
    mySwitches.setOnChangeState([](Switch *obj) {
        String name = String("switch") + obj->getName();
        pm.info(name);
        Scenario::fire(name);
        liveData.writeInt(name, obj->getState());
    });

    sCmd.addCommand("button", cmd_button);
    sCmd.addCommand("buttonSet", cmd_buttonSet);
    sCmd.addCommand("buttonChange", cmd_buttonChange);

    sCmd.addCommand("switch", cmd_switch);

    sCmd.addCommand("pinSet", cmd_pinSet);
    sCmd.addCommand("pinChange", cmd_pinChange);

    sCmd.addCommand("pwm", cmd_pwm);
    sCmd.addCommand("pwmSet", cmd_pwmSet);

    sCmd.addCommand("analog", cmd_analog);

    sCmd.addCommand("levelPr", cmd_levelPr);
    sCmd.addCommand("ultrasonicCm", cmd_ultrasonicCm);
    sCmd.addCommand("dallas", cmd_dallas);
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
    sCmd.addCommand("serialEnd", cmd_serialEnd);
    sCmd.addCommand("serialWrite", cmd_serialWrite);
    sCmd.addCommand("serialLog", cmd_serialLog);

    sCmd.addCommand("telnet", cmd_telnet);

    sCmd.addCommand("logging", cmd_logging);

    sCmd.addCommand("inputDigit", cmd_inputDigit);
    sCmd.addCommand("digitSet", cmd_digitSet);

    sCmd.addCommand("inputTime", cmd_inputTime);
    sCmd.addCommand("timeSet", cmd_timeSet);

    sCmd.addCommand("timerStart", cmd_timerStart);
    sCmd.addCommand("timerStop", cmd_timerStop);

    sCmd.addCommand("text", cmd_text);
    sCmd.addCommand("textSet", cmd_textSet);

    sCmd.addCommand("mqtt", cmd_mqtt);
    sCmd.addCommand("http", cmd_http);

    sCmd.addCommand("push", cmd_push);

    sCmd.addCommand("firmwareUpdate", cmd_firmwareUpdate);
    sCmd.addCommand("firmwareVersion", cmd_firmwareVersion);

    sCmd.addCommand("get", cmd_get);

    sCmd.addCommand("reboot", cmd_reboot);

    sCmd.addCommand("oneWire", cmd_oneWire);
}

unsigned long parsePeriod(const String &str, unsigned long default_multiplier = ONE_MINUTE_ms) {
    unsigned long res = 0;
    if (str.endsWith("ms")) {
        res = str.substring(0, str.indexOf("ms")).toInt();
    } else if (str.endsWith("s")) {
        res = str.substring(0, str.indexOf("s")).toInt() * ONE_SECOND_ms;
    } else if (str.endsWith("m")) {
        res = str.substring(0, str.indexOf("m")).toInt() * ONE_MINUTE_ms;
    } else if (str.endsWith("h")) {
        res = str.substring(0, str.indexOf("h")).toInt() * ONE_HOUR_ms;
    } else {
        res = str.toInt() * default_multiplier;
    }
    return res;
}

//logging temp1 1 10 Температура Датчики 2
void cmd_logging() {
    String name = sCmd.next();
    String period = sCmd.next();
    String limit = sCmd.next();
    String descr = sCmd.next();
    String page = sCmd.next();
    int order = sCmd.nextInt();

    Logger::add(name, parsePeriod(period), limit.toInt());
    // /prefix/3234045-1589487/value_name_ch
    createChart(descr, page, order, "chart", name + "_ch", limit);
}

void cmd_button() {
    String name = sCmd.next();
    String assign = sCmd.next();
    String descr = sCmd.next();
    String page = sCmd.next();
    String state = sCmd.next();
    int order = sCmd.nextInt();
    String inverted = sCmd.next();

    myButtons.add(name, assign, state, inverted);

    liveData.write("button" + name, state);

    createWidget(descr, page, order, "toggle", "button" + name);

    // if (assign == "scen") {
    //     config.general()->enableScenario(state.toInt());
    // } else if (assign.startsWith("line")) {
    //     String str = assign;
    //     while (str.length()) {
    //         if (str == "") {
    //             return;
    //         }
    //         //line1,
    //         String tmp = selectToMarker(str, ",");
    //         //1,
    //         String number = deleteBeforeDelimiter(tmp, "e");
    //         number.replace(",", "");
    //         int number_int = number.toInt();

    //         Scenario::enableBlock(number_int, state);

    //         str = deleteBeforeDelimiter(str, ",");
    //     }
    // }
}
void cmd_buttonSet() {
    String name = sCmd.next();
    String state = sCmd.next();

    ButtonItem *btn = myButtons.get(name);
    String assign = btn->getAssign();

    if (isDigitStr(assign)) {
        btn->setState(state.toInt());
    } else if (assign == "scen") {
        config.general()->enableScenario(state);
    } else if (assign.startsWith("line")) {
        while (assign.length() != 0) {
            String tmp = selectToMarker(assign, ",");         //line1,
            String number = deleteBeforeDelimiter(tmp, "e");  //1,
            number.replace(",", "");
            Scenario::enableBlock(number.toInt(), state);
            assign = deleteBeforeDelimiter(assign, ",");
        }
    }
    String objName = "button" + name;
    Scenario::fire(objName);
    liveData.writeInt(objName, state.toInt());
    MqttClient::publishStatus(VT_INT, objName, state);
}

void cmd_buttonChange() {
    String name = sCmd.next();

    if (!isDigitStr(name)) {
        pm.error("wrong button " + name);
        return;
    }

    ButtonItem *btn = myButtons.get(name);
    btn->toggleState();

    String objName = "button" + name;

    liveData.writeInt(objName, btn->getState());
    MqttClient::publishStatus(VT_INT, objName, String(btn->getState(), DEC));
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

void cmd_pwm() {
    String name = sCmd.next();
    String assign = sCmd.next();
    String description = sCmd.next();

    String page = sCmd.next();
    String value = sCmd.next();
    int order = sCmd.nextInt();

    String style = sCmd.next();

    myPwm.add(name, assign, value);

    liveData.writeInt("pwm" + name, value.toInt());

    createWidget(description, page, order, "range", "pwm" + name);
}

void cmd_pwmSet() {
    String name = sCmd.next();
    String value = sCmd.next();

    myPwm.get(name)->setState(value.toInt());

    String objName = "pwm" + name;

    Scenario::fire(objName);

    liveData.writeInt(objName, value.toInt());

    MqttClient::publishStatus(VT_INT, objName, value);
}

void cmd_switch() {
    String name = sCmd.next();
    String assign = sCmd.next();
    int debounce = sCmd.nextInt();

    Switch *item = mySwitches.add(name, assign);
    if (!item) {
        pm.error("bad command");
        return;
    }

    item->setDebounce(debounce);

    liveData.writeInt(String("switch") + name, item->getState());
}

void loop_items() {
    if (term) {
        term->loop();
    }
    if (telnet) {
        telnet->loop();
    }

    mySwitches.loop();
}

void cmd_inputDigit() {
    String value_name = sCmd.next();
    String number = value_name.substring(5);
    String widget_name = sCmd.next();
    widget_name.replace("#", " ");
    String page_name = sCmd.next();
    page_name.replace("#", " ");
    String start_state = sCmd.next();
    int order = sCmd.nextInt();

    liveData.write("digit" + number, start_state);
    createWidget(widget_name, page_name, order, "inputNum", "digit" + number);
}

void cmd_digitSet() {
    String name = sCmd.next();
    String value = sCmd.next();

    String objName = "pwm" + name;
    liveData.write(objName, value);
    MqttClient::publishStatus(VT_STRING, objName, value);
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
    int order = sCmd.nextInt();

    liveData.write("time" + number, start_state);
    createWidget(widget_name, page_name, order, "inputTime", "time" + number);
}

void cmd_timeSet() {
    String name = sCmd.next();
    String value = sCmd.next();

    String objName = "time" + name;
    liveData.write(objName, value);
    MqttClient::publishStatus(VT_STRING, objName, value);
}

void cmd_text() {
    String number = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    int order = sCmd.nextInt();

    createWidget(widget_name, page_name, order, "anydata", "text" + number);
}

void cmd_textSet() {
    String name = sCmd.next();
    String text = sCmd.next();
    text.replace("_", " ");

    if (text.indexOf("-time") >= 0) {
        text.replace("-time", "");
        text.replace("#", " ");
        text = text + " " + timeNow->getDateTimeDotFormated();
    }

    String objName = "text" + name;
    liveData.write(objName, text);
    MqttClient::publishStatus(VT_STRING, objName, text);
}

void cmd_stepper() {
    String name = sCmd.next();
    String pin_step = sCmd.next();
    String pin_dir = sCmd.next();

    liveData.write("stepper" + name, pin_step + " " + pin_dir);

    pinMode(pin_step.toInt(), OUTPUT);
    pinMode(pin_dir.toInt(), OUTPUT);
}

void cmd_stepperSet() {
    String stepper_number = sCmd.next();
    String steps = sCmd.next();
    options.write("steps" + stepper_number, steps);
    String stepper_speed = sCmd.next();
    String pin_step = selectToMarker(options.read("stepper" + stepper_number), " ");
    String pin_dir = deleteBeforeDelimiter(options.read("stepper" + stepper_number), " ");
    Serial.println(pin_step);
    Serial.println(pin_dir);
    if (steps.toInt() > 0) digitalWrite(pin_dir.toInt(), HIGH);
    if (steps.toInt() < 0) digitalWrite(pin_dir.toInt(), LOW);
    if (stepper_number == "1") {
        ts.add(
            STEPPER1, stepper_speed.toInt(), [&](void *) {
                int steps_int = abs(options.readInt("steps1") * 2);
                static int count;
                count++;
                String pin_step = selectToMarker(options.read("stepper1"), " ");
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
                int steps_int = abs(options.readInt("steps2") * 2);
                static int count;
                count++;
                String pin_step = selectToMarker(options.read("stepper2"), " ");
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

void cmd_servo() {
    //servo 1 13 50 Cервопривод Сервоприводы 0 100 0 180 2
    String name = sCmd.next();
    String pin = sCmd.next();
    String value = sCmd.next();

    String descr = sCmd.next();
    String page = sCmd.next();

    String min_value = sCmd.next();
    String max_value = sCmd.next();
    String min_deg = sCmd.next();
    String max_deg = sCmd.next();

    int order = sCmd.nextInt();

    myServo.add(name, pin, value, min_value, max_value, min_deg, max_deg);

    // options.write("servo_pin" + name, pin);
    // value = map(value, min_value, max_value, min_deg, max_deg);
    // servo->write(value);
    // options.writeInt("s_min_val" + name, min_value);
    // options.writeInt("s_max_val" + name, max_value);
    // options.writeInt("s_min_deg" + name, min_deg);
    // options.writeInt("s_max_deg" + name, max_deg);
    // liveData.writeInt("servo" + name, value);

    createWidget(descr, page, order, "range", "servo" + name, "min", String(min_value), "max", String(max_value), "k", "1");
}

void cmd_servoSet() {
    String name = sCmd.next();
    int value = String(sCmd.next()).toInt();

    BaseServo *servo = myServo.get(name);
    if (servo) {
        servo->setState(value);
    } else {
        pm.error("servo: " + name + " not found");
    }

    String objName = "servo" + name;

    Scenario::fire(objName);
    liveData.writeInt(objName, value);
    MqttClient::publishStatus(VT_INT, objName, String(value, DEC));
}

void cmd_serialBegin() {
    uint32_t baud = atoi(sCmd.next());
    int8_t rx = atoi(sCmd.next());
    int8_t tx = atoi(sCmd.next());

    cmd_serialEnd();
#ifdef ESP8266
    mySerial = new SoftwareSerial(rx, tx);
    mySerial->begin(baud);
#else
    mySerial = new HardwareSerial(2);
    mySerial->begin(rx, tx);
#endif
    term = new Terminal(mySerial);
    term->setOnReadLine([](const char *str) {
        addOrder(str);
    });
}

void cmd_serialEnd() {
    if (mySerial) {
        mySerial->end();
        delete mySerial;
    }
}

void cmd_serialWrite() {
    String payload = sCmd.next();
    if (term) {
        term->println(payload.c_str());
    }
}

void cmd_serialLog() {
    bool enabled = atoi(sCmd.next());
    String out = sCmd.next();

    pm.enablePrint(enabled);
    pm.setOutput(out.equalsIgnoreCase("serial1") ? &Serial : &Serial);

    if (enabled) {
        pm.info("serialLog: enabled");
    }
}

void cmd_telnet() {
    bool enabled = atoi(sCmd.next());
    uint16_t port = atoi(sCmd.next());

    if (enabled) {
        pm.info("telnet: enabled");
        if (!telnet) {
            telnet = new Telnet(port);
        }
        telnet->setOutput(pm.getOutput());
        telnet->start();
    } else {
        pm.info("telnet: disabled");
        telnet->stop();
        telnet->end();
    }
}

void cmd_get() {
    String obj = sCmd.next();
    String param = sCmd.next();
    String res = "";
    if (!obj.isEmpty()) {
        if (obj.equalsIgnoreCase(TAG_OPTIONS)) {
            res = param.isEmpty() ? options.asJson() : options.read(param);
        } else if (obj.equalsIgnoreCase(TAG_RUNTIME)) {
            res = param.isEmpty() ? runtime.asJson() : runtime.read(param);
        } else if (obj.equalsIgnoreCase("state")) {
            res = param.isEmpty() ? liveData.asJson() : liveData.read(param);
        } else if (obj.equalsIgnoreCase("devices")) {
            Devices::get(res, param.toInt());
        } else {
            res = F("unknown param");
        }
    } else {
        res = F("unknown obj");
    }
    pm.info(res);
    if (term) {
        term->println(res.c_str());
    }
}

void cmd_mqtt() {
    String topic = sCmd.next();
    String data = sCmd.next();

    MqttClient::publishOrder(topic, data);
}

// ultrasonicCm cm 14 12 Дистанция,#см Датчики fillgauge 1
void cmd_ultrasonicCm() {
    String measure_unit = sCmd.next();

    String trig = sCmd.next();
    String echo = sCmd.next();
    String widget = sCmd.next();
    String page = sCmd.next();
    String type = sCmd.next();
    String empty_level = sCmd.next();
    String full_level = sCmd.next();
    int order = sCmd.nextInt();

    Ultrasonic::ultrasonicCm_value_name = measure_unit;

    options.write("trig", trig);
    options.write("echo", echo);
    pinMode(trig.toInt(), OUTPUT);
    pinMode(echo.toInt(), INPUT);

    createWidget(widget, page, order, type, measure_unit);
}

void cmd_oneWire() {
    uint8_t pin = sCmd.nextInt();
    onewire.attach(pin);
}

// dallas temp1 0x14 Температура Датчики anydata 1
// dallas temp2 0x15 Температура Датчики anydata 2
void cmd_dallas() {
    if (!onewire.attached()) {
        pm.error("attach bus first");
        return;
    }

    String name = sCmd.next();
    String address = sCmd.next();
    String widget = sCmd.next();
    String page = sCmd.next();
    String type = sCmd.next();
    int order = sCmd.nextInt();

    // uint8_t *address = String(sCmd.next()).toInt();
    dallasSensors.add(name, address);

    createWidget(widget, page, order, type, name);
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
    int order = sCmd.nextInt();

    Ultrasonic::levelPr_value_name = value_name;

    options.write("e_lev", empty_level);
    options.write("f_lev", full_level);
    options.write("trig", trig);
    options.write("echo", echo);
    pinMode(trig.toInt(), OUTPUT);
    pinMode(echo.toInt(), INPUT);
    createWidget(widget_name, page_name, order, type, value_name);
}

//dhtH h 2 dht11 Влажность#DHT,#t°C Датчики any-data 1
void cmd_dhtH() {
    String value_name = sCmd.next();
    String pin = sCmd.next();
    String sensor_type = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    int order = sCmd.nextInt();
    DHTSensor::dhtH_value_name = value_name;
    if (sensor_type == "dht11") {
        DHTSensor::dht.setup(pin.toInt(), DHTesp::DHT11);
    }
    if (sensor_type == "dht22") {
        DHTSensor::dht.setup(pin.toInt(), DHTesp::DHT22);
    }
    createWidget(widget_name, page_name, order, type, value_name);
}

// dhtT t 2 dht11 Температура#DHT,#t°C Датчики any-data 1
void cmd_dhtT() {
    String value_name = sCmd.next();
    String pin = sCmd.next();
    String sensor_type = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    int order = sCmd.nextInt();
    DHTSensor::dhtT_value_name = value_name;
    if (sensor_type == "dht11") {
        DHTSensor::dht.setup(pin.toInt(), DHTesp::DHT11);
    }
    if (sensor_type == "dht22") {
        DHTSensor::dht.setup(pin.toInt(), DHTesp::DHT22);
    }

    createWidget(widget_name, page_name, order, type, value_name);
}

//dhtDewpoint Точка#росы: Датчики 5
void cmd_dhtDewpoint() {
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    int order = sCmd.nextInt();

    createWidget(widget_name, page_name, order, "anydata", "dhtDewpoint");
}

// dhtPerception Восприятие: Датчики 4
void cmd_dhtPerception() {
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    int order = sCmd.nextInt();

    createWidget(widget_name, page_name, order, "any-data", "dhtPerception");
}

// dhtComfort Степень#комфорта: Датчики 3
void cmd_dhtComfort() {
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    int order = sCmd.nextInt();

    createWidget(widget_name, page_name, order, "anydata", "dhtComfort");
}

// analog adc 0 Аналоговый#вход,#% Датчики any-data 1 1023 1 100 1
void cmd_analog() {
    String name = sCmd.next();
    String pin = sCmd.next();
    String descr = sCmd.next();
    String page = sCmd.next();
    String type = sCmd.next();

    int in_min = sCmd.nextInt();
    int in_max = sCmd.nextInt();
    int out_min = sCmd.nextInt();
    int out_max = sCmd.nextInt();

    int order = sCmd.nextInt();

    Sensors::add(name, pin)->setMap(in_min, in_max, out_min, out_max);

    createWidget(descr, page, order, type, name);
}

// bmp280T temp1 0x76 Температура#bmp280 Датчики any-data 1
void cmd_bmp280T() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    int order = sCmd.nextInt();
    BMP280Sensor::bmp280T_value_name = value_name;

    BMP280Sensor::bmp.begin(hexStringToUint8(address));
    BMP280Sensor::bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    createWidget(widget_name, page_name, order, type, value_name);
}

//bmp280P press1 0x76 Давление#bmp280 Датчики any-data 2
void cmd_bmp280P() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    int order = sCmd.nextInt();
    BMP280Sensor::bmp280P_value_name = value_name;

    BMP280Sensor::bmp.begin(hexStringToUint8(address));
    BMP280Sensor::bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    createWidget(widget_name, page_name, order, type, value_name);
}

//=========================================================================================================================================
//=============================================Модуль сенсоров bme280======================================================================
//bme280T temp1 0x76 Температура#bmp280 Датчики any-data 1
void cmd_bme280T() {
    String name = sCmd.next();
    String address = sCmd.next();
    String descr = sCmd.next();
    String page = sCmd.next();
    String type = sCmd.next();
    int order = sCmd.nextInt();

    createWidget(descr, page, order, type, name);
}

//bme280P pres1 0x76 Давление#bmp280 Датчики any-data 1
void cmd_bme280P() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    int order = sCmd.nextInt();

    createWidget(widget_name, page_name, order, type, value_name);
}

//bme280H hum1 0x76 Влажность#bmp280 Датчики any-data 1
void cmd_bme280H() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    int order = sCmd.nextInt();

    createWidget(widget_name, page_name, order, type, value_name);
}

//bme280A altit1 0x76 Высота#bmp280 Датчики any-data 1
void cmd_bme280A() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    int order = sCmd.nextInt();

    createWidget(widget_name, page_name, order, type, value_name);
}

void cmd_http() {
    String host = sCmd.next();
    String param = sCmd.next();
    param.replace("_", "%20");
    String url = "http://" + host + "/cmd?command=" + param;

    WebClient::get(url);
}

void cmd_firmwareUpdate() {
    perform_upgrade();
}

void cmd_firmwareVersion() {
    String widget = sCmd.next();
    String page = sCmd.next();
    int order = sCmd.nextInt();

    liveData.write("firmver", FIRMWARE_VERSION);
    createWidget(widget, page, order, "anydata", "firmver");
}

void cmd_reboot() {
    perform_system_restart();
}

void cmd_timerStart() {
    String name = sCmd.next();
    String period = sCmd.next();
    String type = sCmd.next();

    unsigned long value = 0;
    if (period.indexOf("digit") != -1) {
        value = liveData.readInt(period);
    } else if (type == "sec") {
        value = period.toInt();
    } else if (type == "min") {
        value = period.toInt() * ONE_MINUTE_s;
    } else if (type == "hours") {
        value = period.toInt() * ONE_HOUR_s;
    }

    Timers::add(name, value);

    liveData.writeInt("timer" + name, 1);
}

void cmd_push() {
    String title = sCmd.next();
    String body = sCmd.next();
    title.replace("#", " ");
    body.replace("#", " ");

    pushControl(title, body);
}

void cmd_timerStop() {
    String name = sCmd.next();
    Timers::erase(name);
}

void fileExecute(const String filename) {
    String buf;
    if (readFile(filename.c_str(), buf)) {
        stringExecute(buf);
    }
}

void stringExecute(String str) {
    str += "\r\n";
    str.replace("\r\n", "\n");
    str.replace("\r", "\n");
    while (!str.isEmpty()) {
        String buf = selectToMarker(str, "\n");
        // Comments
        if (!buf.startsWith("//")) {
            sCmd.readStr(buf);
        }
        str = deleteBeforeDelimiter(str, "\n");
    }
}

void addOrder(const String &str) {
    _orders.push(str);
}

void loop_cmd() {
    if (_orders.available()) {
        String buf;
        _orders.pop(buf);
        pm.info("execute: " + buf);
        sCmd.readStr(buf);
    }
}