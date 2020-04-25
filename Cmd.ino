void CMD_init() {

  sCmd.addCommand("button",  button);
  sCmd.addCommand("buttonSet",  buttonSet);
  sCmd.addCommand("buttonChange",  buttonChange);

  sCmd.addCommand("pinSet",  pinSet);
  sCmd.addCommand("pinChange",  pinChange);

  sCmd.addCommand("pwm",  pwm);
  sCmd.addCommand("pwmSet",  pwmSet);

  sCmd.addCommand("switch",  switch_);

  sCmd.addCommand("analog",  analog);
  sCmd.addCommand("level",  level);
  sCmd.addCommand("dallas",  dallas);

  sCmd.addCommand("dhtT",  dhtT);
  sCmd.addCommand("dhtH",  dhtH);
  sCmd.addCommand("dhtPerception",  dhtPerception);
  sCmd.addCommand("dhtComfort",  dhtComfort);
  sCmd.addCommand("dhtDewpoint",  dhtDewpoint);

  sCmd.addCommand("stepper",  stepper);
  sCmd.addCommand("stepperSet",  stepperSet);

  sCmd.addCommand("servo",  servo_);
  sCmd.addCommand("servoSet",  servoSet);

  sCmd.addCommand("logging",  logging);

  sCmd.addCommand("inputDigit",  inputDigit);
  sCmd.addCommand("digitSet",  digitSet);

  sCmd.addCommand("inputTime",  inputTime);
  sCmd.addCommand("timeSet",  timeSet);

  sCmd.addCommand("timerStart",  timerStart);
  sCmd.addCommand("timerStop",  timerStop);

  sCmd.addCommand("text",  text);
  sCmd.addCommand("textSet",  textSet);


  sCmd.addCommand("mqtt",  mqttOrderSend);
  sCmd.addCommand("http",  httpOrderSend);
  sCmd.addCommand("push",  pushControl);


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

  jsonWriteStr(optionJson, "button_param" + button_number, button_param);
  jsonWriteStr(configJson, "buttonSet" + button_number, start_state);

  if (isDigitStr (button_param)) {
    pinMode(button_param.toInt(), OUTPUT);
    digitalWrite(button_param.toInt(), start_state.toInt());
  }

  if  (button_param == "scenario") {
    jsonWriteStr(configSetup, "scenario", start_state);
    Scenario_init();
    saveConfig();
  }

  if  (button_param.indexOf("line") != -1) {
    String str = button_param;
    while (str.length() != 0) {
      if (str == "") return;
      String tmp = selectToMarker (str, ",");  //line1,
      String number = deleteBeforeDelimiter(tmp, "e");   //1,
      number.replace(",", "");
      Serial.println(number);
      int number_int = number.toInt();
      scenario_line_status[number_int] = start_state.toInt();
      str = deleteBeforeDelimiter(str, ",");
    }
  }
  createWidget (widget_name, page_name, page_number, "widgets/widget.toggle.json", "buttonSet" + button_number);
}

void buttonSet() {

  String button_number = sCmd.next();
  String button_state = sCmd.next();
  String button_param = jsonRead(optionJson, "button_param" + button_number);

  if (button_param != "na" || button_param != "scenario" || button_param.indexOf("line") != -1) {
    digitalWrite(button_param.toInt(), button_state.toInt());
  }

  if  (button_param == "scenario") {
    jsonWriteStr(configSetup, "scenario", button_state);
    Scenario_init();
    saveConfig();
  }

  if  (button_param.indexOf("line") != -1) {
    String str = button_param;
    while (str.length() != 0) {
      if (str == "") return;
      String tmp = selectToMarker (str, ",");  //line1,
      String number = deleteBeforeDelimiter(tmp, "e");   //1,
      number.replace(",", "");
      Serial.println(number);
      int number_int = number.toInt();
      scenario_line_status[number_int] = button_state.toInt();
      str = deleteBeforeDelimiter(str, ",");
    }
  }

  eventGen ("buttonSet", button_number);

  jsonWriteStr(configJson, "buttonSet" + button_number, button_state);
  sendSTATUS("buttonSet" + button_number, button_state);
}

