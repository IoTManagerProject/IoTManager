#include "YourSensor.h"

float yourSensorReading(String type) {
    float value;
    if (type == "type1") {  // type1 - замените на название вашего датчика, потом это же название указывайте в type[], в вебе
        //сюда вставляем процедуру чтения одного датчика
        static int a;
        a++;
        value = a;
    } else if (type == "type2") {
        //сюда вставляем процедуру чтения другого датчика
        static int b;
        b--;
        value = b;
    } else if (type == "type3") {
        //сюда третьего и так далее, создавайте сколько угодно else if....
        //если у одного датчика несколько параметров то под каждый из них делайте свой else if и свое имя type
        static int c;
        c++;
        value = c * 10;
    }
    return value;
}
