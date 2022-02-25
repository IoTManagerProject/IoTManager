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
    if (jsonRead(parameters, F("val"), valAsStr, false))    // значение переменной или датчика при инициализации если есть в конфигурации
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
IoTItem::~IoTItem() {}

String IoTItem::getSubtype() {
    return _subtype;
}

String IoTItem::getID() {
    return _id;
};

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
    jsonWriteStr(paramsHeapJson, _id, value);
    publishStatusMqtt(_id, value);
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

IoTItem* findIoTItem(String name) {  // поиск элемента модуля в существующей конфигурации
    for (unsigned int i = 0; i < IoTItems.size(); i++) {
        if (IoTItems[i]->getID() == name) return IoTItems[i];
    }

    return nullptr;
}

IoTItem* myIoTItem;

IoTGpio* IoTItem::getGpioDriver() {
    return nullptr;
}

void IoTItem::setValue(IoTValue Value) {
    value = Value;
    if (value.isDecimal) regEvent(value.valD, "");
    else regEvent(value.valS, "");
}


externalVariable::externalVariable(String parameters) : IoTItem(parameters) {
    Serial.printf("Call from  externalVariable: parameters %s\n", parameters.c_str());
}

externalVariable::~externalVariable() {
    Serial.printf("Call from  ~externalVariable: Im dead\n");
}

void externalVariable::doByInterval() {   // для данного класса doByInterval+int выполняет роль счетчика обратного отсчета до уничтожения
    iAmDead = true;
}