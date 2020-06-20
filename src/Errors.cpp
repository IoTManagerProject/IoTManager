#include "Errors.h"

static const char *str_info = "Info";
static const char *str_warn = "Warn";
static const char *str_error = "Error";
static const char *str_unknown = "Unknown";

String getErrorLevelStr(ErrorLevel_t level) {
    const char *ptr;
    switch (level) {
        case EL_INFO:
            ptr = str_info;
            break;
        case EL_WARNING:
            ptr = str_warn;
            break;
        case EL_ERROR:
            ptr = str_error;
            break;
        default:
            ptr = str_unknown;
            break;
    }
    return String(ptr);
}