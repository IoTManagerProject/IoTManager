#pragma once
#include "Global.h"

extern void createWidget(String& parameters);

extern bool loadWidget(const String& filename, String& buf);

extern const String getWidgetFile(const String& name);