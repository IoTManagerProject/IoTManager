#include "Consts.h"
#ifdef EnableSensorDallas
#include "items/vSensorDallas.h"
#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "DallasTemperature.h"
#include "Utils/StringUtils.h"

#include <Arduino.h>

//ИНТЕГРИРУЮ: переменные необходимые для работы интегрируемой библиотеки. Аналог Arduino
OneWire* oneWire;
DallasTemperature sensors;

//ИНТЕГРИРУЮ:
//Для каждого датчика указанного в конфигурации вызывается конструктор для настройки перед запуском. Аналог функции setup() в Arduino.
//В параметрах передаются дополнительные настройки, указанные все в той же таблице настройки устройства.
SensorDallas::SensorDallas(unsigned long interval, unsigned int pin, unsigned int index, String addr, String key) {
    //все особые параметры сливаем в локальные переменные экземпляра для будущего доступа
    _interval = interval * 1000;
    _key = key;
    _pin = pin;
    _index = index;
    _addr = addr;

    //ИНТЕГРИРУЮ: 
    //вызываем необходимые инициирующие функции интегрируемой библиотеки
    oneWire = new OneWire((uint8_t)_pin);
    sensors.setOneWire(oneWire);
    sensors.begin();
    sensors.setResolution(12);
}

//ИНТЕГРИРУЮ: оставляем как есть или развиваем, если нужно правильно завершить работу с интегрируемой библиотекой после отключения датчика
SensorDallas::~SensorDallas() {}

//ИНТЕГРИРУЮ: аналог loop() в Arduino. Требуется изменить, если интегрируемая библиотека нуждается в другом алгоритме квотирования времени.
void SensorDallas::loop() {
    currentMillis = millis();
    difference = currentMillis - prevMillis;
    if (difference >= _interval) {
        prevMillis = millis();
        readDallas();
    }
}

//ИНТЕГРИРУЮ: вызывается из цикла loop каждый установленный временно интервал в параметрах датчика. Необходимо изменить для чтения данных из датчика.
void SensorDallas::readDallas() {
    //запускаем опрос измерений у всех датчиков на линии
    sensors.requestTemperatures();
    
    //Определяем адрес. Если парамтер addr не установлен, то узнаем адрес по индексу
    DeviceAddress deviceAddress;
    if (_addr == "") {
        sensors.getAddress(deviceAddress, _index);
    } else {
        string2hex(_addr.c_str(), deviceAddress);
    }

    //получаем температуру по адресу
    float value = sensors.getTempC(deviceAddress);
    
    //ИНТЕГРИРУЮ: блок генерации уведомлений в ядре системы. Стоит обратить внимание только на формат выводимого сообщения в консоли. 
    eventGen2(_key, String(value));
    jsonWriteStr(configLiveJson, _key, String(value));
    publishStatus(_key, String(value));
    char addrStr[20] = "";
    hex2string(deviceAddress, 8, addrStr); 
    SerialPrint("I", "Sensor", "'" + _key + "' data: " + String(value) + "' addr: " + String(addrStr));
}

//ИНТЕГРИРУЮ: глобальная переменная необходима для интеграции в ядро. Следим за наименованием.
MySensorDallasVector* mySensorDallas2 = nullptr;

//ИНТЕГРИРУЮ: функция вызывается ядром для каждой записи в таблице настроки для создания экземпляров датчиков
//некоторые датчики записаны в таблице в виде нескольких строк, поэтому необходимо контролировать итерации обращения к данной функции
void dallas() {
    //ИНТЕГРИРУЮ: не меняем
    myLineParsing.update();
    //ИНТЕГРИРУЮ: устанавливаем в соответствии с параметрами таблицы
    String interval = myLineParsing.gint();
    String pin = myLineParsing.gpin();
    String index = myLineParsing.gindex();
    String key = myLineParsing.gkey();
    String addr = myLineParsing.gaddr();
    //ИНТЕГРИРУЮ: не меняем
    myLineParsing.clear();

    //ИНТЕГРИРУЮ: блок создания экземпляров датчиков. Обратить внимание на наименования и передаваемые параметры в конструктор
    static bool firstTime = true;
    if (firstTime) mySensorDallas2 = new MySensorDallasVector();
    firstTime = false;
    mySensorDallas2->push_back(SensorDallas(interval.toInt(), pin.toInt(), index.toInt(), addr, key));
}
#endif
