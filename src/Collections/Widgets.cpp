#include "Collection/Widgets.h"

#include "Global.h"

#include "Objects/Widget.h"

static const char* MODULE = "Widgets";

namespace Widgets {
std::vector<Widget*> _list;

void createWidget(String descr, String page, String order, String templateName, String topic,
                  String name1, String param1, String name2, String param2, String name3, String param3) {
    Widget* widget = new Widget();
    if (!widget->loadTemplate(templateName.c_str())) {
        return;
    }
    descr.replace("#", " ");
    widget->write("descr", descr);

    page.replace("#", " ");
    widget->write("page", page);
    widget->writeInt("order", order);

    String prefix = runtime.read("mqtt_prefix");
    widget->write("topic", prefix + "/" + topic);

    if (!name1.isEmpty()) {
        widget->write(name1, param1);
    }
    if (!name2.isEmpty()) {
        widget->write(name2, param1);
    }
    if (!name3.isEmpty()) {
        widget->write(name3, param1);
    }

    _list.push_back(widget);
}

void createChart(String series, String page, String order, String templateName, String topic, size_t maxCount) {
    Widget* widget = new Widget();
    if (!widget->loadTemplate(templateName.c_str())) {
        return;
    }

    widget->write("page", page);
    widget->writeInt("order", order);

    series.replace("#", " ");
    widget->write("series", series);
    widget->write("maxCount", maxCount);

    String prefix = runtime.read("mqtt_prefix");
    widget->write("topic", prefix + "/" + topic);

    _list.push_back(widget);
}

void clear() {
    _list.clear();
}

void forEach(JsonHandler func) {
    for (auto item : _list) {
        if (!func(item->asJson())) break;
    }
}
}  // namespace Widgets