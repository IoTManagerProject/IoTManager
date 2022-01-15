#include "CreateWidget.h"

void createWidget(String& parameters) {
    String id = jsonReadStr(parameters, "id");
    String widget = jsonReadStr(parameters, "widget");
    String order = jsonReadStr(parameters, "order");
    String page = jsonReadStr(parameters, "page");
    String descr = jsonReadStr(parameters, "descr");

    if (widget != "na") {
        String buf = "{}";
        if (!loadWidget(widget, buf)) {
            return;
        }
        // if (_cnt != "") {
        //     if (widget.indexOf("chart") != -1) jsonWriteStr(buf, "maxCount", _cnt);
        // }

#ifdef GATE_MODE
        jsonWriteStr(buf, "info", " ");
#endif

        jsonWriteStr(buf, "page", page);
        jsonWriteStr(buf, "order", order);
        jsonWriteStr(buf, "descr", descr);
        jsonWriteStr(buf, "topic", prex + "/" + id);

#ifdef LAYOUT_IN_RAM
        all_widgets += widget + "\r\n";
#else
        addFileLn("layout.txt", buf);
#endif
    }
}

bool loadWidget(const String& widget, String& buf) {
    buf = readFile(getWidgetFile(widget), 2048);
    bool res = !(buf == "Failed" || buf == "Large");
    if (!res) {
        SerialPrint("E", "module", "on load" + widget);
    }
    return res;
}

const String getWidgetFile(const String& name) {
    return "/widgets/" + name + ".json";
}