void buttonChange() {
  String button_number = sCmd.next();
  String current_state = jsonRead(configJson, "buttonSet" + button_number);
  if (current_state == "1") {
    current_state = "0";
  } else if (current_state == "0") {
    current_state = "1";
  }
  order_loop += "buttonSet " + button_number + " " + current_state + ",";
  jsonWriteStr(configJson, "buttonSet" + button_number, current_state);
  sendSTATUS("buttonSet" + button_number, current_state);
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

  static boolean flag = true;
  String pwm_number = sCmd.next();
  String pwm_pin = sCmd.next();
  String widget_name = sCmd.next();
  widget_name.replace("#", " ");
  String page_name = sCmd.next();
  String start_state = sCmd.next();
  String page_number = sCmd.next();


  uint8_t pwm_pin_int = pwm_pin.toInt();
  jsonWriteStr(optionJson, "pwm_pin" + pwm_number, pwm_pin);
  pinMode(pwm_pin_int, INPUT);
  analogWrite(pwm_pin_int, start_state.toInt());
  //analogWriteFreq(32000);
  jsonWriteStr(configJson, "pwmSet" + pwm_number, start_state);

  createWidget (widget_name, page_name, page_number, "widgets/widget.range.json", "pwmSet" + pwm_number);
}

void pwmSet() {

  String pwm_number = sCmd.next();
  String pwm_state = sCmd.next();
  int pwm_state_int = pwm_state.toInt();

  int pin = jsonReadtoInt(optionJson, "pwm_pin" + pwm_number);
  analogWrite(pin, pwm_state_int);

  eventGen ("pwmSet", pwm_number);

  jsonWriteStr(configJson, "pwmSet" + pwm_number, pwm_state);
  sendSTATUS("pwmSet" + pwm_number, pwm_state);
}
//==================================================================================================================
//==========================================Модуль физической кнопки================================================
void switch_ () {

  String switch_number = sCmd.next();
  String switch_pin = sCmd.next();
  String switch_delay = sCmd.next();

  buttons[switch_number.toInt()].attach(switch_pin.toInt());
  buttons[switch_number.toInt()].interval(switch_delay.toInt());
  but[switch_number.toInt()] = true;
}

void handleButton()  {

  static uint8_t switch_number = 1;

  if (but[switch_number]) {
    buttons[switch_number].update();
    if (buttons[switch_number].fell()) {

      eventGen ("switchSet", String(switch_number));

      jsonWriteStr(configJson, "switchSet" + String(switch_number), "1");
    }
    if (buttons[switch_number].rose()) {

      eventGen ("switchSet", String(switch_number));

      jsonWriteStr(configJson, "switchSet" + String(switch_number), "0");
    }
  }
  switch_number++;
  if (switch_number == NUM_BUTTONS) switch_number = 0;
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
  jsonWriteStr(configJson, "digitSet" + number, start_state);
  createWidget (widget_name, page_name, page_number, "widgets/widget.inputNum.json", "digitSet" + number);
}
void digitSet() {
  String number = sCmd.next();
  String value = sCmd.next();
  jsonWriteStr(configJson, "digitSet" + number, value);
  sendSTATUS("digitSet" + number, value);
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
  jsonWriteStr(configJson, "timeSet" + number, start_state);
  createWidget (widget_name, page_name, page_number, "widgets/widget.inputTime.json", "timeSet" + number);
}
void timeSet() {
  String number = sCmd.next();
  String value = sCmd.next();
  jsonWriteStr(configJson, "timeSet" + number, value);
  sendSTATUS("timeSet" + number, value);
}

void handle_time_init() {
  ts.add(TIME, 1000, [&](void*) {

    String tmp = GetTime();
    jsonWriteStr(configJson, "time", tmp);
    tmp.replace(":", "-");
    jsonWriteStr(configJson, "timenowSet", tmp);
    eventGen ("timenowSet", "");

  }, nullptr, true);
}

//=====================================================================================================================================
//=========================================Добавление текстового виджета============================================================
void text() {

  String number = sCmd.next();
  String widget_name = sCmd.next();
  String page_name = sCmd.next();
  String page_number = sCmd.next();

  createWidget (widget_name, page_name, page_number, "widgets/widget.anyData.json", "textSet" + number);
}


