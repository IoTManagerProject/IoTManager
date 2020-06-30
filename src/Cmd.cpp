#include "Global.h"

#include "Module/Terminal.h"
#include "Objects/Servos.h"
#include "Objects/Buttons.h"
#include "WebClient.h"

static const char *MODULE = "Cmd";

Terminal *term = nullptr;

#ifdef ESP8266
SoftwareSerial *mySerial = nullptr;
#else
HardwareSerial *mySerial = nullptr;
#endif

void getData();

void cmd_init() {
    myButtons.setOnChangeState([](Button_t *btn, uint8_t num) {
        fireEvent("switch", String(num, DEC));
        jsonWriteInt(configLiveJson, "switch" + String(num, DEC), btn->state);
    });

    sCmd.addCommand("button", button);
    sCmd.addCommand("buttonSet", buttonSet);
    sCmd.addCommand("buttonChange", buttonChange);

    sCmd.addCommand("pinSet", pinSet);
    sCmd.addCommand("pinChange", pinChange);

    sCmd.addCommand("pwm", pwm);
    sCmd.addCommand("pwmSet", pwmSet);

    sCmd.addCommand("switch", switch_);

#ifdef ANALOG_ENABLED
    sCmd.addCommand("analog", analog);
#endif
#ifdef LEVEL_ENABLED
    sCmd.addCommand("levelPr", levelPr);
    sCmd.addCommand("ultrasonicCm", ultrasonicCm);
#endif
#ifdef DALLAS_ENABLED
    sCmd.addCommand("dallas", dallas);
#endif
#ifdef DHT_ENABLED
    sCmd.addCommand("dhtT", dhtT);
    sCmd.addCommand("dhtH", dhtH);
    sCmd.addCommand("dhtPerception", dhtP);
    sCmd.addCommand("dhtComfort", dhtC);
    sCmd.addCommand("dhtDewpoint", dhtD);
#endif

#ifdef BMP_ENABLED
    sCmd.addCommand("bmp280T", bmp280T);
    sCmd.addCommand("bmp280P", bmp280P);
#endif

#ifdef BME_ENABLED
    sCmd.addCommand("bme280T", bme280T);
    sCmd.addCommand("bme280P", bme280P);
    sCmd.addCommand("bme280H", bme280H);
    sCmd.addCommand("bme280A", bme280A);
#endif

#ifdef STEPPER_ENABLED
    sCmd.addCommand("stepper", stepper);
    sCmd.addCommand("stepperSet", stepperSet);
#endif

#ifdef SERVO_ENABLED
    sCmd.addCommand("servo", servo_);
    sCmd.addCommand("servoSet", servoSet);
#endif

#ifdef SERIAL_ENABLED
    sCmd.addCommand("serialBegin", serialBegin);
    sCmd.addCommand("serialWrite", serialWrite);
    sCmd.addCommand("getData", getData);
#endif

#ifdef LOGGING_ENABLED
    sCmd.addCommand("logging", logging);
#endif

    sCmd.addCommand("inputDigit", inputDigit);
    sCmd.addCommand("digitSet", digitSet);

    sCmd.addCommand("inputTime", inputTime);
    sCmd.addCommand("timeSet", timeSet);

    sCmd.addCommand("timerStart", timerStart_);
    sCmd.addCommand("timerStop", timerStop_);

    sCmd.addCommand("text", text);
    sCmd.addCommand("textSet", textSet);

    sCmd.addCommand("mqtt", mqttCommand);
    sCmd.addCommand("http", httpCommand);

#ifdef PUSH_ENABLED
    sCmd.addCommand("push", pushControl);
#endif

    sCmd.addCommand("firmwareUpdate", firmwareUpdate);
    sCmd.addCommand("firmwareVersion", firmwareVersion);

    handle_time_init();
}

void button() {
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
        enableScenario(param);
    } else if (assign.indexOf("line") != -1) {
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
            Scenario::enable(number_int, param);
            str = deleteBeforeDelimiter(str, ",");
        }
    }

    createWidget(description, page, order, "toggle", "button" + name);
}

void buttonSet() {
    String name = sCmd.next();
    String param = sCmd.next();

    Button_t *btn = myButtons.get(name);

    if (isDigitStr(btn->assign)) {
        btn->set(param.toInt());
    } else if (btn->assign == "scen") {
        enableScenario(param.toInt());
    } else if (btn->assign.startsWith("line")) {
        String str = btn->assign;
        while (str.length() != 0) {
            String tmp = selectToMarker(str, ",");            //line1,
            String number = deleteBeforeDelimiter(tmp, "e");  //1,
            number.replace(",", "");
            Serial.println(number);
            Scenario::enable(number.toInt(), param.toInt());
            str = deleteBeforeDelimiter(str, ",");
        }
    }

    fireEvent("button", name);

    jsonWriteStr(configLiveJson, "button" + name, param);

    MqttClient::publishStatus("button" + name, param);
}

void buttonChange() {
    String name = sCmd.next();

    if (!isDigitStr(name)) {
        pm.error("wrong button " + name);
        return;
    }

    Button_t *btn = myButtons.get(name);

    String stateStr = btn->toggle() ? "1" : "0";

    MqttClient::publishStatus("button" + name, stateStr);
}

void pinSet() {
    String pin_number = sCmd.next();
    String pin_state = sCmd.next();
    pinMode(pin_number.toInt(), OUTPUT);
    digitalWrite(pin_number.toInt(), pin_state.toInt());
}

