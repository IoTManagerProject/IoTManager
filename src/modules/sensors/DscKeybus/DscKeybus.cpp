#include "Global.h"
#include "classes/IoTItem.h"
#include "utils/SerialPrint.h"

#include <dscKeybusInterface.h>

#include <vector>

struct DscKeybusSharedContext {
    dscKeybusInterface* iface = nullptr;
    int clockPin = -1;
    int readPin = -1;
    int writePin = -1;
} dscKeybusShared;

static dscKeybusInterface* acquireDscInterface(int clockPin,
                                              int readPin,
                                              int writePin,
                                              bool virtualKeypad,
                                              bool processModule,
                                              bool hideDigits) {
    const byte normalizedWrite = writePin < 0 ? 255 : static_cast<byte>(writePin);

    if (!dscKeybusShared.iface) {
        dscKeybusShared.iface = new dscKeybusInterface(static_cast<byte>(clockPin),
                                                       static_cast<byte>(readPin),
                                                       normalizedWrite);
        dscKeybusShared.clockPin = clockPin;
        dscKeybusShared.readPin = readPin;
        dscKeybusShared.writePin = writePin;

        dscKeybusShared.iface->hideKeypadDigits = hideDigits;
        dscKeybusInterface::virtualKeypad = virtualKeypad;
        dscKeybusInterface::processModuleData = processModule;

        dscKeybusShared.iface->begin(Serial);
        dscKeybusShared.iface->resetStatus();
        SerialPrint("i", F("Sensor DscKeybus"), "interface ready");
    } else {
        if (dscKeybusShared.clockPin != clockPin || dscKeybusShared.readPin != readPin ||
            dscKeybusShared.writePin != writePin) {
            SerialPrint("E", F("Sensor DscKeybus"), "pins mismatch, reuse first init");
        }

        if (virtualKeypad) {
            dscKeybusInterface::virtualKeypad = true;
        }
        if (processModule) {
            dscKeybusInterface::processModuleData = true;
        }
        if (hideDigits) {
            dscKeybusShared.iface->hideKeypadDigits = true;
        }
    }

    return dscKeybusShared.iface;
}

class DscKeybusItem : public IoTItem {
public:
    DscKeybusItem(String parameters, const String& subtype);

    void doByInterval() override;
    void loop() override;
    IoTValue execute(String command, std::vector<IoTValue>& param) override;

private:
    dscKeybusInterface* iface();
    String makeStatus() const;
    String makeZones() const;
    String makeTrouble() const;
    String makeRaw() const;
    void publishIfChanged(const String& payload);
    int readPinFromConfig(const String& jsonValue, int fallback) const;
    byte clampPartition(int candidate) const;

private:
    int _clockPin;
    int _readPin;
    int _writePin;
    byte _partition;
    byte _writePartition;
    bool _virtualKeypad;
    bool _processModule;
    bool _hideDigits;
    String _mode;
    String _lastValue;
    bool _initialised = false;
};

DscKeybusItem::DscKeybusItem(String parameters, const String& subtype) : IoTItem(parameters) {
    _clockPin = readPinFromConfig(jsonReadStr(parameters, "clockPin"), 18);
    _readPin = readPinFromConfig(jsonReadStr(parameters, "readPin"), 19);
    _writePin = readPinFromConfig(jsonReadStr(parameters, "writePin"), 21);

    _partition = clampPartition(jsonReadInt(parameters, "partition"));
    if (_partition == 0) {
        _partition = 1;
    }

    _writePartition = clampPartition(jsonReadInt(parameters, "writePartition"));
    if (_writePartition == 0) {
        _writePartition = _partition;
    }

    _virtualKeypad = jsonReadBool(parameters, "virtualKeypad");
    _processModule = jsonReadBool(parameters, "processModule");
    _hideDigits = jsonReadBool(parameters, "hideDigits");

    _mode = jsonReadStr(parameters, "mode");
    if (_mode.length() == 0) {
        _mode = subtype;
    }
    if (_mode.length() == 0) {
        _mode = F("status");
    }
    _mode.toLowerCase();

    if (subtype.equalsIgnoreCase(F("DscKeybusZones"))) {
        _mode = F("zones");
    }

    if (_writePin < 0) {
        _virtualKeypad = false;
    }
}

