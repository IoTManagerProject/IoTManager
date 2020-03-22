void CMD_init() {

  sCmd.addCommand("button",  button);
  sCmd.addCommand("buttonSet",  buttonSet);
  sCmd.addCommand("buttonChange",  buttonChange);

  //sCmd.addCommand("button_touch",  button_touch);

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

  //======новые виджеты ver2.0=======//
  /*
    sCmd.addCommand("inputText",  inputText);
    sCmd.addCommand("inputTextSet",  inputTextSet);

    sCmd.addCommand("inputTime",  inputTime);
    sCmd.addCommand("inputTimeSet",  inputTimeSet);

    sCmd.addCommand("inputDate",  inputDate);
    sCmd.addCommand("inputDateSet",  inputDateSet);

    sCmd.addCommand("inputDate",  inputDate);

    //sCmd.addCommand("inputDropdown",  inputDropdown);
  */
  //=================================//
}



//==========================================================================================================
//==========================================Модуль кнопок===================================================
void button() {

  String button_number = sCmd.next();
  String button_param = sCmd.next();
  String viget_name = sCmd.next();
  String page_name = sCmd.next();
  String start_state = sCmd.next();
  String page_number = sCmd.next();

  jsonWrite(optionJson, "button_param" + button_number, button_param);
  jsonWrite(configJson, "buttonSet" + button_number, start_state);

  if (isDigitStr (button_param)) {
    pinMode(button_param.toInt(), OUTPUT);
    digitalWrite(button_param.toInt(), start_state.toInt());
  }

  if  (button_param == "scenario") {
    jsonWrite(configSetup, "scenario", start_state);
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
  createViget (viget_name, page_name, page_number, "vigets/viget.toggle.json", "buttonSet" + button_number);
}

void buttonSet() {

  String button_number = sCmd.next();
  String button_state = sCmd.next();
  String button_param = jsonRead(optionJson, "button_param" + button_number);

  if (button_param != "na" || button_param != "scenario" || button_param.indexOf("line") != -1) {
    digitalWrite(button_param.toInt(), button_state.toInt());
  }

  if  (button_param == "scenario") {
    jsonWrite(configSetup, "scenario", button_state);
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

  jsonWrite(configJson, "buttonSet" + button_number, button_state);
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
  jsonWrite(configJson, "buttonSet" + button_number, current_state);
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
  String viget_name = sCmd.next();
  viget_name.replace("#", " ");
  String page_name = sCmd.next();
  String start_state = sCmd.next();
  String page_number = sCmd.next();


  uint8_t pwm_pin_int = pwm_pin.toInt();
  jsonWrite(optionJson, "pwm_pin" + pwm_number, pwm_pin);
  pinMode(pwm_pin_int, INPUT);
  analogWrite(pwm_pin_int, start_state.toInt());
  //analogWriteFreq(32000);
  jsonWrite(configJson, "pwmSet" + pwm_number, start_state);

  createViget (viget_name, page_name, page_number, "vigets/viget.range.json", "pwmSet" + pwm_number);
}

void pwmSet() {

  String pwm_number = sCmd.next();
  String pwm_state = sCmd.next();
  int pwm_state_int = pwm_state.toInt();

  int pin = jsonReadtoInt(optionJson, "pwm_pin" + pwm_number);
  analogWrite(pin, pwm_state_int);

  eventGen ("pwmSet", pwm_number);

  jsonWrite(configJson, "pwmSet" + pwm_number, pwm_state);
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

      jsonWrite(configJson, "switchSet" + String(switch_number), "1");
    }
    if (buttons[switch_number].rose()) {

      eventGen ("switchSet", String(switch_number));

      jsonWrite(configJson, "switchSet" + String(switch_number), "0");
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
  String viget_name = sCmd.next();
  viget_name.replace("#", " ");
  String page_name = sCmd.next();
  page_name.replace("#", " ");
  String start_state = sCmd.next();
  String page_number = sCmd.next();
  jsonWrite(configJson, "digitSet" + number, start_state);
  createViget (viget_name, page_name, page_number, "vigets/viget.inputNum.json", "digitSet" + number);
}
void digitSet() {
  String number = sCmd.next();
  String value = sCmd.next();
  jsonWrite(configJson, "digitSet" + number, value);
  sendSTATUS("digitSet" + number, value);
}
//=====================================================================================================================================
//=========================================Добавление окна ввода времени===============================================================
void inputTime() {
  String value_name = sCmd.next();
  String number = value_name.substring(4);
  String viget_name = sCmd.next();
  viget_name.replace("#", " ");
  String page_name = sCmd.next();
  page_name.replace("#", " ");
  String start_state = sCmd.next();
  String page_number = sCmd.next();
  jsonWrite(configJson, "timeSet" + number, start_state);
  createViget (viget_name, page_name, page_number, "vigets/viget.inputTime.json", "timeSet" + number);
}
void timeSet() {
  String number = sCmd.next();
  String value = sCmd.next();
  jsonWrite(configJson, "timeSet" + number, value);
  sendSTATUS("timeSet" + number, value);
}

void handle_time_init() {
  ts.add(TIME, 1000, [&](void*) {

    String tmp = GetTime();
    tmp.replace(":", "-");
    jsonWrite(configJson, "timenowSet", tmp);
    eventGen ("timenowSet", "");

  }, nullptr, true);
}

//=====================================================================================================================================
//=========================================Добавление текстового виджета============================================================
void text() {

  String number = sCmd.next();
  String viget_name = sCmd.next();
  String page_name = sCmd.next();
  String page_number = sCmd.next();

  createViget (viget_name, page_name, page_number, "vigets/viget.anydata.json", "textSet" + number);
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

  jsonWrite(configJson, "textSet" + number, text);
  sendSTATUS("textSet" + number, text);
}

//=====================================================================================================================================
//=========================================Модуль шагового мотора======================================================================

//stepper 1 12 13
void stepper() {
  String stepper_number = sCmd.next();
  String pin_step = sCmd.next();
  String pin_dir = sCmd.next();

  jsonWrite(optionJson, "stepper" + stepper_number, pin_step + " " + pin_dir);
  pinMode(pin_step.toInt(), OUTPUT);
  pinMode(pin_dir.toInt(), OUTPUT);
}

//stepperSet 1 100 5
void stepperSet() {
  String stepper_number = sCmd.next();
  String steps = sCmd.next();
  jsonWrite(optionJson, "steps" + stepper_number, steps);
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
      if (count > steps_int) {
        digitalWrite(pin_step.toInt(), LOW);
        ts.remove(STEPPER2);
        count = 0;
      }
    }, nullptr, true);
  }
}


