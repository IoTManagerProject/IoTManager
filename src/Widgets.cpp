#include "Global.h"

static const char* MODULE = "Widget";

const String getWidgetFile(const String& name);

bool loadWidget(const String& filename, String& buf) {
    buf = readFile(getWidgetFile(filename), 2048);
    bool res = !(buf == "failed" || buf == "large");
    if (!res) {
        pm.error("on load " + filename);
    }
    return res;
}

void createWidget(String descr, String page, String order, String filename, String topic,
                  String name1, String param1, String name2, String param2, String name3, String param3) {
    String buf = "";
    if (!loadWidget(filename, buf)) {
        return;
    }
    page.replace("#", " ");
    descr.replace("#", " ");

    jsonWriteStr(buf, "page", page);
    jsonWriteStr(buf, "order", order);
    jsonWriteStr(buf, "descr", descr);
    jsonWriteStr(buf, "topic", prex + "/" + topic);

    if (name1.length()) jsonWriteStr(buf, name1, param1);
    if (name2.length()) jsonWriteStr(buf, name2, param2);
    if (name3.length()) jsonWriteStr(buf, name3, param3);

#ifdef LAYOUT_IN_RAM
    all_widgets += widget + "\r\n";
#else
    addFile("layout.txt", buf);
#endif
}

void createChart(String widget, String page, String pageNumber, String filename, String topic,
                 String maxCount) {
    String buf = "";
    if (!loadWidget(filename, buf)) {
        return;
    }

    widget.replace("#", " ");
    page.replace("#", " ");

    jsonWriteStr(buf, "page", page);
    jsonWriteStr(buf, "order", pageNumber);
    jsonWriteStr(buf, "series", widget);
    jsonWriteStr(buf, "maxCount", maxCount);
    jsonWriteStr(buf, "topic", prex + "/" + topic);

#ifdef LAYOUT_IN_RAM
    all_widgets += widget + "\r\n";
#else
    addFile("layout.txt", buf);
#endif
}

const String getWidgetFile(const String& widget_type) {
    return "widgets/" + widget_type + ".json";
}
