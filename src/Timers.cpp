#include "Global.h"

#include "Events.h"

void timer_countdown() {
    String timers = options.read("timers");
    if (timers.isEmpty()) {
        return;
    }
    size_t i = 0;
    do {
        Serial.println(timers);
        String timer = selectFromMarkerToMarker(timers, ",", i);
        Serial.print("timer no " + String(i) + ": ");
        Serial.println(timer);
        if (timer == "not found" || timer == "") {
            return;
        }
        int number = selectToMarker(timer, ":").toInt();
        int time = readTimer(number);
        if (time == 0) {
            delTimer(String(number));
            liveData.write("timer" + String(number), "0");
            Events::fire("timer", String(number));
        } else {
            time--;
            addTimer(String(number), String(time));
        }
        i++;
    } while (i <= 9);
}

void timer_countdown_init() {
    ts.add(
        TIMER_COUNTDOWN, 1000, [&](void*) {
            timer_countdown();
        },
        nullptr, true);
}


void addTimer(String number, String time) {
    String tmp = options.read("timers");  //1:60,2:120,
    String new_timer = number + ":" + time;
    int psn1 = tmp.indexOf(number + ":");          //0  ищем позицию таймера который надо заменить
    if (psn1 != -1) {                              //если он есть
        int psn2 = tmp.indexOf(",", psn1);         //4    от этой позиции находим позицию запятой
        String timer = tmp.substring(psn1, psn2);  //1:60  выделяем таймер который надо заменить
        ///tmp.replace(timer, new_timer);            //заменяем таймер на новый (во всей стороке)
        tmp.replace(timer + ",", "");
        tmp += new_timer + ",";
    } else {  //если его нет
        tmp += new_timer + ",";
    }
    options.write("timers", tmp);
}

void delTimer(String number) {
    String tmp = options.read("timers");                 //1:60,2:120,
    int psn1 = tmp.indexOf(number + ":");                //0  ищем позицию таймера который надо удалить
    if (psn1 != -1) {                                    //если он есть
        int psn2 = tmp.indexOf(",", psn1);               //4    от этой позиции находим позицию запятой
        String timer = tmp.substring(psn1, psn2) + ",";  //1:60,  выделяем таймер который надо удалить
        tmp.replace(timer, "");                          //удаляем таймер
        options.write("timers", tmp);
    }
}

int readTimer(int number) {
    String tmp = options.read("timers");           //1:60,2:120,
    int psn1 = tmp.indexOf(String(number) + ":");  //0  ищем позицию таймера который надо прочитать
    String timer;
    if (psn1 != -1) {                       //если он есть
        int psn2 = tmp.indexOf(",", psn1);  //4    от этой позиции находим позицию запятой
        timer = tmp.substring(psn1, psn2);  //1:60  выделяем таймер который надо прочитать
        timer = deleteBeforeDelimiter(timer, ":");
    }
    return timer.toInt();
}