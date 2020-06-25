#include "Utils/PresetUtils.h"

static const char* config_file_fmt PROGMEM = "/conf/%s%03d.txt";

const String getConfigFile(uint8_t preset, ConfigType_t type) {
    char buf[64];
    sprintf_P(buf, config_file_fmt, (type == CT_CONFIG) ? "c" : "s", preset);
    return String(buf);
}