//====================================================================================================================================================
/*
  void inputText() {
  String number = sCmd.next();
  String viget_name = sCmd.next();
  viget_name.replace("#", " ");
  String page_name = sCmd.next();
  page_name.replace("#", " ");
  String start_state = sCmd.next();
  String page_number = sCmd.next();
  jsonWrite(configJson, "inputTextSet" + number, start_state);
  createViget (viget_name, page_name, page_number, "vigets/viget.inputText.json", "inputTextSet" + number);
  }
  void inputTextSet() {
  String number = sCmd.next();
  String value = sCmd.next();
  jsonWrite(configJson, "inputTextSet" + number, value);
  sendSTATUS("inputTextSet" + number, value);
  }

  void inputTime() {
  String number = sCmd.next();
  String viget_name = sCmd.next();
  viget_name.replace("#", " ");
  String page_name = sCmd.next();
  page_name.replace("#", " ");
  String start_state = sCmd.next();
  String page_number = sCmd.next();
  jsonWrite(configJson, "inputTimeSet" + number, start_state);
  createViget (viget_name, page_name, page_number, "vigets/viget.inputTime.json", "inputTimeSet" + number);
  }
  void inputTimeSet() {
  String number = sCmd.next();
  String value = sCmd.next();
  value.replace(":", ".");
  jsonWrite(configJson, "inputTimeSet" + number, value);
  value.replace(".", ":");
  sendSTATUS("inputTimeSet" + number, value);
  }


  void inputDate() {
  String number = sCmd.next();
  String viget_name = sCmd.next();
  viget_name.replace("#", " ");
  String page_name = sCmd.next();
  page_name.replace("#", " ");
  String start_state = sCmd.next();
  String page_number = sCmd.next();
  jsonWrite(configJson, "inputDateSet" + number, start_state);
  createViget (viget_name, page_name, page_number, "vigets/viget.inputDate.json", "inputDateSet" + number);
  }
  void inputDateSet() {
  String number = sCmd.next();
  String value = sCmd.next();
  jsonWrite(configJson, "inputDateSet" + number, value);
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

//=============================выполнение команд (через период) по очереди из строки order=======================================
/*void handleCMD_ticker() {

  ts.add(CMD, CMD_update_int, [&](void*) {
    if (!busy) {
      if (order_ticker != "") {

        String tmp = selectToMarker(order_ticker, ",");                //выделяем из страки order первую команду pus title body
        if (tmp != "no_command") sCmd.readStr(tmp);                                             //выполняем первую команду
        Serial.println("order_ticker => " + order_ticker);
        order_ticker = deleteBeforeDelimiter(order_ticker, ",");       //осекаем выполненную команду
      }
    }
  }, nullptr, true);
  }*/



