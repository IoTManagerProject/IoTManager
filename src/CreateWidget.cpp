#include "CreateWidget.h"

void createWidget(String& parameters) {
    String id = jsonReadStr(parameters, "id");
    String widget = jsonReadStr(parameters, "widget");
    String order = jsonReadStr(parameters, "order");
    String page = jsonReadStr(parameters, "page");
    String descr = jsonReadStr(parameters, "descr");

    SerialPrint(F("i"), F("Widget"), "Start create widget: " + widget);

    if (widget != "na") {
        String buf = "{}";
        if (!loadWidget(widget, buf)) {
            return;
        }
        //  if (_cnt != "") {
        //      if (widget.indexOf("chart") != -1) jsonWriteStr(buf, "maxCount", _cnt);
        //  }

#ifdef GATE_MODE
        jsonWriteStr(buf, "info", " ");
#endif

        jsonWriteStr(buf, "page", page);
        jsonWriteStr(buf, "order", order);
        jsonWriteStr(buf, "descr", descr);
        jsonWriteStr(buf, "topic", prex + "/" + id);

        SerialPrint(F("i"), F("Widget"), buf);

#ifdef LAYOUT_IN_RAM
        all_widgets += widget + "\r\n";
#else
        addFileLn("layout.json", buf);
#endif
    }
}

bool loadWidget(const String& widget, String& buf) {
    File file = seekFile("/widgets.json");
    file.find("[");
    bool ret = false;
    while (file.available()) {
        String jsonArrayElement = file.readStringUntil('}') + "}";
        if (jsonArrayElement.startsWith(",")) {
            jsonArrayElement = jsonArrayElement.substring(1, jsonArrayElement.length());  //это нужно оптимизировать в последствии
        }
        String name;
        if (!jsonRead(jsonArrayElement, F("name"), name)) {  //если нет такого ключа в представленном json или он не валидный
            SerialPrint(F("E"), F("Config"), "json error " + name);
            continue;
        } else {
            if (name == widget) {
                buf = jsonArrayElement;
                ret = true;
            }
        }
    }
    return ret;
}

const String getWidgetFile(const String& name) {
    return "/widgets/" + name + ".json";
}
