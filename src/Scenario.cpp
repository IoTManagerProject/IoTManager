#include "Scenario.h"

#include "Global.h"
#include "Cmd.h"

static const char* MODULE = "Scenario";

LiveParam::LiveParam(const String& name) : _name{name} {};

const String LiveParam::value() {
    return liveData.read(_name);
}

StaticParam::StaticParam(const String& value) : _value{value} {};

const String StaticParam::value() {
    return _value;
}

ScenarioItem::ScenarioItem(const String& str) {
    size_t split = str.indexOf("\n");
    String first_line = str.substring(0, split);
    _commands = str.substring(split + 1, str.indexOf("\nend"));
    _valid = parseCondition(first_line) && !_commands.isEmpty();
}

bool ScenarioItem::equation(const String& object, const String& value) {
    bool res = false;
    if (!object.equalsIgnoreCase(_obj)) {
        return res;
    }
    switch (_sign) {
        case OP_EQUAL:
            res = value.equals(_param->value());
            break;
        case OP_NOT_EQUAL:
            res = !value.equals(_param->value());
            break;
        case OP_LESS:
            res = value.toFloat() < _param->value().toFloat();
            break;
        case OP_LESS_OR_EQAL:
            res = value.toFloat() <= _param->value().toFloat();
            break;
        case OP_GREATER:
            res = value.toFloat() > _param->value().toFloat();
            break;
        case OP_GREATER_OR_EQAL:
            res = value.toFloat() >= _param->value().toFloat();
            break;
        default:
            break;
    }
    return res;
}

bool ScenarioItem::run(const String& event) {
    String obj = event;
    String value = liveData.read(obj);
    if (equation(obj, value)) {
        stringExecute(_commands);
        return true;
    }
    return false;
}

bool ScenarioItem::enable(bool value) {
    _enabled = value;
    return isEnabled();
}

bool ScenarioItem::isEnabled() {
    return _valid & _enabled;
}

bool ScenarioItem::parseCondition(const String& str) {
    size_t s1 = str.indexOf(" ");
    size_t s2 = str.indexOf(" ", s1 + 1);
    _obj = str.substring(0, s1);
    if (_obj.isEmpty()) {
        pm.error("wrong obj");
        return false;
    }
    String signStr = str.substring(s1 + 1, s2);
    if (!parseSign(signStr, _sign)) {
        pm.error("wrong sign");
        return false;
    };
    String paramStr = str.substring(s2 + 1);
    if (paramStr.isEmpty()) {
        pm.error("wrong param");
        return false;
    }
    if (paramStr.startsWith("digit") || paramStr.startsWith("time")) {
        _param = new LiveParam(paramStr);
    } else {
        _param = new StaticParam(paramStr);
    }
    return true;
}

bool ScenarioItem::parseSign(const String& str, EquationSign& sign) {
    bool res = true;
    if (str.equals("=")) {
        sign = OP_EQUAL;
    } else if (str.equals("!=")) {
        sign = OP_NOT_EQUAL;
    } else if (str.equals("<")) {
        sign = OP_LESS;
    } else if (str.equals(">")) {
        sign = OP_GREATER;
    } else if (str.equals(">=")) {
        sign = OP_GREATER_OR_EQAL;
    } else if (str.equals("<=")) {
        sign = OP_LESS_OR_EQAL;
    } else {
        res = false;
    }
    return res;
}

namespace Scenario {

std::vector<ScenarioItem*> _items;

StringQueue _events;

bool _ready = false;

void fire(Named* obj) {
    fire(obj->getName());
}

void fire(const String& name) {
    if (config.general()->isScenarioEnabled()) {
        _events.push(name);
    }
}

bool isBlockEnabled(size_t num) {
    return _items.at(num)->isEnabled();
}

void enableBlock(size_t num, boolean value) {
    _items.at(num)->enable(value);
}

void reinit() {
    _ready = false;
}

const String removeComments(const String buf) {
    String res = "";
    size_t startIndex = 0;
    while (startIndex < buf.length() - 1) {
        size_t endIndex = buf.indexOf("\n", startIndex);
        String line = buf.substring(startIndex, endIndex);
        startIndex = endIndex + 1;
        if (line.startsWith("//") || line.isEmpty()) {
            continue;
        }
        res += line + "\n";
    }
    return res;
}

void init() {
    _items.clear();
    String buf;
    if (!readFile(DEVICE_SCENARIO_FILE, buf)) {
        config.general()->enableScenario(false);
        return;
    }
    buf += "\n";
    buf.replace("\r\n", "\n");
    buf.replace("\r", "\n");

    buf = removeComments(buf);

    while (!buf.isEmpty()) {
        String block = selectToMarker(buf, "end");
        if (block.isEmpty()) {
            continue;
        }
        _items.push_back(new ScenarioItem(block));
        buf = deleteBeforeDelimiter(buf, "end\n");
    }
    pm.info("items: " + String(_items.size(), DEC));
    _ready = true;
}  // namespace Scenario

void loop() {
    if (!_ready) {
        init();
        _ready = true;
    }
    if (!_events.available()) {
        return;
    }
    String buf;
    _events.pop(buf);
    if (buf.isEmpty()) {
        return;
    }
    for (auto item : _items) {
        if (item->isEnabled()) {
            item->run(buf);
        }
    }
}

}  // namespace Scenario