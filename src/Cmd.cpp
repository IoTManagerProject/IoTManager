#include "Global.h"

boolean but[NUM_BUTTONS];
Bounce *buttons = new Bounce[NUM_BUTTONS];

Servo myServo1;
Servo myServo2;
SoftwareSerial *mySerial = nullptr;

void CMD_init() {
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

    sCmd.addCommand("mqtt", mqttOrderSend);
    sCmd.addCommand("http", httpOrderSend);

#ifdef PUSH_ENABLED
    sCmd.addCommand("push", pushControl);
#endif

    sCmd.addCommand("firmwareUpdate", firmwareUpdate);
    sCmd.addCommand("firmwareVersion", firmwareVersion);

    handle_time_init();
}

//==========================================================================================================
//==========================================Модуль кнопок===================================================
void button() {
    String button_number = sCmd.next();
    String button_param = sCmd.next();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String start_state = sCmd.next();
    String page_number = sCmd.next();

    jsonWriteStr(configOptionJson, "button_param" + button_number, button_param);
    jsonWriteStr(configLiveJson, "button" + button_number, start_state);

    if (isDigitStr(button_param)) {
        pinMode(button_param.toInt(), OUTPUT);
        digitalWrite(button_param.toInt(), start_state.toInt());
    }

    if (button_param == "scen") {
        jsonWriteStr(configSetupJson, "scen", start_state);
        Scenario_init();
        saveConfig();
    }

    if (button_param.indexOf("line") != -1) {
        String str = button_param;
        while (str.length() != 0) {
            if (str == "") return;
            String tmp = selectToMarker(str, ",");            //line1,
            String number = deleteBeforeDelimiter(tmp, "e");  //1,
            number.replace(",", "");
            Serial.println(number);
            int number_int = number.toInt();
            scenario_line_status[number_int] = start_state.toInt();
            str = deleteBeforeDelimiter(str, ",");
        }
    }
    createWidget(widget_name, page_name, page_number, "widgets/widget.toggle.json", "button" + button_number);
}

void buttonSet() {
    String button_number = sCmd.next();
    String button_state = sCmd.next();
    String button_param = jsonReadStr(configOptionJson, "button_param" + button_number);

    if (button_param != "na" || button_param != "scen" || button_param.indexOf("line") != -1) {
        digitalWrite(button_param.toInt(), button_state.toInt());
    }

    if (button_param == "scen") {
        jsonWriteStr(configSetupJson, "scen", button_state);
        Scenario_init();
        saveConfig();
    }

    if (button_param.indexOf("line") != -1) {
        String str = button_param;
        while (str.length() != 0) {
            if (str == "") return;
            String tmp = selectToMarker(str, ",");            //line1,
            String number = deleteBeforeDelimiter(tmp, "e");  //1,
            number.replace(",", "");
            Serial.println(number);
            int number_int = number.toInt();
            scenario_line_status[number_int] = button_state.toInt();
            str = deleteBeforeDelimiter(str, ",");
        }
    }

    eventGen("button", button_number);

    jsonWriteStr(configLiveJson, "button" + button_number, button_state);

    publishStatus("button" + button_number, button_state);
}

void buttonChange() {
    String button_number = sCmd.next();
    String current_state = jsonReadStr(configLiveJson, "button" + button_number);

    if (current_state == "1") {
        current_state = "0";
    } else if (current_state == "0") {
        current_state = "1";
    }
    order_loop += "buttonSet " + button_number + " " + current_state + ",";
    jsonWriteStr(configLiveJson, "button" + button_number, current_state);
    publishStatus("button" + button_number, current_state);
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
    //analogWriteFreq(32000);
    jsonWriteStr(configLiveJson, "pwm" + pwm_number, start_state);

    createWidget(widget_name, page_name, page_number, "widgets/widget.range.json", "pwm" + pwm_number);
}