//=======================================================================================================================================
//=======================================================================================================================================
void txtExecution(String file) {

  String command_all = readFile(file, 2048) + "\r\n";  //2048

  command_all.replace("\r\n", "\n");
  command_all.replace("\r", "\n");

  while (command_all.length() != 0) {

    String tmp = selectToMarker (command_all, "\n");
    //if (tmp.indexOf("//") < 0)
    sCmd.readStr(tmp);
    command_all = deleteBeforeDelimiter(command_all, "\n");
  }
}
void stringExecution(String str) {

  String command_all = str + "\r\n";  //"\r\n"

  command_all.replace("\r\n", "\n");
  command_all.replace("\r", "\n");

  while (command_all.length() != 0) {

    String tmp = selectToMarker (command_all, "\n");
    //if (tmp.indexOf("//") < 0)
    sCmd.readStr(tmp);
    command_all = deleteBeforeDelimiter(command_all, "\n");
  }
}

//======================================================================================================================
//===============================================Создание виджетов=======================================================

void createViget (String viget_name, String  page_name, String page_number, String file, String topic) {

  String viget;
  viget = readFile(file, 1024);

  if (viget == "Failed") return;
  if (viget == "Large") return;

  viget_name.replace("#", " ");
  page_name.replace("#", " ");

  jsonWrite(viget, "page", page_name);
  jsonWrite(viget, "order", page_number);
  jsonWrite(viget, "descr", viget_name);
  jsonWrite(viget, "topic", prex + "/" + topic);
  all_vigets += viget + "\r\n";
  viget = "";
}

/*
  void createViget (String viget_name, String  page_name, String page_number, String file, String topic, String key, String value) {

  String viget;
  viget = readFile(file, 1024);

  if (viget == "Failed") return;
  if (viget == "Large") return;

  viget_name.replace("#", " ");
  page_name.replace("#", " ");

  value.replace("#", " ");

  viget = vidgetConfigWrite(viget, key, value);

  jsonWrite(viget, "page", page_name);
  jsonWrite(viget, "pageId", page_number);
  jsonWrite(viget, "descr", viget_name);
  jsonWrite(viget, "topic", prex + "/" + topic);

  all_vigets += viget + "\r\n";
  viget = "";
  }

  void createViget (String viget_name, String  page_name, String page_number, String file, String topic, String key, String value, String key2, String value2) {

  String viget;
  viget = readFile(file, 1024);

  if (viget == "Failed") return;
  if (viget == "Large") return;

  viget_name.replace("#", " ");
  page_name.replace("#", " ");

  value.replace("#", " ");

  viget = vidgetConfigWrite(viget, key, value);
  viget = vidgetConfigWrite(viget, key2, value2);

  jsonWrite(viget, "page", page_name);
  jsonWrite(viget, "pageId", page_number);
  jsonWrite(viget, "descr", viget_name);
  jsonWrite(viget, "topic", prex + "/" + topic);

  all_vigets += viget + "\r\n";
  viget = "";
  }

String vidgetConfigWrite(String viget, String key, String value) {

  if (viget == "") return "";
  if (viget == "{}") return "";
  int psn1 = viget.indexOf("{");
  if (psn1 != -1) {
    psn1 = viget.indexOf("{", psn1 + 1);
    if (psn1 != -1) {
      int psn2 = viget.indexOf("}", psn1);
      String WigetConfig = viget.substring(psn1, psn2) + "}";
      jsonWrite(WigetConfig, key, value);
      String part1 = viget.substring(0, psn1);
      viget = part1 + WigetConfig + "}";
      return viget;

    }
  }
}
*/











//============разное

/*
  void delAlert() {

  String alert_id = sCmd.next();
  delViget(alert_id);
  sendAllWigets();
  }


  void delViget(String text_in_viget) {
  String viget = all_vigets;
  while (viget.length() != 0) {
    String tmp = selectToMarkerPlus (viget, "\r\n", 2);
    if (tmp.indexOf(text_in_viget) > 0) {

      all_vigets.replace(tmp, "");
      //Serial.println(all_vigets);

      viget = deleteBeforeDelimiter(viget, "\r\n");
    } else {
      viget = deleteBeforeDelimiter(viget, "\r\n");
    }
  }
  }
*/
