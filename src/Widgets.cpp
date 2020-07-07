#include "Global.h"

static const char* MODULE = "Widget";

void createWidget(String descr, String page, String order, String filename, String topic, String name1, String param1, String name2, String param2, String name3, String param3);
void createChart(String widget, String page, String order, String filename, String topic, String maxCount);
void clearWidgets();

const String getWidgetFile(const String& name);
void addWidget(const String& widget);
bool loadWidget(const String& filename, String& buf);

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

    addWidget(buf);
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

    addWidget(buf);
}

const String getWidgetFile(const String& widget_type) {
    return "widgets/" + widget_type + ".json";
}

void addWidget(const String& widget) {
#ifdef LAYOUT_IN_RAM
    all_widgets += widget + "\r\n";
#else
    addFile("layout.txt", widget);
#endif
}

void clearWidgets() {
#ifdef LAYOUT_IN_RAM
    all_widgets = "";
#else
    removeFile("layout.txt");
#endif
}

bool loadWidget(const String& filename, String& buf) {
    String path = getWidgetFile(filename);
    bool res = readFile(path.c_str(), buf);
    if (!res) {
        pm.error("on load " + filename);
    }
    return res;
}