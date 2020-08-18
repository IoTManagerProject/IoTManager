#include "Cmd.h"

#include "Class/Button.h"
#include "Class/Input.h"
#include "Class/LineParsing.h"
#include "Class/Pwm.h"
#include "Class/Switch.h"
//-----------------------------
#include "Global.h"
#include "Module/Terminal.h"
#include "Servo/Servos.h"

static const char *MODULE = "Cmd";

Terminal *term = nullptr;

boolean but[NUM_BUTTONS];
Bounce *buttons = new Bounce[NUM_BUTTONS];

#ifdef ESP8266
SoftwareSerial *mySerial = nullptr;
#else
HardwareSerial *mySerial = nullptr;
#endif

void getData();

void cmd_init() {
    sCmd.addCommand("button-out", buttonOut);
    sCmd.addCommand("pwm-out", pwmOut);
    sCmd.addCommand("button-in", buttonIn);
    sCmd.addCommand("input-digit", inputDigit);
    sCmd.addCommand("input-time", inputTime);

    sCmd.addCommand("text", text);
    sCmd.addCommand("textSet", textSet);




    sCmd.addCommand("timerStart", timerStart_);
    sCmd.addCommand("timerStop", timerStop_);

    

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

    sCmd.addCommand("mqtt", mqttOrderSend);
    sCmd.addCommand("http", httpOrderSend);

#ifdef PUSH_ENABLED
    sCmd.addCommand("push", pushControl);
#endif

    sCmd.addCommand("firmwareUpdate", firmwareUpdate);
    sCmd.addCommand("firmwareVersion", firmwareVersion);

    handle_time_init();
}

//==========================================Модуль кнопок===================================================
//button-out light toggle Кнопки Свет 1 pin[12] inv[1] st[1]
//==========================================================================================================
void buttonOut() {
    myButton = new Button1();
    myButton->update();
    String key = myButton->gkey();
    String pin = myButton->gpin();
    String inv = myButton->ginv();
    sCmd.addCommand(key.c_str(), buttonOutSet);
    jsonWriteStr(configOptionJson, key + "_pin", pin);
    jsonWriteStr(configOptionJson, key + "_inv", inv);
    myButton->pinModeSet();
    myButton->pinStateSetDefault();
    myButton->pinStateSetInvDefault();
    myButton->clear();
}

void buttonOutSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    String pin = jsonReadStr(configOptionJson, key + "_pin");
    String inv = jsonReadStr(configOptionJson, key + "_inv");
    if (inv == "") {
        myButton->pinChange(key, pin, state, true);
    } else {
        myButton->pinChange(key, pin, state, false);
    }
}

//==========================================Модуль управления ШИМ===================================================
//pwm-out volume range Кнопки Свет 1 pin[12] st[500]
//==================================================================================================================
void pwmOut() {
    myPwm = new Pwm();
    myPwm->update();
    String key = myPwm->gkey();
    String pin = myPwm->gpin();
    String inv = myPwm->ginv();
    sCmd.addCommand(key.c_str(), pwmOutSet);
    jsonWriteStr(configOptionJson, key + "_pin", pin);
    myPwm->pwmModeSet();
    myPwm->pwmStateSetDefault();
    myPwm->clear();
}

void pwmOutSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    String pin = jsonReadStr(configOptionJson, key + "_pin");
    myPwm->pwmChange(key, pin, state);
}

//==========================================Модуль физических кнопок========================================
//button-in switch1 toggle Кнопки Свет 1 pin[2] db[20]
//==========================================================================================================
void buttonIn() {
    mySwitch = new Switch();
    mySwitch->update();
    String key = mySwitch->gkey();
    String pin = mySwitch->gpin();
    sCmd.addCommand(key.c_str(), buttonInSet);
    mySwitch->init();
    mySwitch->switchStateSetDefault();
    mySwitch->clear();
}

void buttonInSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    mySwitch->switchChangeVirtual(key, state);
}

//==========================================Модуль ввода цифровых значений==================================
//input-digit digit1 inputDigit Ввод Введите.цифру 4 st[60]
//==========================================================================================================
void inputDigit() {
    myInput = new Input();
    myInput->update();
    String key = myInput->gkey();
    sCmd.addCommand(key.c_str(), inputDigitSet);
    myInput->inputSetDefaultFloat();
    myInput->clear();
}

void inputDigitSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    myInput->inputSetFloat(key, state);
}

//==========================================Модуль ввода времени============================================
//input-time time1 inputTime Ввод Введите.время 4 st[10-00-00]
//==========================================================================================================
void inputTime() {
    myInput = new Input();
    myInput->update();
    String key = myInput->gkey();
    sCmd.addCommand(key.c_str(), inputTimeSet);
    myInput->inputSetDefaultStr();
    myInput->clear();
}

void inputTimeSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    myInput->inputSetStr(key, state);
}

void handle_time_init() {
    ts.add(
        TIME, 1000, [&](void *) {
            jsonWriteStr(configLiveJson, "time", timeNow->getTime());
            jsonWriteStr(configLiveJson, "timenow", timeNow->getTimeJson());
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

    createWidgetParam(widget, page, pageNumber, "range", "servo" + number, "min", String(min_value), "max", String(max_value), "k", "1");
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

    eventGen("servo", number);
    jsonWriteInt(configLiveJson, "servo" + number, value);
    MqttClient::publishStatus("servo" + number, String(value, DEC));
}
#endif
//====================================================================================================================================================
//=============================================================Модуль сериал порта=======================================================================

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

void mqttOrderSend() {
    String id = sCmd.next();
    String order = sCmd.next();

    String all_line = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + id + "/order";
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
        String tmp = selectToMarker(order_loop, ",");  //выделяем первую команду rel 5 1,
        pm.info("do: " + tmp);
        sCmd.readStr(tmp);                                    //выполняем
        order_loop = deleteBeforeDelimiter(order_loop, ",");  //осекаем
    }
}

void loopSerial() {
    if (term) {
        term->loop();
    }
}
