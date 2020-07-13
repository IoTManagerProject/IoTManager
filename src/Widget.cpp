#include "Widget.h"

#include "Utils/PrintMessage.h"

static const char* MODULE = "Widget";

Widget::Widget() {
    _content = new KeyValueFile();
}

Widget::~Widget() {
    delete _content;
}

bool Widget::loadTemplate(const char* name) {
    _content->clear();
    if (!_content->loadFile(getFilename(name).c_str())) {
        pm.error("load template: " + String(name));
        return false;
    }
    return true;
}

void Widget::write(const String& key, const String& value) {
    _content->write(key, value, VT_STRING);
}

void Widget::writeInt(const String& key, const String& value) {
    _content->write(key, value, VT_INT);
}

void Widget::writeFloat(const String& key, const String& value) {
    _content->write(key, value, VT_FLOAT);
}

const String Widget::asJson() {
    return _content->asJson();
}

const String Widget::getFilename(const char* name) {
    String res;
    res = "widgets/";
    res += name;
    res += ".json";
    return res;
};