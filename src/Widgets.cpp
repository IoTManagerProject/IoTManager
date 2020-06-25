#include "Global.h"

static const char* MODULE = "Widget";

const String getWidgetFile(const String& name);

bool loadWidget(const String& filename, String& buf) {
    buf = readFile(getWidgetFile(filename), 2048);
    bool res = !(buf == "Failed" || buf == "Large");
    if (!res) {
        pm.error("on load" + filename);
    }
    return res;
}

void createWidget(String widget, String page, String pageNumber, String filename, String topic) {
    String buf = "{}";
    if (!loadWidget(filename, buf)) {
        pm.error("failed " + widget);
        return;
    }
    widget.replace("#", " ");
    page.replace("#", " ");

    jsonWriteStr(buf, "page", page);
    jsonWriteStr(buf, "order", pageNumber);
    jsonWriteStr(buf, "descr", widget);
    jsonWriteStr(buf, "topic", prex + "/" + topic);

#ifdef LAYOUT_IN_RAM
    all_widgets += widget + "\r\n";
#else
    addFile("layout.txt", buf);
#endif
}

void createWidgetParam(String widget, String page, String pageNumber, String filename, String topic, String name1, String param1, String name2, String param2, String name3, String param3) {
    String buf = "";
    if (!loadWidget(filename, buf)) {
        return;
    }

    widget.replace("#", " ");
    page.replace("#", " ");

    jsonWriteStr(buf, "page", page);
    jsonWriteStr(buf, "order", pageNumber);
    jsonWriteStr(buf, "descr", widget);
    jsonWriteStr(buf, "topic", prex + "/" + topic);

    if (name1) jsonWriteStr(buf, name1, param1);
    if (name2) jsonWriteStr(buf, name2, param2);
    if (name3) jsonWriteStr(buf, name3, param3);

#ifdef LAYOUT_IN_RAM
    all_widgets += widget + "\r\n";
#else
    addFile("layout.txt", buf);
#endif
}

void createChart(String widget, String page, String pageNumber, String filename, String topic, String maxCount) {
    String buf = "";
    if (!loadWidget(filename, buf)) {
        return;
    }

    widget.replace("#", " ");
    page.replace("#", " ");

    jsonWriteStr(buf, "page", page);
    jsonWriteStr(buf, "order", pageNumber);
    //jsonWriteStr(widget, "descr", widget_name);
    jsonWriteStr(buf, "series", widget);
    jsonWriteStr(buf, "maxCount", maxCount);
    jsonWriteStr(buf, "topic", prex + "/" + topic);

#ifdef LAYOUT_IN_RAM
    all_widgets += widget + "\r\n";
#else
    addFile("layout.txt", buf);
#endif
}

void createWidgetByType(String widget, String page, String pageNumber, String type, String topic) {
    pm.info("create" + type);
    createWidget(widget, page, pageNumber, getWidgetFile(type), topic);
}

const String getWidgetFile(const String& name) {
    pm.info("get " + name);
    return "/widgets/" + name + ".json";
}