dscKeybusInterface* DscKeybusItem::iface() {
    if (!_initialised) {
        dscKeybusInterface* ifacePtr = acquireDscInterface(_clockPin,
                                                           _readPin,
                                                           _writePin,
                                                           _virtualKeypad,
                                                           _processModule,
                                                           _hideDigits);
        if (!ifacePtr) {
            return nullptr;
        }
        dscKeybusInterface::writePartition = _writePartition;
        _initialised = true;
    }

    return dscKeybusShared.iface;
}

String DscKeybusItem::makeStatus() const {
    if (!dscKeybusShared.iface) {
        return F("offline");
    }

    const byte idx = clampPartition(_partition) - 1;
    std::vector<String> tokens;

    if (!dscKeybusShared.iface->keybusConnected) {
        tokens.emplace_back(F("keybus_offline"));
    } else {
        if (dscKeybusShared.iface->alarm[idx]) {
            tokens.emplace_back(F("alarm"));
        }
        if (dscKeybusShared.iface->fire[idx]) {
            tokens.emplace_back(F("fire"));
        }
        if (dscKeybusShared.iface->exitDelay[idx]) {
            tokens.emplace_back(F("exit_delay"));
        }
        if (dscKeybusShared.iface->entryDelay[idx]) {
            tokens.emplace_back(F("entry_delay"));
        }
        if (dscKeybusShared.iface->noEntryDelay[idx]) {
            tokens.emplace_back(F("no_entry_delay"));
        }
        if (dscKeybusShared.iface->armed[idx]) {
            if (dscKeybusShared.iface->armedAway[idx]) {
                tokens.emplace_back(F("armed_away"));
            } else if (dscKeybusShared.iface->armedStay[idx]) {
                tokens.emplace_back(F("armed_stay"));
            } else {
                tokens.emplace_back(F("armed"));
            }
        } else if (dscKeybusShared.iface->ready[idx]) {
            tokens.emplace_back(F("ready"));
        } else {
            tokens.emplace_back(F("not_ready"));
        }
        if (dscKeybusShared.iface->disabled[idx]) {
            tokens.emplace_back(F("disabled"));
        }
    }

    if (dscKeybusShared.iface->trouble) {
        tokens.emplace_back(F("trouble"));
    }
    if (dscKeybusShared.iface->powerTrouble) {
        tokens.emplace_back(F("ac_fail"));
    }
    if (dscKeybusShared.iface->batteryTrouble) {
        tokens.emplace_back(F("battery"));
    }

    if (tokens.empty()) {
        tokens.emplace_back(F("idle"));
    }

    String result;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) {
            result += F(", ");
        }
        result += tokens[i];
    }
    return result;
}

String DscKeybusItem::makeZones() const {
    if (!dscKeybusShared.iface) {
        return F("offline");
    }

    std::vector<int> zones;
    for (byte group = 0; group < dscZones; ++group) {
        byte mask = dscKeybusShared.iface->openZones[group];
        if (mask == 0) {
            continue;
        }
        for (byte bit = 0; bit < 8; ++bit) {
            if (mask & (1 << bit)) {
                zones.emplace_back(group * 8 + bit + 1);
            }
        }
    }

    if (zones.empty()) {
        return F("none");
    }

    String result;
    for (size_t i = 0; i < zones.size(); ++i) {
        if (i > 0) {
            result += F(", ");
        }
        result += String(zones[i]);
    }
    return result;
}

String DscKeybusItem::makeTrouble() const {
    if (!dscKeybusShared.iface) {
        return F("offline");
    }

    std::vector<String> tokens;

    if (!dscKeybusShared.iface->keybusConnected) {
        tokens.emplace_back(F("keybus"));
    }
    if (dscKeybusShared.iface->trouble) {
        tokens.emplace_back(F("panel"));
    }
    if (dscKeybusShared.iface->powerTrouble) {
        tokens.emplace_back(F("ac"));
    }
    if (dscKeybusShared.iface->batteryTrouble) {
        tokens.emplace_back(F("battery"));
    }
    if (dscKeybusShared.iface->keypadAuxAlarm) {
        tokens.emplace_back(F("aux"));
    }
    if (dscKeybusShared.iface->keypadPanicAlarm) {
        tokens.emplace_back(F("panic"));
    }
    if (dscKeybusShared.iface->keypadFireAlarm) {
        tokens.emplace_back(F("fire_keypad"));
    }

    if (tokens.empty()) {
        return F("ok");
    }

    String result;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) {
            result += F(", ");
        }
        result += tokens[i];
    }
    return result;
}

