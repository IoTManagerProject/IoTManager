#include "Cmd.h"

#include "Collection/Pwms.h"
#include "Collection/Widgets.h"

/*
* pwm №1 подключен к PIN 12 начальное состояние 100%
pwm {id:1,descr:"Зеленый",pin:12,state:100%,page:"Лампа",order:4}
*/
void cmd_pwm() {
    KeyValueStore* params = new KeyValueStore(sCmd.next());

    String name = getObjectName(TAG_PWM, params->read("id").c_str());
    String assign = params->read("pin");
    String descr = params->read("name");
    String page = params->read("page");
    String state = params->read("state");
    String order = params->read("order");

    delete params;

    String styles = sCmd.next();

    Pwm* item = (Pwm*)pwms.add(name, assign);

    item->setMap(1, 100, 0, 1023);
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