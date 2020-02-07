//================================================================================================================
//=========================================Таймера=================================================================
void Timer_countdown_init() {

  ts.add(TIMER_COUNTDOWN, 1000, [&](void*) {

    String old_line = jsonRead(optionJson, "timers");


    if (old_line != "") {

      Serial.println(old_line);

      int i = 0;

      do {

        String timer = selectFromMarkerToMarker(old_line, "," , i);
        // Serial.print("timer no " + String (i) + ": ");
        // Serial.println(timer);
        if (timer == "not found" || timer == "") return;
        int number = selectToMarker (timer, ":").toInt();
        int time = readTimer(number);
        if (time == 0) {

          delTimer (String (number));

          jsonWrite(configJson, "timerSet" + String(number), "0");

          eventGen ("timerSet", String(number));

        } else {
          time--;
          addTimer(String (number), String (time));
        }
        i++;
      } while (i <= 9);

    }
  }, nullptr, true);
}



void timerStart() {

  String number = sCmd.next();
  String period_of_time = sCmd.next();
  String type = sCmd.next();

  if (period_of_time.indexOf("digit") != -1) {
    period_of_time = add_set(period_of_time);
    period_of_time = jsonRead(configJson, period_of_time);
  }

  if (type == "sec") period_of_time = period_of_time;
  if (type == "min") period_of_time = String(period_of_time.toInt() * 60);
  if (type == "hours") period_of_time = String(period_of_time.toInt() * 60 * 60);

  addTimer(number, period_of_time);
  jsonWrite(configJson, "timerSet" + number, "1");
}
void addTimer(String number, String time) {

  String tmp = jsonRead(optionJson, "timers");   //1:60,2:120,
  String new_timer = number + ":" + time;
  int psn1 = tmp.indexOf(number + ":");          //0  ищем позицию таймера который надо заменить

  if (psn1 != -1) {                              //если он есть

    int psn2 = tmp.indexOf(",", psn1);           //4    от этой позиции находим позицию запятой

    String timer = tmp.substring(psn1, psn2);    //1:60  выделяем таймер который надо заменить
    ///tmp.replace(timer, new_timer);            //заменяем таймер на новый (во всей стороке)
    tmp.replace(timer + ",", "");
    tmp += new_timer + ",";

  } else {                                       //если его нет
    tmp += new_timer + ",";
  }
  jsonWrite(optionJson, "timers", tmp);
  //Serial.println("ura");
}


void timerStop() {

  String number = sCmd.next();
  delTimer(number);

}
void delTimer (String number) {

  String tmp = jsonRead(optionJson, "timers");       //1:60,2:120,
  int psn1 = tmp.indexOf(number + ":");              //0  ищем позицию таймера который надо удалить

  if (psn1 != -1) {                                  //если он есть
    int psn2 = tmp.indexOf(",", psn1);               //4    от этой позиции находим позицию запятой
    String timer = tmp.substring(psn1, psn2) + ",";  //1:60,  выделяем таймер который надо удалить
    tmp.replace(timer, "");                          //удаляем таймер
    jsonWrite(optionJson, "timers", tmp);
  }
}

int readTimer(int number) {

  String tmp = jsonRead(optionJson, "timers");       //1:60,2:120,

  int psn1 = tmp.indexOf(String(number) + ":");              //0  ищем позицию таймера который надо прочитать

  String timer;

  if (psn1 != -1) {                                  //если он есть
    int psn2 = tmp.indexOf(",", psn1);               //4    от этой позиции находим позицию запятой
    timer = tmp.substring(psn1, psn2);               //1:60  выделяем таймер который надо прочитать
    timer = deleteBeforeDelimiter(timer, ":");
  }
  return timer.toInt();
}

/*void timer() {

  String seted_time = sCmd.next();
  String order = sCmd.next();
  order.replace("_", " ");
  if (seted_time == current_time) {

    order_loop += order + ",";

  }
}*/

//------------------------------таймеры------------------------------------------------------
void time() {

  String time_number = sCmd.next();
  String time = sCmd.next();

  String time_to_add = time_number + "-" + time;

  String replace_line = jsonRead(optionJson, "times") ;
  int psn1 = replace_line.indexOf(time_number + "-") ;  //ищем позицию времени которое надо заменить

  if (psn1 != -1) {                              //если оно есть

    int psn2 = replace_line.indexOf(",", psn1);           //от этой позиции находим позицию запятой

    String timer = replace_line.substring(psn1, psn2);    //выделяем таймер который надо заменить
    ///tmp.replace(timer, new_timer);                     //заменяем таймер на новый (во всей стороке)
    replace_line.replace(timer + ",", "");
    replace_line += time_to_add + ",";

  } else {                                              //если его нет
    replace_line += time_to_add + ",";
  }


  jsonWrite(optionJson, "times", replace_line);

  jsonWrite(configJson, "timeSet" + time_number, "1");

  ts.add(TIMERS, 1000, [&](void*) {

    current_time = GetTime();
    Serial.println(current_time);

    String seted_times = jsonRead(optionJson, "times");

    while (seted_times.length() != 0) {
      String tmp = selectToMarker (seted_times, ",");

      String time_number = selectToMarker(tmp, "-");
      String seted_time = deleteBeforeDelimiter(tmp, "-");

      Serial.println(seted_time);

      if (current_time == seted_time) {
        jsonWrite(configJson, "timeSet" + time_number, "0");
        eventGen ("timeSet", time_number);
      }

      seted_times = deleteBeforeDelimiter(seted_times, ",");
    }
  }, nullptr, true);
}