String DscKeybusItem::makeRaw() const {
    if (!dscKeybusShared.iface) {
        return F("offline");
    }

    String result;
    result.reserve(dscReadSize * 3);

    const byte size = dscKeybusShared.iface->panelByteCount + 2;
    for (byte i = 0; i < size && i < dscReadSize; ++i) {
        if (i > 0) {
            result += ' ';
        }
        const byte value = dscKeybusShared.iface->panelData[i];
        if (value < 16) {
            result += '0';
        }
        result += String(value, HEX);
    }
    result.toUpperCase();

    if (result.length() == 0) {
        return F("no data");
    }
    return result;
}

void DscKeybusItem::publishIfChanged(const String& payload) {
    if (payload == _lastValue) {
        return;
    }

    value.valS = payload;
    regEvent(value.valS, "DscKeybus");
    _lastValue = payload;
}

void DscKeybusItem::doByInterval() {
    if (!iface()) {
        return;
    }

    if (_mode == F("zones")) {
        publishIfChanged(makeZones());
    } else if (_mode == F("trouble")) {
        publishIfChanged(makeTrouble());
    } else if (_mode == F("raw")) {
        publishIfChanged(makeRaw());
    } else {
        publishIfChanged(makeStatus());
    }
}

void DscKeybusItem::loop() {
    dscKeybusInterface* ifacePtr = iface();
    if (ifacePtr) {
        if (ifacePtr->loop()) {
            if (_mode == F("zones")) {
                if (ifacePtr->openZonesStatusChanged || ifacePtr->alarmZonesStatusChanged) {
                    publishIfChanged(makeZones());
                }
            } else if (_mode == F("trouble")) {
                if (ifacePtr->troubleChanged || ifacePtr->powerChanged || ifacePtr->batteryChanged ||
                    ifacePtr->keypadAuxAlarm || ifacePtr->keypadFireAlarm || ifacePtr->keypadPanicAlarm ||
                    ifacePtr->keybusChanged) {
                    publishIfChanged(makeTrouble());
                }
            } else if (_mode == F("raw")) {
                publishIfChanged(makeRaw());
            } else {
                if (ifacePtr->statusChanged || ifacePtr->keybusChanged) {
                    publishIfChanged(makeStatus());
                }
            }
        }
    }

    IoTItem::loop();
}

IoTValue DscKeybusItem::execute(String command, std::vector<IoTValue>& param) {
    IoTValue ret;
    dscKeybusInterface* ifacePtr = iface();
    if (!ifacePtr) {
        return ret;
    }

    command.toLowerCase();

    if (command == F("write")) {
        if (!_virtualKeypad || _writePin < 0) {
            SerialPrint("E", F("Sensor DscKeybus"), "virtual keypad disabled");
            return ret;
        }
        if (param.empty()) {
            return ret;
        }

        String payload = param[0].valS;
        if (payload.length() == 0) {
            payload = String(param[0].valD);
        }
        payload.trim();
        if (payload.length() == 0) {
            return ret;
        }

        dscKeybusInterface::writePartition = _writePartition;
        ifacePtr->write(payload.c_str(), true);
        SerialPrint("i", F("Sensor DscKeybus"), "keys sent");
        return ret;
    }

    if (command == F("resetstatus")) {
        ifacePtr->resetStatus();
        publishIfChanged(makeStatus());
        return ret;
    }

    if (command == F("setpartition") && !param.empty()) {
        int value = param[0].valS.length() ? param[0].valS.toInt() : static_cast<int>(param[0].valD);
        byte candidate = clampPartition(value);
        if (candidate >= 1) {
            _partition = candidate;
            publishIfChanged(makeStatus());
        }
        return ret;
    }

    if (command == F("setwritepartition") && !param.empty()) {
        int value = param[0].valS.length() ? param[0].valS.toInt() : static_cast<int>(param[0].valD);
        byte candidate = clampPartition(value);
        if (candidate >= 1) {
            _writePartition = candidate;
            dscKeybusInterface::writePartition = _writePartition;
        }
        return ret;
    }

    return ret;
}

int DscKeybusItem::readPinFromConfig(const String& jsonValue, int fallback) const {
    if (jsonValue.length() == 0 || jsonValue.equalsIgnoreCase(F("null"))) {
        return fallback;
    }
    return jsonValue.toInt();
}

byte DscKeybusItem::clampPartition(int candidate) const {
    if (candidate < 1) {
        return 0;
    }
    if (candidate > dscPartitions) {
        return dscPartitions;
    }
    return static_cast<byte>(candidate);
}

void* getAPI_DscKeybus(String subtype, String param) {
    return new DscKeybusItem(param, subtype);
}
