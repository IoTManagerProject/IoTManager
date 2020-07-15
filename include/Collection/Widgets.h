#pragma once

#include <Arduino.h>
#include <functional>

typedef std::function<bool(String)> JsonHandler;

namespace Widgets {
void createWidget(String descr, String page, String order, String templateName, String topic, String name1 = "", String param1 = "", String name2 = "", String param2 = "", String name3 = "", String param3 = "");
void createChart(String series, String page, String order, String templateName, String topic, size_t maxCount);
void clear();
void forEach(JsonHandler func);
}  // namespace Widgets
