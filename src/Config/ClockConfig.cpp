#include "Config/ClockConfig.h"

static const char* TAG_NTP = "ntp";
static const char* TAG_TIMEZONE = "timezone";

ClockConfig::ClockConfig() {
    // TODO Defaults
}

void ClockConfig::setNtp(const String value) {
    if (!_ntp.equals(value)) {
        _ntp = value;
        setChanged();
    }
}

void ClockConfig::setTimezone(const int value) {
    if (_timezone != value) {
        _timezone = value;
        setChanged();
    }
}

String ClockConfig::getNtp() const {
    return _ntp;
}

int ClockConfig::getTimezone() const {
    return _timezone;
}

void ClockConfig::load(const JsonObject& root) {
    _ntp = root[TAG_NTP].as<String>();
    _timezone = root[TAG_TIMEZONE];
};

void ClockConfig::save(JsonObject& root) {
    root[TAG_NTP] = _ntp.c_str();
    root[TAG_TIMEZONE] = _timezone;
}

bool ClockConfig::setParamByName(const String& param, const String& value) {
    bool handled = true;
    if (param.equals(TAG_TIMEZONE)) {
        setTimezone(value.toInt());
    } else if (param.equals(TAG_NTP)) {
        setNtp(value);
    } else {
        handled = false;
    }
    return handled;
};