void textSet() {

  String number = sCmd.next();
  String text = sCmd.next();
  text.replace("_", " ");

  if (text.indexOf("-time") >= 0) {
    text.replace("-time", "");
    text.replace("#", " ");
    String time = GetTime();
    time.replace(":", ".");
    text = text + " " + GetDataDigital() + " " + time;
  }

  jsonWriteStr(configJson, "textSet" + number, text);
  sendSTATUS("textSet" + number, text);
}

//=====================================================================================================================================
//=========================================Модуль шагового мотора======================================================================

//stepper 1 12 13
void stepper() {
  String stepper_number = sCmd.next();
  String pin_step = sCmd.next();
  String pin_dir = sCmd.next();

  jsonWriteStr(optionJson, "stepper" + stepper_number, pin_step + " " + pin_dir);
  pinMode(pin_step.toInt(), OUTPUT);
  pinMode(pin_dir.toInt(), OUTPUT);
}

//stepperSet 1 100 5
void stepperSet() {
  String stepper_number = sCmd.next();
  String steps = sCmd.next();
  jsonWriteStr(optionJson, "steps" + stepper_number, steps);
  String stepper_speed = sCmd.next();
  String pin_step = selectToMarker (jsonRead(optionJson, "stepper" + stepper_number), " ");
  String pin_dir =  deleteBeforeDelimiter (jsonRead(optionJson, "stepper" + stepper_number), " ");
  Serial.println(pin_step);
  Serial.println(pin_dir);
  if (steps.toInt() > 0) digitalWrite(pin_dir.toInt(), HIGH);
  if (steps.toInt() < 0) digitalWrite(pin_dir.toInt(), LOW);
  if (stepper_number == "1") {
    ts.add(STEPPER1, stepper_speed.toInt(), [&](void*) {
      int steps_int = abs(jsonReadtoInt(optionJson, "steps1") * 2);
      static int count;
      count++;
      String pin_step = selectToMarker (jsonRead(optionJson, "stepper1"), " ");
      digitalWrite(pin_step.toInt(), !digitalRead(pin_step.toInt()));
      yield();
      if (count > steps_int) {
        digitalWrite(pin_step.toInt(), LOW);
        ts.remove(STEPPER1);
        count = 0;
      }
    }, nullptr, true);
  }
  if (stepper_number == "2") {
    ts.add(STEPPER2, stepper_speed.toInt(), [&](void*) {
      int steps_int = abs(jsonReadtoInt(optionJson, "steps2") * 2);
      static int count;
      count++;
      String pin_step = selectToMarker (jsonRead(optionJson, "stepper2"), " ");
      digitalWrite(pin_step.toInt(), !digitalRead(pin_step.toInt()));
      yield();
      if (count > steps_int) {
        digitalWrite(pin_step.toInt(), LOW);
        ts.remove(STEPPER2);
        count = 0;
      }
    }, nullptr, true);
  }
}

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

  jsonWriteStr(optionJson, "servo_pin" + servo_number, servo_pin);
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

  jsonWriteStr(optionJson, "s_min_val" + servo_number, min_value);
  jsonWriteStr(optionJson, "s_max_val" + servo_number, max_value);
  jsonWriteStr(optionJson, "s_min_deg" + servo_number, min_deg);
  jsonWriteStr(optionJson, "s_max_deg" + servo_number, max_deg);

  jsonWriteStr(configJson, "servoSet" + servo_number, start_state);

  createWidgetParam (widget_name, page_name, page_number, "widgets/widget.range.json", "servoSet" + servo_number, "min", min_value, "max", max_value, "k", "1");
}

