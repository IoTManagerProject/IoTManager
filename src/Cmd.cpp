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
#include "Timers.h"
#include "Objects/Switches.h"
#include "Objects/Buttons.h"
#include "Objects/PwmItems.h"
#include "Objects/ServoItems.h"
#include "Objects/OneWireBus.h"
#include "Objects/Terminal.h"
#include "Objects/Telnet.h"
#include "Utils/PrintMessage.h"

static const char *MODULE = "Cmd";

StringQueue _orders;

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
        Events::fire(name);
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

    sCmd.addCommand("push", pushControl);

    sCmd.addCommand("firmwareUpdate", cmd_firmwareUpdate);
    sCmd.addCommand("firmwareVersion", cmd_firmwareVersion);

    sCmd.addCommand("get", cmd_get);

    sCmd.addCommand("reboot", cmd_reboot);
}

//===============================================Логирование============================================================
//logging temp1 1 10 Температура Датчики 2
void cmd_logging() {
    String name = sCmd.next();
    String period = sCmd.next();
    String count = sCmd.next();
    String descr = sCmd.next();
    String page = sCmd.next();
    String order = sCmd.next();

    Logger::add(name, period.toInt() * 1000 * 60, count.toInt());
    // /prefix/3234045-1589487/value_name_ch
    createChart(descr, page, order, "chart", name + "_ch", count);
}

void cmd_button() {
    String name = sCmd.next();
    String assign = sCmd.next();
    String descr = sCmd.next();
    String page = sCmd.next();
    String state = sCmd.next();
    String order = sCmd.next();
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
    String assign = btn->assigned();

    if (isDigitStr(assign)) {
        btn->setState(state.toInt());
    } else if (assign == "scen") {
        config.general()->enableScenario(state.toInt());
    } else if (assign.startsWith("line")) {
        while (assign.length() != 0) {
            String tmp = selectToMarker(assign, ",");         //line1,
            String number = deleteBeforeDelimiter(tmp, "e");  //1,
            number.replace(",", "");
            Scenario::enableBlock(number.toInt(), state.toInt());
            assign = deleteBeforeDelimiter(assign, ",");
        }
    }

    Events::fire("button", name);

    liveData.write("button" + name, state);

    MqttClient::publishStatus("button" + name, state);
}

void cmd_buttonChange() {
    String name = sCmd.next();

    if (!isDigitStr(name)) {
        pm.error("wrong button " + name);
        return;
    }

    ButtonItem *btn = myButtons.get(name);
    btn->toggleState();

    liveData.write("button" + name, String(btn->getState(), DEC));
    MqttClient::publishStatus("button" + name, String(btn->getState(), DEC));
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

    myPwm.add(name, assign, value);

    liveData.writeInt("pwm" + name, value.toInt());

    createWidget(description, page, order, "range", "pwm" + name);
}

void cmd_pwmSet() {
    String name = sCmd.next();
    String value = sCmd.next();

    myPwm.get(name)->setState(value.toInt());

    Events::fire("pwm", name);

    liveData.writeInt("pwm" + name, value.toInt());

    MqttClient::publishStatus("pwm" + name, value);
}

void cmd_switch() {
    String name = sCmd.next();
    String assign = sCmd.next();
    String debounce = sCmd.next();

    Switch *item = mySwitches.add(name, assign, debounce);

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
    String page_number = sCmd.next();

    liveData.write("digit" + number, start_state);
    createWidget(widget_name, page_name, page_number, "inputNum", "digit" + number);
}

void cmd_digitSet() {
    String number = sCmd.next();
    String value = sCmd.next();

    liveData.write("digit" + number, value);
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

    liveData.write("time" + number, start_state);
    createWidget(widget_name, page_name, page_number, "inputTime", "time" + number);
}

void cmd_timeSet() {
    String number = sCmd.next();
    String value = sCmd.next();

    liveData.write("time" + number, value);
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

    liveData.write("text" + number, text);
    MqttClient::publishStatus("text" + number, text);
}

// stepper 1 12 13
void cmd_stepper() {
    String stepper_number = sCmd.next();
    String pin_step = sCmd.next();
    String pin_dir = sCmd.next();

    liveData.write("stepper" + stepper_number, pin_step + " " + pin_dir);
    pinMode(pin_step.toInt(), OUTPUT);
    pinMode(pin_dir.toInt(), OUTPUT);
}

