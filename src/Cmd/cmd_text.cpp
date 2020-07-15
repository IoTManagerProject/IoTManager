#include "Cmd.h"

void cmd_textSet() {
    String name = sCmd.next();
    String value = sCmd.next();

    value.replace("_", " ");

    if (value.indexOf("-time") >= 0) {
        value.replace("-time", "");
        value.replace("#", " ");
        value = value + " " + now.getDateTimeDotFormated();
    }

    String objName = "text" + name;
    liveData.write(objName, value, VT_STRING);
    MqttClient::publishStatus(objName, value, VT_STRING);
}