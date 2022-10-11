#include "EventsAndOrders.h"
#include "classes/IoTScenario.h"

extern IoTScenario iotScen;  // объект управления сценарием

// orderBuf - буфер прямых команд, которые нужно выпонить сразу, нажали на кнопку в приложении
//необходимо тут же изменить параметр, записать новое значение

// eventBuf - буфер событий, в нем как на конвеере едут события, и каждое событие проверяется через все условия сценариев
//его главное отличие от orderBuf в том что события в нем не для выполнения, а для проверки в условиях сценариев
//события в нем лишь вызывают другие команды, помещенные в блоке сценария, если условие совпало.

//эта функция будет вызываться:
// 1.на приеме сообщений в mqtt (см mqttCallback строку 142 в mqttClient.cpp)
// 2.на приеме сообщений в ws
// 3.в сценарях когда надо выполнить команды
void generateOrder(const String& id, const String& value) {
    String event = id + " " + value + ",";
    orderBuf += event;
}

//эта функция будет вызываться в loop()
void handleOrder() {
    if (orderBuf.length()) {
        String order = selectToMarker(orderBuf, ",");
        SerialPrint("i", F("ORDER"), order);

        String id = selectToMarker(order, " ");

        //здесь нужно перебрать все методы execute всех векторов и выполнить те id которых совпали с id события
        IoTItem* item = findIoTItem(id);
        if (item) {
            SerialPrint("i", F("ORDER"), "order found " + order);
            String valStr = selectToMarkerLast(order, " ");
            item->setValue(valStr);
        }

        orderBuf = deleteBeforeDelimiter(orderBuf, ",");
    }
}

//эта функция будет вызываться:
// 1.в местах где событие только что произошло (IoTItem::regEvent)
void generateEvent(const String& id, const String& value) {
    String event = id + " " + value + ",";
    eventBuf += event;
}

//эта функция будет вызываться в loop()
void handleEvent() {
    if (eventBuf.length()) {
        String event = selectToMarker(eventBuf, ",");
        SerialPrint("i", F("EVENT"), event);

        //здесь нужно пропускать данное событие через условия сценариев
        //и если оно есть в условии сценария и совподает
        iotScen.exec(selectToMarker(event, " "));

        eventBuf = deleteBeforeDelimiter(eventBuf, ",");
    }
}