void servoSet() {
  String servo_number = sCmd.next();
  String servo_state = sCmd.next();
  int servo_state_int = servo_state.toInt();

  int pin = jsonReadtoInt(optionJson, "servo_pin" + servo_number);

  servo_state_int = map(servo_state_int,
                        jsonReadtoInt(optionJson, "s_min_val" + servo_number),
                        jsonReadtoInt(optionJson, "s_max_val" + servo_number),
                        jsonReadtoInt(optionJson, "s_min_deg" + servo_number),
                        jsonReadtoInt(optionJson, "s_max_deg" + servo_number));

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

  eventGen ("servoSet", servo_number);

  jsonWriteStr(configJson, "servoSet" + servo_number, servo_state);
  sendSTATUS("servoSet" + servo_number, servo_state);
}
//====================================================================================================================================================
/*
  void inputText() {
  String number = sCmd.next();
  String widget_name = sCmd.next();
  widget_name.replace("#", " ");
  String page_name = sCmd.next();
  page_name.replace("#", " ");
  String start_state = sCmd.next();
  String page_number = sCmd.next();
  jsonWriteStr(configJson, "inputTextSet" + number, start_state);
  createWidget (widget_name, page_name, page_number, "widgets/widget.inputText.json", "inputTextSet" + number);
  }
  void inputTextSet() {
  String number = sCmd.next();
  String value = sCmd.next();
  jsonWriteStr(configJson, "inputTextSet" + number, value);
  sendSTATUS("inputTextSet" + number, value);
  }

  void inputTime() {
  String number = sCmd.next();
  String widget_name = sCmd.next();
  widget_name.replace("#", " ");
  String page_name = sCmd.next();
  page_name.replace("#", " ");
  String start_state = sCmd.next();
  String page_number = sCmd.next();
  jsonWriteStr(configJson, "inputTimeSet" + number, start_state);
  createWidget (widget_name, page_name, page_number, "widgets/widget.inputTime.json", "inputTimeSet" + number);
  }
  void inputTimeSet() {
  String number = sCmd.next();
  String value = sCmd.next();
  value.replace(":", ".");
  jsonWriteStr(configJson, "inputTimeSet" + number, value);
  value.replace(".", ":");
  sendSTATUS("inputTimeSet" + number, value);
  }


  void inputDate() {
  String number = sCmd.next();
  String widget_name = sCmd.next();
  widget_name.replace("#", " ");
  String page_name = sCmd.next();
  page_name.replace("#", " ");
  String start_state = sCmd.next();
  String page_number = sCmd.next();
  jsonWriteStr(configJson, "inputDateSet" + number, start_state);
  createWidget (widget_name, page_name, page_number, "widgets/widget.inputDate.json", "inputDateSet" + number);
  }
  void inputDateSet() {
  String number = sCmd.next();
  String value = sCmd.next();
  jsonWriteStr(configJson, "inputDateSet" + number, value);
  sendSTATUS("inputDateSet" + number, value);
  }
*/
//=================================================Глобальные команды удаленного управления===========================================================

void mqttOrderSend() {

  String id = sCmd.next();
  String order = sCmd.next();

  String  all_line = jsonRead(configSetup, "mqttPrefix") + "/" + id + "/order";
  //Serial.print(all_line);
  //Serial.print("->");
  //Serial.println(order);
  int send_status = client.publish (all_line.c_str(), order.c_str(), false);
}

void httpOrderSend() {

  String ip = sCmd.next();
  String order = sCmd.next();
  order.replace("_", "%20");
  String url = "http://" + ip + "/cmd?command=" + order;
  getURL(url);
}



//==============================================================================================================================
//============================выполнение команд (в лупе) по очереди из строки order=============================================
void handleCMD_loop() {

  if (order_loop != "") {

    String tmp = selectToMarker(order_loop, ",");                //выделяем из страки order первую команду rel 5 1,
    sCmd.readStr(tmp);                                           //выполняем первую команду
    Serial.println("[ORDER] => " + order_loop);
    order_loop = deleteBeforeDelimiter(order_loop, ",");         //осекаем выполненную команду
  }
}

//=======================================================================================================================================
//=======================================================================================================================================
void txtExecution(String file) {

  String command_all = readFile(file, 2048) + "\r\n";

  command_all.replace("\r\n", "\n");
  command_all.replace("\r", "\n");

  while (command_all.length() != 0) {

    String tmp = selectToMarker (command_all, "\n");
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

    String tmp = selectToMarker (str, "\n");
    sCmd.readStr(tmp);
    str = deleteBeforeDelimiter(str, "\n");
  }
}