//stepperSet 1 100 5
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
    Events::fire("servo", name);
    liveData.writeInt("servo" + name, value);
    MqttClient::publishStatus("servo" + name, String(value, DEC));
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
            res = param.isEmpty() ? options.get() : options.read(param);
        } else if (obj.equalsIgnoreCase(TAG_RUNTIME)) {
            res = param.isEmpty() ? runtime.get() : runtime.read(param);
        } else if (obj.equalsIgnoreCase("state")) {
            res = param.isEmpty() ? liveData.get() : liveData.read(param);
        } else if (obj.equalsIgnoreCase("devices")) {
            Devices::asString(res, param.toInt());
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

void cmd_onewire() {
    uint8_t pin = (uint8_t)String(sCmd.next()).toInt();
    oneWireBus.set(pin);
    if (!oneWireBus.exists()) {
        pm.error("on create OneWire");
    }
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
    String page_number = sCmd.next();

    Ultrasonic::ultrasonicCm_value_name = measure_unit;

    options.write("trig", trig);
    options.write("echo", echo);
    pinMode(trig.toInt(), OUTPUT);
    pinMode(echo.toInt(), INPUT);

    createWidget(widget, page, page_number, type, measure_unit);

    Sensors::enable(0);
}

// dallas temp1 0x14 Температура Датчики anydata 1
// dallas temp2 0x15 Температура Датчики anydata 2
void cmd_dallas() {
    if (!oneWireBus.exists()) {
        pm.error("needs OneWire");
        return;
    }

    if (Dallas::dallasSensors.count() == 0) {
        Dallas::dallasTemperature = new DallasTemperature(oneWireBus.get());
        Dallas::dallasTemperature->begin();
        Dallas::dallasTemperature->setResolution(12);
    }

    String value = sCmd.next();

    // uint8_t *address = String(sCmd.next()).toInt();

    // dallasSensors.create(address, value);

    String widget = sCmd.next();
    String page = sCmd.next();
    String type = sCmd.next();
    String pageNumber = sCmd.next();

    // jsonWriteStr(configOptionJson, value + "_ds", String(address, DEC));

    Dallas::dallas_value_name += value + ";";
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

    Ultrasonic::levelPr_value_name = value_name;

    options.write("e_lev", empty_level);
    options.write("f_lev", full_level);
    options.write("trig", trig);
    options.write("echo", echo);
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
    DHTSensor::dhtH_value_name = value_name;
    if (sensor_type == "dht11") {
        DHTSensor::dht.setup(pin.toInt(), DHTesp::DHT11);
    }
    if (sensor_type == "dht22") {
        DHTSensor::dht.setup(pin.toInt(), DHTesp::DHT22);
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
    DHTSensor::dhtT_value_name = value_name;
    if (sensor_type == "dht11") {
        DHTSensor::dht.setup(pin.toInt(), DHTesp::DHT11);
    }
    if (sensor_type == "dht22") {
        DHTSensor::dht.setup(pin.toInt(), DHTesp::DHT22);
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
    String name = sCmd.next();
    String pin = sCmd.next();
    String descr = sCmd.next();
    String page = sCmd.next();
    String type = sCmd.next();

    String analog_start = sCmd.next();
    String analog_end = sCmd.next();
    String analog_start_out = sCmd.next();
    String analog_end_out = sCmd.next();

    String order = sCmd.next();

    // options.write(name + "_st", analog_start);
    // options.write(name + "_end", analog_end);
    // options.write(name + "_st_out", analog_start_out);
    // options.write(name + "_end_out", analog_end_out);

    AnalogSensor::add(name, pin, analog_start, analog_end, analog_start_out, analog_end_out);

    createWidget(descr, page, order, type, name);
}

// bmp280T temp1 0x76 Температура#bmp280 Датчики any-data 1
void cmd_bmp280T() {
    String value_name = sCmd.next();
    String address = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String type = sCmd.next();
    String page_number = sCmd.next();
    BMP280Sensor::bmp280T_value_name = value_name;

    BMP280Sensor::bmp.begin(hexStringToUint8(address));
    BMP280Sensor::bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
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
    BMP280Sensor::bmp280P_value_name = value_name;

    BMP280Sensor::bmp.begin(hexStringToUint8(address));
    BMP280Sensor::bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
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
    BME280Sensor::bme280T_value_name = value_name;

    BME280Sensor::bme.begin(hexStringToUint8(address));

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
    BME280Sensor::bme280P_value_name = value_name;

    BME280Sensor::bme.begin(hexStringToUint8(address));

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
    BME280Sensor::bme280H_value_name = value_name;

    BME280Sensor::bme.begin(hexStringToUint8(address));

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
    BME280Sensor::bme280A_value_name = value_name;

    BME280Sensor::bme.begin(hexStringToUint8(address));

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
    perform_upgrade_flag = true;
}

void cmd_firmwareVersion() {
    String widget = sCmd.next();
    String page = sCmd.next();
    String pageNumber = sCmd.next();

    liveData.write("firmver", FIRMWARE_VERSION);
    createWidget(widget, page, pageNumber, "anydata", "firmver");
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
        sCmd.readStr(buf);
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