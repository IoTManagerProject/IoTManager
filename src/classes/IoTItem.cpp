#include "utils/JsonUtils.h"
#include "utils/SerialPrint.h"
#include "classes/IoTItem.h"
#include "WsServer.h"
#include "ESPConfiguration.h"
#include "EventsAndOrders.h"

IoTItem::IoTItem(const String& parameters) {
    jsonRead(parameters, F("int"), _interval, false);
    setInterval(_interval);
    jsonRead(parameters, F("subtype"), _subtype, false);
    jsonRead(parameters, F("id"), _id);
    if (!jsonRead(parameters, F("multiply"), _multiply, false)) _multiply = 1;
    if (!jsonRead(parameters, F("plus"), _plus, false)) _plus = 0;
    if (!jsonRead(parameters, F("round"), _round, false)) _round = -1;
    if (!jsonRead(parameters, F("numDigits"), _numDigits, false)) _numDigits = 1;

    if (!jsonRead(parameters, F("global"), _global, false)) _global = false;

    String map;
    jsonRead(parameters, F("map"), map, false);
    if (map != "") {
        _map1 = selectFromMarkerToMarker(map, ",", 0).toInt();
        _map2 = selectFromMarkerToMarker(map, ",", 1).toInt();
        _map3 = selectFromMarkerToMarker(map, ",", 2).toInt();
        _map4 = selectFromMarkerToMarker(map, ",", 3).toInt();
    } else
        _map1 = _map2 = _map3 = _map4 = 0;

    String valAsStr = "";
    if (jsonRead(parameters, F("val"), valAsStr, false))  // значение переменной или датчика при инициализации если есть в конфигурации
        setValue(valAsStr, false);

    jsonRead(parameters, F("needSave"), _needSave, false);
    if (_needSave && jsonRead(valuesFlashJson, _id, valAsStr, false))  // пробуем достать из сохранения значение элемента, если указано, что нужно сохранять
        setValue(valAsStr, false);

    // проверяем нужно ли отслеживать значение другого элемента
    String trackingID = "";
    IoTItem* item = nullptr;
    if (jsonRead(parameters, F("trackingID"), trackingID, false) && (item = findIoTItem(trackingID)) != nullptr) {
        _trackingValue = &(item->value);
    }
}

void IoTItem::suspendNextDoByInt(unsigned long _delay) { // 0 - force
    nextMillis = millis() + _delay; 
}

void IoTItem::loop() {
    if (enableDoByInt) {
        unsigned long currentMillis = millis(); // _interval должен быть < 2147483647 мс (24 суток)
        if (nextMillis - currentMillis > 2147483647UL /*ULONG_MAX/2*/ ) {
            nextMillis = currentMillis + _interval;
            // SerialPrint(F("i"), _id, "this->doByInterval");
            this->doByInterval();
        }
    }
}

String IoTItem::getValue() {
    if (value.isDecimal) {
        return getRoundValue();
    } else
        return value.valS;
}

long IoTItem::getInterval() { return _interval; }

bool IoTItem::isGlobal() { return _global; }

void IoTItem::setValue(const String& valStr, bool genEvent) {
            value.isDecimal = isDigitDotCommaStr(valStr);
    
    if (value.isDecimal) {
        value.valD = valStr.toFloat();
                getRoundValue();
    } else {
                value.valS = valStr;
    }
    setValue(value, genEvent);
}

void IoTItem::setValue(const IoTValue& Value, bool genEvent) {
    value = Value; 

    if (value.isDecimal) {
        regEvent(value.valD, "", false, genEvent);
    } else {
        regEvent(value.valS, "", false, genEvent);
    }
}

// метод отправки из модуля дополнительных json полей виджета в приложение и веб интерфейс,
// необходимый для изменения виджетов "на лету" из модуля
void IoTItem::sendSubWidgetsValues(String& id, String& json) {
    publishJsonMqtt(id, json);
    publishJsonWs(id, json);
}

// когда событие случилось
void IoTItem::regEvent(const String& value, const String& consoleInfo, bool error, bool genEvent) {
        if (_needSave) {
        jsonWriteStr_(valuesFlashJson, _id, value);
        needSaveValues = true;
    }
    publishStatusMqtt(_id, value);
    publishStatusWs(_id, value);
    // SerialPrint("i", "Sensor", consoleInfo + " '" + _id + "' data: " + value + "'");

    if (genEvent) {
        generateEvent(_id, value);

        // отправка события другим устройствам в сети если не было ошибки
        if (_global && !error) {
            String json = "{}";
            jsonWriteStr_(json, "id", _id);
            jsonWriteStr_(json, "val", value);
            jsonWriteInt_(json, "int", _interval / 1000);
            publishEvent(_id, json);
            SerialPrint("i", F("<=MQTT"), "Broadcast event: " + json);
        }
    }
}

