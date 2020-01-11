void CMD_init() {


  sCmd.addCommand("button",  button);
  sCmd.addCommand("buttonSet",  buttonSet);

  sCmd.addCommand("pinSet",  pinSet);

  sCmd.addCommand("pwm",  pwm);
  sCmd.addCommand("pwmSet",  pwmSet);

  sCmd.addCommand("switch",  switch_);

  sCmd.addCommand("analog",  analog);
  sCmd.addCommand("ph",  ph);
  sCmd.addCommand("level",  level);
  sCmd.addCommand("dallas",  dallas);

  sCmd.addCommand("logging",  logging);

  sCmd.addCommand("input",  input);
  sCmd.addCommand("valueUpSet",  valueUpSet);
  sCmd.addCommand("valueDownSet",  valueDownSet);



  sCmd.addCommand("text",  text);
  sCmd.addCommand("textSet",  textSet);

  // sCmd.addCommand("time",  time);
  // sCmd.addCommand("timeSet",  timeSet);

  sCmd.addCommand("timerStart",  timerStart);
  sCmd.addCommand("timerStop",  timerStop);

  sCmd.addCommand("mqtt",  mqttOrderSend);
  sCmd.addCommand("http",  httpOrderSend);
  //!sCmd.addCommand("push",  pushControl);

  //handleCMD_ticker();


  //======новые виджеты ver2.0=======//

  sCmd.addCommand("inputNumber",  inputNumber);
  sCmd.addCommand("inputNumberSet",  inputNumberSet);

  sCmd.addCommand("inputText",  inputText);
  sCmd.addCommand("inputTextSet",  inputTextSet);

  sCmd.addCommand("inputTime",  inputTime);
  sCmd.addCommand("inputTimeSet",  inputTimeSet);

  sCmd.addCommand("inputDate",  inputDate);
  sCmd.addCommand("inputDateSet",  inputDateSet);

  //sCmd.addCommand("inputDropdown",  inputDropdown);

  //=================================//

}

void inputNumber() {
  String number = sCmd.next();
  String viget_name = sCmd.next();
  viget_name.replace("#", " ");
  String page_name = sCmd.next();
  page_name.replace("#", " ");
  String start_state = sCmd.next();
  String page_number = sCmd.next();
  jsonWrite(configJson, "inputNumberSet" + number, start_state);
  createViget (viget_name, page_name, page_number, "vigets/viget.inputNumber.json", "inputNumberSet" + number);
}
void inputNumberSet() {
  String number = sCmd.next();
  String value = sCmd.next();
  jsonWrite(configJson, "inputNumberSet" + number, value);
  sendSTATUS("inputNumberSet" + number, value);
}

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

void pinSet() {

  String pin_number = sCmd.next();
  String pin_state = sCmd.next();
  pinMode(pin_number.toInt(), OUTPUT);
  digitalWrite(pin_number.toInt(), pin_state.toInt());

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
//=========================================Добавление окна ввода переменной============================================================
void input() {

  String name_ = sCmd.next();
  String number = name_.substring(5);
  String start_value = sCmd.next();
  String step_ = sCmd.next();
  String value_name = sCmd.next();
  String page_name = sCmd.next();
  String page_number = sCmd.next();

  int psn1 = start_value.indexOf(".");                         //ищем позицию запятой
  int digits = 0;
  if (psn1 != -1) {                                            //если она есть
    String last_part = deleteBeforeDelimiter(start_value, ".");
    digits = last_part.length() + 1;
  }

  createViget ("", page_name, page_number, "vigets/viget.button.json", "valueDownSet" + number, "title", "-");
  createViget (value_name, page_name, String(page_number.toInt() + 1), "vigets/viget.alertbg.json", name_);
  createViget ("", page_name, String(page_number.toInt() + 2), "vigets/viget.button.json", "valueUpSet" + number , "title", "+");

  //jsonWrite(valuesJson, name_, start_value);
  //saveValues ();
  sendSTATUS(name_, start_value);

  jsonWrite(configJson, name_ + "step", step_);
  jsonWrite(configJson, name_ + "digits", digits);
}

void valueUpSet() {
  String number = sCmd.next();
  float val = jsonRead(configJson, "value" + number).toFloat();
  float step_ = jsonRead(configJson, "value" + number + "step").toFloat();
  int digits = jsonRead(configJson, "value" + number + "digits").toInt();
  val = val + step_;
  String val_str = String(val);
  val_str = selectToMarkerPlus (val_str, ".", digits);
  jsonWrite(configJson, "value" + number, val_str);
  //jsonWrite(valuesJson, "value" + number, val_str);
  //saveValues ();
  sendSTATUS("value" + number, val_str);
}

void valueDownSet() {
  String number = sCmd.next();
  float val = jsonRead(configJson, "value" + number).toFloat();
  float step_ = jsonRead(configJson, "value" + number + "step").toFloat();
  int digits = jsonRead(configJson, "value" + number + "digits").toInt();
  val = val - step_;
  String val_str = String(val);
  val_str = selectToMarkerPlus (val_str, ".", digits);
  jsonWrite(configJson, "value" + number, val_str);
  //jsonWrite(valuesJson, "value" + number, val_str);
  //saveValues ();
  sendSTATUS("value" + number, val_str);
}

//=====================================================================================================================================
//=========================================Добавление текстового виджета============================================================
void text() {

  String number = sCmd.next();
  String viget_name = sCmd.next();
  String page_name = sCmd.next();
  String page_number = sCmd.next();

  createViget (viget_name, page_name, page_number, "vigets/viget.alertsm.json", "textSet" + number);
}


void textSet() {

  String number = sCmd.next();
  String text = sCmd.next();
  text.replace("_", " ");

  if (text.indexOf("-time") >= 0) {
    text.replace("-time", "");
    String time = GetTime();
    time.replace(":", ".");
    text = GetDataDigital() + " " + time + " " + text;
  }

  jsonWrite(configJson, "textSet" + number, text);
  sendSTATUS("textSet" + number, text);
}

//=================================================Глобальные команды удаленного управления===========================================================

void mqttOrderSend() {

  String id = sCmd.next();
  String order = sCmd.next();

  String  all_line = prefix + "/" + id + "/order";
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
  jsonWrite(viget, "id", page_number);//"pageId"
  jsonWrite(viget, "descr", viget_name);
  jsonWrite(viget, "topic", prex + "/" + topic);
  all_vigets += viget + "\r\n";
  viget = "";
}
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
