#include "utils/JsonUtils.h"
#include "utils/SerialPrint.h"
#include "classes/ScenarioClass3.h"
#include "classes/IoTItem.h"
#include "WsServer.h"
#include "ESPConfiguration.h"
#include "EventsAndOrders.h"

IoTItem::IoTItem(String parameters) {
    jsonRead(parameters, F("int"), _interval);
    _interval = _interval * 1000;
    jsonRead(parameters, F("subtype"), _subtype);
    jsonRead(parameters, F("id"), _id);
    jsonRead(parameters, F("multiply"), _multiply, false);
    jsonRead(parameters, F("plus"), _plus, false);
    jsonRead(parameters, F("round"), _round, false);

    String valAsStr;
    if (jsonRead(parameters, F("val"), valAsStr, false))  // значение переменной или датчика при инициализации если есть в конфигурации
        if (value.isDecimal = isDigitDotCommaStr(valAsStr))
            value.valD = valAsStr.toFloat();
        else
            value.valS = valAsStr;

    String map;
    jsonRead(parameters, F("map"), map, false);
    if (map != "") {
        _map1 = selectFromMarkerToMarker(map, ",", 0).toInt();
        _map2 = selectFromMarkerToMarker(map, ",", 1).toInt();
        _map3 = selectFromMarkerToMarker(map, ",", 2).toInt();
        _map4 = selectFromMarkerToMarker(map, ",", 3).toInt();
    }
}

// IoTItem::~IoTItem() {}

String IoTItem::getSubtype() {
    return _subtype;
}

String IoTItem::getID() {
    return _id;
};

String IoTItem::getValue() {
    if (value.isDecimal)
        return (String)value.valD;
    else
        return value.valS;
}

void IoTItem::loop() {
    currentMillis = millis();
    difference = currentMillis - prevMillis;
    if (difference >= _interval) {
        prevMillis = millis();
        this->doByInterval();
    }
}

void IoTItem::regEvent(String value, String consoleInfo = "") {
    generateEvent(_id, value);
    publishStatusMqtt(_id, value);
    //отправка события другим устройствам в сети==============================
    if (jsonReadBool(settingsFlashJson, "mqttin")) {
        String json = "{}";
        jsonWriteStr_(json, "id", _id);
        jsonWriteStr_(json, "val", value);
        jsonWriteInt_(json, "int", _interval + 5000);  // 5 секунд про запас
        publishEvent(_id, json);
        SerialPrint("i", F("<=MQTT"), "Broadcast event: " + json);
    }
    //========================================================================
    publishStatusWs(_id, value);
    SerialPrint("i", "Sensor " + consoleInfo, "'" + _id + "' data: " + value + "'");
}

void IoTItem::regEvent(float value, String consoleInfo = "") {
    if (_multiply) value = value * _multiply;
    if (_plus) value = value + _multiply;
    if (_round != 0) {
        if (value > 0) {
            value = (int)(value * _round + 0.5F);
            value = value / _round;
        }
        if (value < 0) {
            value = (int)(value * _round - 0.5F);
            value = value / _round;
        }

        // value = (float)value / (_round ? pow(10, (int)_round) : 1);  // TODO: решить как указывать округление, количество знаков после запятой или десятые сотые ...
    }
    if (_map1 != _map2) value = map(value, _map1, _map2, _map3, _map4);

    // убираем лишние нули
    char buf[20];
    sprintf(buf, "%g", value);
    regEvent((String)buf, consoleInfo);
}

void IoTItem::doByInterval() {}

IoTValue IoTItem::execute(String command, std::vector<IoTValue>& param) { return {}; }

IoTGpio* IoTItem::getGpioDriver() {
    return nullptr;
}

void IoTItem::setValue(IoTValue Value) {
    value = Value;
    if (value.isDecimal)
        regEvent(value.valD, "");
    else
        regEvent(value.valS, "");
}

externalVariable::externalVariable(String parameters) : IoTItem(parameters) {
    prevMillis = millis();  // запоминаем текущее значение таймера для выполения doByInterval после int сек
    iAmLocal = false;       // указываем, что это сущность прилетела из сети
    Serial.printf("Call from  externalVariable: parameters %s %d\n", parameters.c_str(), _interval);
}

externalVariable::~externalVariable() {
    Serial.printf("Call from  ~externalVariable: Im dead\n");
}

void externalVariable::doByInterval() {  // для данного класса doByInterval+int выполняет роль счетчика обратного отсчета до уничтожения
    iAmDead = true;
}

IoTItem* myIoTItem;

IoTItem* findIoTItem(String name) {  // поиск элемента модуля в существующей конфигурации
    for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
        if ((*it)->getID() == name) return *it;
    }

    return nullptr;
}

StaticJsonDocument<JSON_BUFFER_SIZE> docForExport;

StaticJsonDocument<JSON_BUFFER_SIZE>* getLocalItemsAsJSON() {
    docForExport.clear();
    for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
        if ((*it)->iAmLocal) docForExport[(*it)->getID()] = (*it)->getValue();
    }

    return &docForExport;
}