String IoTItem::getRoundValue() {
    if (!value.isDecimal) return value.valS;
    
    if (_round >= 0 && _round <= 6) {
        int sot = _round ? pow(10, (int)_round) : 1;
        value.valD = round(value.valD * sot) / sot;
        //todo: оптимизировать. Вынести расчет строки формата округления, чтоб использовать постоянно готовую
        char buf[15];
        sprintf(buf, ("%0" + (String)(_numDigits + _round) + "." + (String)_round + "f").c_str(), value.valD);
        value.valS = (String)buf;
        return value.valS;
    } else {
        value.valS = (String)value.valD;
        return value.valS;
    }
}

void IoTItem::regEvent(float regvalue, const String& consoleInfo, bool error, bool genEvent) {
        value.valD = regvalue;

    if (_multiply) value.valD = value.valD * _multiply;
    if (_plus) value.valD = value.valD + _plus;
    if (_map1 != _map2) value.valD = map(value.valD, _map1, _map2, _map3, _map4);

    regEvent(getRoundValue(), consoleInfo, error, genEvent);
}

void IoTItem::doByInterval() {}

IoTValue IoTItem::execute(String command, std::vector<IoTValue>& param) { return {}; }

String IoTItem::getSubtype() {
    return _subtype;
}

int IoTItem::getIntFromNet() {
    return _intFromNet;
}

void IoTItem::getNetEvent(String& event) {
    event = "{}";
    jsonWriteStr_(event, "id", _id);
    jsonWriteStr_(event, "val", getValue());
    jsonWriteInt_(event, "int", _interval / 1000);
}

void IoTItem::setIntFromNet(int interval) {
    _intFromNet = interval;
}

void IoTItem::checkIntFromNet() {
    // проверяем элемент на доверие данным.
    if (_intFromNet >= 0) {
        // если время жизни истекло, то удаляем элемент чуть позже на следующем такте loop
        // если это было уведомление не об ошибке или начале работы, то сообщаем, что сетевое событие давно не приходило
        if (_intFromNet == 0 && _id.indexOf("onError") == -1 && _id.indexOf("onStart") == -1 && _id.indexOf("onInit") == -1) {
            SerialPrint("E", _id, "The new data did not come from the network. The level of trust is low.", _id);
        }
        _intFromNet--;
    }
}

String IoTItem::getMqttExterSub() {
    return "";
}

// хуки для системных событий (должны начинаться с "on")
void IoTItem::onRegEvent(IoTItem* item) {}
void IoTItem::onMqttRecive(String& topic, String& msg) {}
void IoTItem::onMqttWsAppConnectEvent() {}
void IoTItem::onModuleOrder(String& key, String& value) {}
void IoTItem::onTrackingValue(IoTItem* item) {
    setValue(item->getValue(), false);
}

bool IoTItem::isTracking(IoTItem* item) {
    return &(item->value) == _trackingValue;
}

// делаем доступным модулям отправку сообщений в телеграм
void IoTItem::sendTelegramMsg(bool often, String msg) {}
void IoTItem::sendFoto(uint8_t *buf, uint32_t length, const String &name) {}
void IoTItem::editFoto(uint8_t *buf, uint32_t length, const String &name) {}
// для обновления экрана Nextion из телеграм
void IoTItem::uploadNextionTlgrm(String &url) {}

// методы для графиков (будет упрощено)
void IoTItem::publishValue() {}
void IoTItem::clearValue() {}
void IoTItem::setPublishDestination(int publishType, int wsNum){};
void IoTItem::clearHistory() {}
void IoTItem::setTodayDate() {}

String IoTItem::getID() {
    return _id;
};

bool IoTItem::isStrInID(const String& str) {
    return _id.indexOf(str) != -1; 
}

void IoTItem::setInterval(long interval) {
    if (interval == 0) enableDoByInt = false;           // выключаем использование периодического выполнения в модуле
    else {
        enableDoByInt = true;
        if (interval > 0) _interval = interval * 1000;    // если int положителен, то считаем, что получены секунды
        else if (interval < 0) _interval = interval * -1;      // если int отрицательный, то миллисекунды
    }
    // SerialPrint(F("i"), F("IoTItem"), "setInterval: " + _interval.toString);
}

IoTGpio* IoTItem::getGpioDriver() {
    return nullptr;
}