void pwmSet() {
    String pwm_number = sCmd.next();
    String pwm_state = sCmd.next();
    int pwm_state_int = pwm_state.toInt();

    int pin = jsonReadInt(configOptionJson, "pwm_pin" + pwm_number);
    analogWrite(pin, pwm_state_int);

    eventGen("pwm", pwm_number);

    jsonWriteStr(configLiveJson, "pwm" + pwm_number, pwm_state);

    publishStatus("pwm" + pwm_number, pwm_state);
}
//==================================================================================================================
//==========================================Модуль физической кнопки================================================
void switch_() {
    String switch_number = sCmd.next();
    String switch_pin = sCmd.next();
    String switch_delay = sCmd.next();

    buttons[switch_number.toInt()].attach(switch_pin.toInt());
    buttons[switch_number.toInt()].interval(switch_delay.toInt());
    but[switch_number.toInt()] = true;
}

void loopButton() {
    static uint8_t switch_number = 1;

    if (but[switch_number]) {
        buttons[switch_number].update();
        if (buttons[switch_number].fell()) {
            eventGen("switch", String(switch_number));

            jsonWriteStr(configLiveJson, "switch" + String(switch_number), "1");
        }
        if (buttons[switch_number].rose()) {
            eventGen("switch", String(switch_number));

            jsonWriteStr(configLiveJson, "switch" + String(switch_number), "0");
        }
    }
    switch_number++;
    if (switch_number == NUM_BUTTONS) {
        switch_number = 0;
    }
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
    createWidget(widget_name, page_name, page_number, "widgets/widget.inputNum.json", "digit" + number);
}

void digitSet() {
    String number = sCmd.next();
    String value = sCmd.next();
    jsonWriteStr(configLiveJson, "digit" + number, value);
    publishStatus("digit" + number, value);
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
    createWidget(widget_name, page_name, page_number, "widgets/widget.inputTime.json", "time" + number);
}

void timeSet() {
    String number = sCmd.next();
    String value = sCmd.next();
    jsonWriteStr(configLiveJson, "time" + number, value);
    publishStatus("time" + number, value);
}