void pinChange() {
    String pin_number = sCmd.next();
    pinMode(pin_number.toInt(), OUTPUT);
    digitalWrite(pin_number.toInt(), !digitalRead(pin_number.toInt()));
}
//==================================================================================================================
//==========================================Модуль управления ШИМ===================================================
void pwm() {
    //static boolean flag = true;
    String pwm_number = sCmd.next();
    String pwm_pin = sCmd.next();
    String widget_name = sCmd.next();
    widget_name.replace("#", " ");
    String page_name = sCmd.next();
    String start_state = sCmd.next();
    String page_number = sCmd.next();

    uint8_t pwm_pin_int = pwm_pin.toInt();
    jsonWriteStr(configOptionJson, "pwm_pin" + pwm_number, pwm_pin);
    pinMode(pwm_pin_int, INPUT);
    analogWrite(pwm_pin_int, start_state.toInt());

    jsonWriteStr(configLiveJson, "pwm" + pwm_number, start_state);
    createWidget(widget_name, page_name, page_number, "range", "pwm" + pwm_number);
}

void pwmSet() {
    String pwm_number = sCmd.next();
    String pwm_state = sCmd.next();
    int pwm_state_int = pwm_state.toInt();

    int pin = jsonReadInt(configOptionJson, "pwm_pin" + pwm_number);
    analogWrite(pin, pwm_state_int);

    fireEvent("pwm", pwm_number);

    jsonWriteStr(configLiveJson, "pwm" + pwm_number, pwm_state);

    MqttClient::publishStatus("pwm" + pwm_number, pwm_state);
}

void switch_() {
    String name = sCmd.next();
    String assign = sCmd.next();
    String debounse = sCmd.next();

    myButtons.addSwitch(name, assign, debounse);
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
void inputDigit() {
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

void digitSet() {
    String number = sCmd.next();
    String value = sCmd.next();

    jsonWriteStr(configLiveJson, "digit" + number, value);
    MqttClient::publishStatus("digit" + number, value);
}

//=====================================================================================================================================
//=========================================Добавление окна ввода времени===============================================================
void inputTime() {
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

void timeSet() {
    String number = sCmd.next();
    String value = sCmd.next();

    jsonWriteStr(configLiveJson, "time" + number, value);
    MqttClient::publishStatus("time" + number, value);
}

void handle_time_init() {
    ts.add(
        TIME, 1000, [&](void *) {
            jsonWriteStr(configLiveJson, "time", timeNow->getTime());
            jsonWriteStr(configLiveJson, "timenow", timeNow->getTimeJson());
            fireEvent("timenow", "");
        },
        nullptr, true);
}

//=====================================================================================================================================
//=========================================Добавление текстового виджета============================================================
void text() {
    String number = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String page_number = sCmd.next();

    createWidget(widget_name, page_name, page_number, "anydata", "text" + number);
}

void textSet() {
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
#ifdef STEPPER_ENABLED
//stepper 1 12 13
void stepper() {
    String stepper_number = sCmd.next();
    String pin_step = sCmd.next();
    String pin_dir = sCmd.next();

    jsonWriteStr(configOptionJson, "stepper" + stepper_number, pin_step + " " + pin_dir);
    pinMode(pin_step.toInt(), OUTPUT);
    pinMode(pin_dir.toInt(), OUTPUT);
}

//stepperSet 1 100 5
void stepperSet() {
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
#endif
//====================================================================================================================================================
//=================================================================Сервоприводы=======================================================================
#ifdef SERVO_ENABLED
//servo 1 13 50 Мой#сервопривод Сервоприводы 0 100 0 180 2
void servo_() {
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
#ifdef ESP32
    myServo1.attach(servo_pin.toInt(), 500, 2400);
    myServo1.write(start_state_int);
#endif

    jsonWriteInt(configOptionJson, "s_min_val" + number, min_value);
    jsonWriteInt(configOptionJson, "s_max_val" + number, max_value);
    jsonWriteInt(configOptionJson, "s_min_deg" + number, min_deg);
    jsonWriteInt(configOptionJson, "s_max_deg" + number, max_deg);

    jsonWriteInt(configLiveJson, "servo" + number, value);

    createWidget(widget, page, pageNumber, "range", "servo" + number, "min", String(min_value), "max", String(max_value), "k", "1");
}

void servoSet() {
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
#endif

#ifdef SERIAL_ENABLED
void serialBegin() {
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

void getData() {
    String param = sCmd.next();
    String res = param.length() ? jsonReadStr(configLiveJson, param) : configLiveJson;
    if (term) {
        term->println(res.c_str());
    }
}

void serialWrite() {
    String payload = sCmd.next();
    if (term) {
        term->println(payload.c_str());
    }
}
#endif
//====================================================================================================================================================
//=================================================Глобальные команды удаленного управления===========================================================

void mqttCommand() {
    String topic = sCmd.next();
    String data = sCmd.next();

    MqttClient::publishOrder(topic, data);
}

void httpCommand() {
    String host = sCmd.next();
    String param = sCmd.next();
    param.replace("_", "%20");

    String url = "http://" + host + "/cmd?command=" + param;
    WebClient::get(url);
}

void firmwareUpdate() {
    updateFlag = true;
}

void firmwareVersion() {
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

void fileExecute(const String &filename) {
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