IoTItem* IoTItem::getRtcDriver() {
    return nullptr;
}
/*
IoTItem* IoTItem::getCAMDriver() {
    return nullptr;
}
*/
IoTItem* IoTItem::getTlgrmDriver() {
    return nullptr;
}

IoTBench *IoTItem::getBenchmarkTask()
{
    return nullptr;
}
IoTBench *IoTItem::getBenchmarkLoad()
{
    return nullptr;
}
unsigned long IoTItem::getRtcUnixTime()
{
    return 0;
}

// сетевое общение====================================================================================================================================

// externalVariable::externalVariable(const String& parameters) : IoTItem(parameters) {
//     prevMillis = millis();  // запоминаем текущее значение таймера для выполения doByInterval после int сек
//     iAmLocal = false;       // указываем, что это сущность прилетела из сети
//     //Serial.printf("Call from  externalVariable: parameters %s %d\n", parameters.c_str(), _interval);
// }

// externalVariable::~externalVariable() {
//     Serial.printf("Call from  ~externalVariable: Im dead\n");
// }

// void externalVariable::doByInterval() {  // для данного класса doByInterval+int выполняет роль счетчика обратного отсчета до уничтожения
//     iAmDead = true;
// }

//=========================================================================================================================================

IoTItem* myIoTItem;

// поиск элемента модуля в существующей конфигурации
IoTItem* findIoTItem(const String& name) {
    if (name == "") return nullptr;
    for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
        if ((*it)->getID() == name) return *it;
    }

    return nullptr;
}

// поиск элемента модуля в существующей конфигурации по части имени
IoTItem* findIoTItemByPartOfName(const String& partName) {
    if (partName == "") return nullptr;
    for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
        if ((*it)->isStrInID(partName)) return *it;
    }

    return nullptr;
}

// поиск плюс получение значения
String getItemValue(const String& name) {
    IoTItem* tmp = findIoTItem(name);
    if (tmp)
        return tmp->getValue();
    else
        return "";
}

// существует ли айтем
bool isItemExist(const String& name) {
    IoTItem* tmp = findIoTItem(name);
    if (tmp)
        return true;
    else
        return false;
}

// создаем временную копию элемента из сети на основе события
IoTItem* createItemFromNet(const String& itemId, const String& value, int interval) {
    String jsonStr = "{\"id\":\"";
    jsonStr += itemId;
    jsonStr += "\",\"val\":\"";
    jsonStr += value;
    jsonStr += "\",\"int\":";
    jsonStr += interval;
    jsonStr += "}";

    return createItemFromNet(jsonStr);
}

// создаем временную копию элемента из сети на основе события
IoTItem* createItemFromNet(const String& msgFromNet) {
    IoTItem* tmpp = new IoTItem(msgFromNet);

    //Serial.println("vvvvvvvvvvv " + msgFromNet + " " + (String)tmpp->getInterval());

    if (tmpp->getInterval()) tmpp->setIntFromNet(tmpp->getInterval() / 1000 + 5);
    tmpp->iAmLocal = false;
    IoTItems.push_back(tmpp);
    generateEvent(tmpp->getID(), tmpp->getValue());
    return tmpp;
}

void analyzeMsgFromNet(const String& msg, String altId) {
    if (!jsonRead(msg, F("id"), altId, altId == "")) return;  // ничего не предпринимаем, если ошибка и altId = "", вообще данная конструкция нужна для совместимости с форматом данных 3 версией
    IoTItem* itemExist = findIoTItem(altId);
    if (itemExist) {
        String valAsStr = msg;
        jsonRead(msg, F("val"), valAsStr, false);
        long interval = 0;
        jsonRead(msg, F("int"), interval, false);

        itemExist->setInterval(interval);                      // устанавливаем такой же интервал как на источнике события
        itemExist->setValue(valAsStr, false);                  // только регистрируем изменения в интерфейсе без создания цикла сетевых событий
        if (interval) itemExist->setIntFromNet(interval + 5);  // если пришедший интервал =0, значит не нужно контролировать доверие, иначе даем фору в 5 сек
        generateEvent(altId, valAsStr);
    } else {
        // временно зафиксируем данные в базе, если локально элемент отсутствует
        createItemFromNet(msg);
        //Serial.println("ffffffffff " + msg + " altId=" + altId);
    }
}

//StaticJsonDocument<JSON_BUFFER_SIZE> docForExport;

// StaticJsonDocument<JSON_BUFFER_SIZE>* getLocalItemsAsJSON() {
//     docForExport.clear();
//     for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
//         if ((*it)->iAmLocal) docForExport[(*it)->getID()] = (*it)->getValue();
//     }

//     return &docForExport;
// }