void handle_time_init() {
    ts.add(
        TIME, 1000, [&](void *) {
            String tmp = getTime();
            jsonWriteStr(configLiveJson, "time", tmp);
            tmp.replace(":", "-");
            jsonWriteStr(configLiveJson, "timenow", tmp);
            eventGen("timenow", "");
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

    createWidget(widget_name, page_name, page_number, "widgets/widget.anyData.json", "text" + number);
}

void textSet() {
    String number = sCmd.next();
    String text = sCmd.next();
    text.replace("_", " ");

    if (text.indexOf("-time") >= 0) {
        text.replace("-time", "");
        text.replace("#", " ");
        String time = getTime();
        time.replace(":", ".");
        text = text + " " + getDateDigitalFormated() + " " + time;
    }

    jsonWriteStr(configLiveJson, "text" + number, text);
    publishStatus("text" + number, text);
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
    String servo_number = sCmd.next();
    String servo_pin = sCmd.next();
    String start_state = sCmd.next();
    int start_state_int = start_state.toInt();
    String widget_name = sCmd.next();
    String page_name = sCmd.next();

    String min_value = sCmd.next();
    String max_value = sCmd.next();

    String min_deg = sCmd.next();
    String max_deg = sCmd.next();

    String page_number = sCmd.next();

    jsonWriteStr(configOptionJson, "servo_pin" + servo_number, servo_pin);
    start_state_int = map(start_state_int, min_value.toInt(), max_value.toInt(), min_deg.toInt(), max_deg.toInt());

    if (servo_number == "1") {
#ifdef ESP8266
        myServo1.attach(servo_pin.toInt());
        myServo1.write(start_state_int);
#endif
#ifdef ESP32
        myServo1.attach(servo_pin.toInt(), 500, 2400);
        myServo1.write(start_state_int);
#endif
    }

    if (servo_number == "2") {
#ifdef ESP8266
        myServo2.attach(servo_pin.toInt());
        myServo2.write(start_state_int);
#endif
#ifdef ESP32
        myServo2.attach(servo_pin.toInt(), 500, 2400);
        myServo2.write(start_state_int);
#endif
    }

    jsonWriteStr(configOptionJson, "s_min_val" + servo_number, min_value);
    jsonWriteStr(configOptionJson, "s_max_val" + servo_number, max_value);
    jsonWriteStr(configOptionJson, "s_min_deg" + servo_number, min_deg);
    jsonWriteStr(configOptionJson, "s_max_deg" + servo_number, max_deg);

    jsonWriteStr(configLiveJson, "servo" + servo_number, start_state);

    createWidgetParam(widget_name, page_name, page_number, "widgets/widget.range.json", "servo" + servo_number, "min", min_value, "max", max_value, "k", "1");
}

void servoSet() {
    String servo_number = sCmd.next();
    String servo_state = sCmd.next();
    int servo_state_int = servo_state.toInt();

    //int pin = jsonReadInt(configOptionJson, "servo_pin" + servo_number);

    servo_state_int = map(servo_state_int,
                          jsonReadInt(configOptionJson, "s_min_val" + servo_number),
                          jsonReadInt(configOptionJson, "s_max_val" + servo_number),
                          jsonReadInt(configOptionJson, "s_min_deg" + servo_number),
                          jsonReadInt(configOptionJson, "s_max_deg" + servo_number));

    if (servo_number == "1") {
#ifdef ESP8266
        myServo1.write(servo_state_int);
#endif
#ifdef ESP32
        myServo1.write(servo_state_int);
#endif
    }

    if (servo_number == "2") {
#ifdef ESP8266
        myServo2.write(servo_state_int);
#endif
#ifdef ESP32
        myServo2.write(servo_state_int);
#endif
    }

    //Serial.println(servo_state_int);

    eventGen("servo", servo_number);

    jsonWriteStr(configLiveJson, "servo" + servo_number, servo_state);

    publishStatus("servo" + servo_number, servo_state);
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
    mySerial = new SoftwareSerial(rxPin.toInt(), txPin.toInt());
    mySerial->begin(s_speed.toInt());
}

void serialWrite() {
    String payload = sCmd.next();
    if (mySerial) {
        mySerial->println(payload);
    }
}
#endif
//====================================================================================================================================================
//=================================================Глобальные команды удаленного управления===========================================================

void mqttOrderSend() {
    String id = sCmd.next();
    String order = sCmd.next();

    String all_line = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + id + "/order";
    //Serial.print(all_line);
    //Serial.print("->");
    //Serial.println(order);
    mqtt.publish(all_line.c_str(), order.c_str(), false);
}

void httpOrderSend() {
    String ip = sCmd.next();
    String order = sCmd.next();
    order.replace("_", "%20");
    String url = "http://" + ip + "/cmd?command=" + order;
    getURL(url);
}

void firmwareUpdate() {
    upgrade = true;
}

void firmwareVersion() {
    String widget_name = sCmd.next();
    String page_name = sCmd.next();
    String page_number = sCmd.next();
    jsonWriteStr(configLiveJson, "firmver", FIRMWARE_VERSION);
    choose_widget_and_create(widget_name, page_name, page_number, "any-data", "firmver");
}

//==============================================================================================================================
//============================выполнение команд (в лупе) по очереди из строки order=============================================
void loopCmd() {
    if (order_loop != "") {
        String tmp = selectToMarker(order_loop, ",");  //выделяем первую команду rel 5 1,
        sCmd.readStr(tmp);                             //выполняем
        Serial.println("[ORDER] => " + order_loop);
        order_loop = deleteBeforeDelimiter(order_loop, ",");  //осекаем
    }
}

//=======================================================================================================================================
//=======================================================================================================================================
void txtExecution(String file) {
    String command_all = readFile(file, 2048) + "\r\n";

    command_all.replace("\r\n", "\n");
    command_all.replace("\r", "\n");

    while (command_all.length() != 0) {
        String tmp = selectToMarker(command_all, "\n");
        sCmd.readStr(tmp);
        command_all = deleteBeforeDelimiter(command_all, "\n");
    }
    command_all = "";
}

void stringExecution(String str) {
    str = str + "\r\n";

    str.replace("\r\n", "\n");
    str.replace("\r", "\n");

    while (str.length() != 0) {
        String tmp = selectToMarker(str, "\n");
        sCmd.readStr(tmp);

        str = deleteBeforeDelimiter(str, "\n");
    }
}