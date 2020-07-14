#include "Cmd.h"

#include "Collection/Buttons.h"
#include "Collection/Devices.h"
#include "Collection/Logger.h"
#include "Collection/Sensors.h"
#include "Collection/Timers.h"
#include "Collection/Widgets.h"

#include "Sensors/AnalogSensor.h"
#include "Sensors/OneWireBus.h"

#include "Objects/Switches.h"
#include "Objects/PwmItems.h"
#include "Objects/ServoItems.h"
#include "Objects/Terminal.h"
#include "Objects/Telnet.h"

#include "PrintMessage.h"

#include "StringConsts.h"
#include "MqttClient.h"
#include "WebClient.h"
#include "Scenario.h"

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

unsigned long parsePeriod(const String &str, unsigned long default_multiplier) {
    unsigned long res = 0;
    if (str.indexOf("digit") != -1) {
        res = liveData.readInt(str);
    } else {
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
    }
    return res;
}

void cmd_init() {
    pm.info(TAG_INIT);

    mySwitches.setOnChangeState([](Switch *obj) {
        String name = String("switch") + obj->getName();
        pm.info(name);
        Scenario::fire(name);
        liveData.write(name, obj->getValue(), VT_INT);
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

//logging temp1 1 10 Температура Датчики 2
void cmd_logging() {
    String name = sCmd.next();
    String period = sCmd.next();
    String maxCount = sCmd.next();
    String descr = sCmd.next();
    String page = sCmd.next();
    String order = sCmd.next();

    Logger::add(name.c_str(), parsePeriod(period), maxCount.toInt());
    // /prefix/3234045-1589487/value_name_ch
    Widgets::createChart(descr, page, order, "chart", name + "_ch", maxCount);
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
    String order = sCmd.next();

    String style = sCmd.next();

    String objName = "pwm" + name;

    myPwm.add(objName, assign, value);
    liveData.write(objName, value, VT_INT);
    Widgets::createWidget(description, page, order, "range", objName);
}

void cmd_pwmSet() {
    String name = sCmd.next();
    String value = sCmd.next();

    myPwm.get(name)->setValue(value);

    String objName = "pwm" + name;

    Scenario::fire(objName);
    liveData.write(objName, value, VT_INT);
    MqttClient::publishStatus(objName, value, VT_INT);
}

void cmd_switch() {
    String name = sCmd.next();
    String assign = sCmd.next();
    String debounce = sCmd.next();

    Switch *item = mySwitches.add(name, assign);
    if (!item) {
        pm.error("bad command");
        return;
    }
    item->setDebounce(debounce.toInt());

    String objName = "switch" + name;

    liveData.write(objName, item->getValue(), VT_INT);
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
    String name = sCmd.next();
    String number = name.substring(5);
    String widget_name = sCmd.next();
    widget_name.replace("#", " ");
    String page_name = sCmd.next();
    page_name.replace("#", " ");
    String start_state = sCmd.next();
    String order = sCmd.next();

    liveData.write("digit" + number, start_state, VT_STRING);
    Widgets::createWidget(widget_name, page_name, order, "inputNum", "digit" + number);
}

void cmd_digitSet() {
    String name = sCmd.next();
    String value = sCmd.next();

    String objName = "digit" + name;
    liveData.write(objName, value, VT_STRING);
    MqttClient::publishStatus(objName, value, VT_STRING);
}

void cmd_inputTime() {
    String name = sCmd.next();
    String number = name.substring(4);
    String widget_name = sCmd.next();
    widget_name.replace("#", " ");
    String page_name = sCmd.next();
    page_name.replace("#", " ");
    String state = sCmd.next();
    String order = sCmd.next();

    liveData.write("time" + number, state, VT_STRING);
    Widgets::createWidget(widget_name, page_name, order, "inputTime", "time" + number);
}

void cmd_timeSet() {
    String name = sCmd.next();
    String value = sCmd.next();

    String objName = "time" + name;
    liveData.write(objName, value, VT_STRING);
    MqttClient::publishStatus(objName, value, VT_STRING);
}

void cmd_text() {
    String name = sCmd.next();
    String descr = sCmd.next();
    String page = sCmd.next();
    String order = sCmd.next();

    String objName = "text" + name;
    Widgets::createWidget(descr, page, order, "anydata", objName);
}

void cmd_textSet() {
    String name = sCmd.next();
    String value = sCmd.next();
    value.replace("_", " ");

    if (value.indexOf("-time") >= 0) {
        value.replace("-time", "");
        value.replace("#", " ");
        value = value + " " + now.getDateTimeDotFormated();
    }

    String objName = "text" + name;
    liveData.write(objName, value, VT_STRING);
    MqttClient::publishStatus(objName, value, VT_STRING);
}

void cmd_stepper() {
    String name = sCmd.next();
    String pin_step = sCmd.next();
    String pin_dir = sCmd.next();

    // String objName = "stepper" + name;
    // liveData.write(objName, pin_step + " " + pin_dir);

    // pinMode(pin_step.toInt(), OUTPUT);
    // pinMode(pin_dir.toInt(), OUTPUT);
}

void cmd_stepperSet() {
    String stepper_number = sCmd.next();
    String steps = sCmd.next();
    // options.write("steps" + stepper_number, steps);
    // String stepper_speed = sCmd.next();
    // String pin_step = selectToMarker(options.read("stepper" + stepper_number), " ");
    // String pin_dir = deleteBeforeDelimiter(options.read("stepper" + stepper_number), " ");
    // Serial.println(pin_step);
    // Serial.println(pin_dir);
    // if (steps.toInt() > 0) digitalWrite(pin_dir.toInt(), HIGH);
    // if (steps.toInt() < 0) digitalWrite(pin_dir.toInt(), LOW);
    // if (stepper_number == "1") {
    //     ts.add(
    //         STEPPER1, stepper_speed.toInt(), [&](void *) {
    //             int steps_int = abs(options.readInt("steps1") * 2);
    //             static int count;
    //             count++;
    //             String pin_step = selectToMarker(options.read("stepper1"), " ");
    //             digitalWrite(pin_step.toInt(), !digitalRead(pin_step.toInt()));
    //             yield();
    //             if (count > steps_int) {
    //                 digitalWrite(pin_step.toInt(), LOW);
    //                 ts.remove(STEPPER1);
    //                 count = 0;
    //             }
    //         },
    //         nullptr, true);
    // }

    // if (stepper_number == "2") {
    //     ts.add(
    //         STEPPER2, stepper_speed.toInt(), [&](void *) {
    //             int steps_int = abs(options.readInt("steps2") * 2);
    //             static int count;
    //             count++;
    //             String pin_step = selectToMarker(options.read("stepper2"), " ");
    //             digitalWrite(pin_step.toInt(), !digitalRead(pin_step.toInt()));
    //             yield();
    //             if (count > steps_int) {
    //                 digitalWrite(pin_step.toInt(), LOW);
    //                 ts.remove(STEPPER2);
    //                 count = 0;
    //             }
    //         },
    //         nullptr, true);
    // }
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

    String order = sCmd.next();

    myServo.add(name, pin, value, min_value, max_value, min_deg, max_deg);

    // options.write("servo_pin" + name, pin);
    // value = map(value, min_value, max_value, min_deg, max_deg);
    // servo->write(value);
    // options.writeInt("s_min_val" + name, min_value);
    // options.writeInt("s_max_val" + name, max_value);
    // options.writeInt("s_min_deg" + name, min_deg);
    // options.writeInt("s_max_deg" + name, max_deg);
    // liveData.writeInt("servo" + name, value);

    Widgets::createWidget(descr, page, order, "range", "servo" + name, "min", String(min_value), "max", String(max_value), "k", "1");
}

void cmd_servoSet() {
    String name = sCmd.next();
    String value = sCmd.next();

    BaseServo *servo = myServo.get(name);
    if (servo) {
        servo->setValue(value);
    }
    String objName = "servo" + name;

    Scenario::fire(objName);
    liveData.write(objName, value, VT_INT);
    MqttClient::publishStatus(objName, value, VT_INT);
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
    String order = sCmd.next();

    Ultrasonic::ultrasonicCm_value_name = measure_unit;

    // options.write("trig", trig);
    // options.write("echo", echo);
    // pinMode(trig.toInt(), OUTPUT);
    // pinMode(echo.toInt(), INPUT);

    Widgets::createWidget(widget, page, order, type, measure_unit);
}

void cmd_oneWire() {
    String assign = sCmd.next();
    onewire.attach(assign.toInt());
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
    String descr = sCmd.next();
    String page = sCmd.next();
    String widget = sCmd.next();
    String order = sCmd.next();

    auto *item = Sensors::add(SENSOR_DALLAS, name, address);

    Widgets::createWidget(descr, page, order, widget, name);
}

//levelPr p 14 12 Вода#в#баке,#% Датчики fillgauge 125 20 1
void cmd_levelPr() {
    String name = sCmd.next();
    String trig = sCmd.next();
    String echo = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String empty_level = sCmd.next();
    String full_level = sCmd.next();
    String order = sCmd.next();

    Ultrasonic::levelPr_value_name = name;

    // options.write("e_lev", empty_level);
    // options.write("f_lev", full_level);
    // options.write("trig", trig);
    // options.write("echo", echo);
    // pinMode(trig.toInt(), OUTPUT);
    // pinMode(echo.toInt(), INPUT);
    Widgets::createWidget(widget_name, page_name, order, type, name);
}

//dhtH h 2 dht11 Влажность#DHT,#t°C Датчики any-data 1
void cmd_dhtH() {
    String name = sCmd.next();
    String pin = sCmd.next();
    String sensor_type = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String order = sCmd.next();
    DHTSensor::dhtH_value_name = name;
    if (sensor_type == "dht11") {
        DHTSensor::dht.setup(pin.toInt(), DHTesp::DHT11);
    }
    if (sensor_type == "dht22") {
        DHTSensor::dht.setup(pin.toInt(), DHTesp::DHT22);
    }
    Widgets::createWidget(widget_name, page_name, order, type, name);
}

// dhtT t 2 dht11 Температура#DHT,#t°C Датчики any-data 1
void cmd_dhtT() {
    String name = sCmd.next();
    String pin = sCmd.next();
    String sensor_type = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String order = sCmd.next();
    DHTSensor::dhtT_value_name = name;
    if (sensor_type == "dht11") {
        DHTSensor::dht.setup(pin.toInt(), DHTesp::DHT11);
    }
    if (sensor_type == "dht22") {
        DHTSensor::dht.setup(pin.toInt(), DHTesp::DHT22);
    }

    Widgets::createWidget(widget_name, page_name, order, type, name);
}

//dhtDewpoint Точка#росы: Датчики 5
void cmd_dhtDewpoint() {
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String order = sCmd.next();

    Widgets::createWidget(widget_name, page_name, order, "anydata", "dhtDewpoint");
}

// dhtPerception Восприятие: Датчики 4
void cmd_dhtPerception() {
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String order = sCmd.next();

    Widgets::createWidget(widget_name, page_name, order, "any-data", "dhtPerception");
}

// dhtComfort Степень#комфорта: Датчики 3
void cmd_dhtComfort() {
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String order = sCmd.next();

    Widgets::createWidget(widget_name, page_name, order, "anydata", "dhtComfort");
}

// bmp280T temp1 0x76 Температура#bmp280 Датчики any-data 1
void cmd_bmp280T() {
    String name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String order = sCmd.next();
    BMP280Sensor::bmp280T_value_name = name;

    BMP280Sensor::bmp.begin(hexStringToUint8(address));
    BMP280Sensor::bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    Widgets::createWidget(widget_name, page_name, order, type, name);
}

//bmp280P press1 0x76 Давление#bmp280 Датчики any-data 2
void cmd_bmp280P() {
    String name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String order = sCmd.next();
    BMP280Sensor::bmp280P_value_name = name;

    BMP280Sensor::bmp.begin(hexStringToUint8(address));
    BMP280Sensor::bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

    Widgets::createWidget(widget_name, page_name, order, type, name);
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
    String order = sCmd.next();

    Widgets::createWidget(descr, page, order, type, name);
}

//bme280P pres1 0x76 Давление#bmp280 Датчики any-data 1
void cmd_bme280P() {
    String name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String order = sCmd.next();

    Widgets::createWidget(widget_name, page_name, order, type, name);
}

//bme280H hum1 0x76 Влажность#bmp280 Датчики any-data 1
void cmd_bme280H() {
    String name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String order = sCmd.next();

    Widgets::createWidget(widget_name, page_name, order, type, name);
}

//bme280A altit1 0x76 Высота#bmp280 Датчики any-data 1
void cmd_bme280A() {
    String name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String order = sCmd.next();

    Widgets::createWidget(widget_name, page_name, order, type, name);
}

void cmd_firmwareUpdate() {
    perform_upgrade();
}

void cmd_firmwareVersion() {
    String widget = sCmd.next();
    String page = sCmd.next();
    String order = sCmd.next();

    liveData.write("firmver", FIRMWARE_VERSION, VT_STRING);
    Widgets::createWidget(widget, page, order, "anydata", "firmver");
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