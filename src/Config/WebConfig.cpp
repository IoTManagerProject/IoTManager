#include "Config/WebConfig.h"

static const char* TAG_WEB_LOGIN = "weblogin";
static const char* TAG_WEB_PASS = "webpass";

WebConfig::WebConfig(){
    // TODO Defaults
};

void WebConfig::setLogin(const String str) {
    bool changed = updateField(_login, str, sizeof(_login));
    if (changed) {
        setChanged();
    }
}

void WebConfig::setPass(const String str) {
    bool changed = updateField(_pass, str, sizeof(_pass));
    if (changed) {
        setChanged();
    }
}

const String WebConfig::getLogin() const {
    return _login;
}

const String WebConfig::getPass() const {
    return _pass;
}

void WebConfig::load(const JsonObject& root) {
    strlcpy(_login, root[TAG_WEB_LOGIN], sizeof(_login));
    strlcpy(_pass, root[TAG_WEB_PASS], sizeof(_pass));
}

void WebConfig::save(JsonObject& root) {
    root[TAG_WEB_LOGIN] = _login;
    root[TAG_WEB_PASS] = _pass;
};

bool WebConfig::setParamByName(const String& param, const String& value) {
    bool handled = true;
    if (param.equals(TAG_WEB_LOGIN)) {
        setLogin(value);
    } else if (param.equals(TAG_WEB_PASS)) {
        setPass(value);
    } else {
        handled = false;
    }
    return handled;
};