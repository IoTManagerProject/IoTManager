#include "Cmd.h"

#include "Collection/Pwms.h"
#include "Collection/Widgets.h"

/*
* pwm №1 подключен к PIN 12 начальное состояние 100%
pwm {id:1, descr: "Зеленый", pin:12, state:100%, page:"Лампа" order:4}
*/
void cmd_pwm() {
    KeyValueStore param = KeyValueStore(sCmd.next());

    String name = getObjectName(TAG_PWM, param.read("id").c_str());
    String assign = param.read("pin");
    String descr = param.read("name");
    String page = param.read("page");
    String state = param.read("state");
    String order = param.read("order");

    Pwm* item = (Pwm*)pwms.add(name, assign);

    item->setMap(0, 100, 0, 1023);
    item->setValue(state);

    liveData.write(name, state, VT_INT);
    Widgets::createWidget(descr, page, order, "range", name);
}

void cmd_pwmSet() {
    String name = getObjectName(TAG_PWM, sCmd.next());
    String value = sCmd.next();

    pwms.get(name)->setValue(value);

    liveData.write(name, value, VT_INT);
    Scenario::fire(name);
    MqttClient::publishStatus(name, value, VT_INT);
}