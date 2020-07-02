#pragma once

#include "Global.h"

namespace Logger {
void deleteOldDataTask(LoggerLog_t log);
size_t add(String name);
void deleteOldData(const String filename, size_t max_lines_cnt, String payload);
void init();
void logging();
void clear();
void choose_log_date_and_send();
}  // namespace Logger
