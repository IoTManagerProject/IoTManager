#include "Global.h"
#include "classes/IoTItem.h"
#include "Arduino.h"
#include "MySensorsGate.h"

#ifdef MYSENSORS
// callback библиотеки mysensors
void receive(const MyMessage& message) {
    String inMsg = String(message.getSender()) + "," +   // node-id
                   String(message.getSensor()) + "," +   // child-sensor-id
                   String(message.getType()) + "," +     // type of var
                   String(message.getCommand()) + "," +  // command
                   parseToString(message) + ";";         // value

    // Serial.println("=>" + inMsg);

    mysensorBuf += inMsg;
}

String parseToString(const MyMessage& message) {
    String value = "error";
    switch (message.getPayloadType()) {
        case 0:  // Payload type is string
            value = message.getString();
            return value;
        case 1:  // Payload type is byte
            value = String(message.getByte());
            return value;
        case 2:  // Payload type is INT16
            value = String(message.getInt());
            return value;
        case 3:  // Payload type is UINT16
            value = String(message.getUInt());
            return value;
        case 4:  // Payload type is INT32
            value = String(message.getInt());
            return value;
        case 5:  // Payload type is UINT32
            value = String(message.getUInt());
            return value;
        case 6:  // Payload type is binary
            value = String(message.getBool());
            return value;
        case 7:  // Payload type is float32
            value = String(message.getFloat());
            return value;
        default:
            return value;
    }
}

#endif
class MySensorsGate : public IoTItem {
   private:
   public:
    MySensorsGate(String parameters) : IoTItem(parameters) { SerialPrint("i", "MySensors", "Gate initialized"); }

    void doByInterval() {}

    void loop() { loopMySensorsExecute(); }

    ~MySensorsGate(){};

    void loopMySensorsExecute() {
        if (mysensorBuf.length()) {
            String tmp = selectToMarker(mysensorBuf, ";");

            String nodeId = selectFromMarkerToMarker(tmp, ",", 0);         // node-id
            String childSensorId = selectFromMarkerToMarker(tmp, ",", 1);  // child-sensor-id
            String type = selectFromMarkerToMarker(tmp, ",", 2);           // type of var
            String command = selectFromMarkerToMarker(tmp, ",", 3);        // command
            String value = selectFromMarkerToMarker(tmp, ",", 4);          // value

            static bool presentBeenStarted = false;

            String ID = "n" + nodeId + "s" + childSensorId;
            static String infoJson = "{}";

            if (childSensorId == "255") {
                if (command == "3") {    // это особое внутреннее сообщение
                    if (type == "11") {  // название ноды
                        SerialPrint("i", "MySensors", "===================== " + value + " =====================");
                    }
                    if (type == "12") {  // версия ноды
                        SerialPrint("i", "MySensors", "Node version: " + value);
                    }
                }
            } else {
                if (command == "0") {  // это презентация
                    presentBeenStarted = true;
                    int num;
                    String widget;
                    String descr;
                    sensorType(type.toInt(), num, widget, descr);

                    descr.replace("#", " ");
                    SerialPrint("i", "MySensors", "Presentation: " + ID + ": " + descr);
                }
                if (command == "1") {  // это данные
                    if (value != "") {
                        if (presentBeenStarted) {
                            presentBeenStarted = false;
                            SerialPrint("i", "MySensors", "===================== " + nodeId + " =====================");
                        }

                        bool found = false;

                        for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
                            if ((*it)->getID() == ID) {
                                found = true;
                                (*it)->setValue(value, true);
                            }
                        }

                        SerialPrint("i", "MySensors", "node: " + nodeId + ", sensor: " + childSensorId + ", command: " + command + ", type: " + type + ", val: " + value + ", found: " + String(found));
                    }
                }
                if (command == "2") {  // это запрос значения переменной
                    SerialPrint("i", "MySensors", "Request a variable value");
                }
            }

            mysensorBuf = deleteBeforeDelimiter(mysensorBuf, ";");
        }
    }

    void sensorType(int index, int& num, String& widget, String& descr) {
        switch (index) {
            case 0:
                descr = F("Door#and#window#sensors");
                widget = F("alarm");
                num = 1;
                break;
            case 1:
                descr = F("Motion#sensors");
                widget = F("alarm");
                num = 1;
                break;
            case 2:
                descr = F("Smoke#sensor");
                widget = F("fillgauge");
                num = 1;
                break;
            case 3:
                descr = F("Binary#device#(on/off)");
                widget = F("toggleBtn");
                num = 2;
                break;
            case 4:
                descr = F("Dimmable#device");
                // to do
                // widget = F("range");
                // num = 2;
                break;
            case 5:
                descr = F("Window#covers#or#shades");
                // to do
                // widget = F("range");
                // num = 2;
                break;
            case 6:
                descr = F("Temperature#sensor");
                widget = F("anydataTemp");
                num = 1;
                break;
            case 7:
                descr = F("Humidity#sensor");
                widget = F("anydataHum");
                num = 1;
                break;
            case 8:
                descr = F("Pressure#sensor");
                widget = F("anydataPress");
                num = 1;
                break;
            case 9:
                descr = F("Wind#sensor");
                widget = F("anydataTime");
                num = 1;
                break;
            case 10:
                descr = F("Rain#sensor");
                widget = F("anydataTime");
                num = 1;
                break;
            case 11:
                descr = F("UV#sensor");
                widget = F("anydataTime");
                num = 1;
                break;
            case 12:
                descr = F("Weight#sensor");
                widget = F("anydataTime");
                num = 1;
                break;
            case 13:
                descr = F("Power#measuring#device");
                widget = F("anydataWtt");
                num = 1;
                break;
            case 14:
                descr = F("Heater#device");
                widget = F("anydataTemp");
                num = 1;
                break;
            case 15:
                descr = F("Distance#sensor");
                widget = F("anydata");
                num = 1;
                break;
            case 16:
                descr = F("Light#sensor");
                widget = F("anydataTime");
                num = 1;
                break;
            case 17:
                descr = F("Arduino#node#device");
                widget = F("anydata");
                num = 1;
                break;
            case 18:
                descr = F("Arduino#repeating#node#device");
                widget = F("anydata");
                num = 1;
                break;
            case 19:
                descr = F("Lock#device");
                widget = F("toggleBtn");
                num = 2;
                break;
            case 20:
                descr = F("Ir#sender/receiver#device");
                widget = F("toggleBtn");
                num = 2;
                break;
            case 21:
                descr = F("Water#meter");
                widget = F("anydata");
                num = 1;
                break;
            case 22:
                descr = F("Air#quality#sensor");
                widget = F("anydata");
                num = 1;
                break;
            case 23:
                descr = F("Custom#sensor");
                widget = F("anydata");
                num = 1;
                break;
            case 24:
                descr = F("Dust#level#sensor");
                widget = F("anydata");
                num = 1;
                break;
            case 25:
                descr = F("Scene#controller#device");
                widget = F("anydata");
                num = 1;
                break;
            case 26:
                descr = F("RGB#light");
                widget = F("anydata");
                num = 1;
                break;
            case 27:
                descr = F("RGBW#light#(with#separate#white#component)");
                widget = F("anydata");
                num = 1;
                break;
            case 28:
                descr = F("Color#sensor");
                widget = F("anydata");
                num = 1;
                break;
            case 29:
                descr = F("Thermostat/HVAC#device");
                widget = F("anydata");
                num = 1;
                break;
            case 30:
                descr = F("Multimeter#device");
                widget = F("anydataVlt");
                num = 1;
                break;
            case 31:
                descr = F("Sprinkler#device");
                widget = F("anydata");
                num = 1;
                break;
            case 32:
                descr = F("Water#leak#sensor");
                widget = F("alarm");
                num = 1;
                break;
            case 33:
                descr = F("Sound#sensor");
                widget = F("anydata");
                num = 1;
                break;
            case 34:
                descr = F("Vibration#sensor");
                widget = F("anydata");
                num = 1;
                break;
            case 35:
                descr = F("Moisture#sensor");
                widget = F("anydata");
                num = 1;
                break;
            case 36:
                descr = F("LCD#text#device");
                widget = F("anydata");
                num = 1;
                break;
            case 37:
                descr = F("Gas#meter");
                widget = F("anydata");
                num = 1;
                break;
            case 38:
                descr = F("GPS#Sensor");
                widget = F("anydata");
                num = 1;
                break;
            case 39:
                descr = F("Water#quality#sensor");
                widget = F("anydata");
                num = 1;
                break;
            default:
                descr = F("Unknown");
                widget = F("anydata");
                num = 1;
                break;
        }
    }
};

class MySensorsNode : public IoTItem {
   private:
    String id = "";
    int orange = 0;
    int red = 0;
    int offline = 0;
    int _minutesPassed = 0;
    String json = "{}";
    bool dataFromNode = false;
    // временное решение
    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;

   public:
    MySensorsNode(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, F("id"), id);

        jsonRead(parameters, F("orange"), orange);
        jsonRead(parameters, F("red"), red);
        jsonRead(parameters, F("offline"), offline);

        dataFromNode = false;
        SerialPrint("i", "MySensors", "Node initialized");
    }

    void setValue(const IoTValue& Value, bool genEvent = true) {
        value = Value;
        regEvent(value.valD, "MySensorsNode", false, genEvent);
        _minutesPassed = 0;
        prevMillis = millis();
        dataFromNode = true;
        setNewWidgetAttributes();
    }

    void doByInterval() {
        _minutesPassed++;
        setNewWidgetAttributes();
    }

    void loop() {
        currentMillis = millis();
        difference = currentMillis - prevMillis;
        if (difference > 60000) {
            prevMillis = millis();
            this->doByInterval();
        }
    }

    // событие когда пользователь подключается приложением или веб интерфейсом к усройству
    void onMqttWsAppConnectEvent() { setNewWidgetAttributes(); }

    void setNewWidgetAttributes() {
        if (dataFromNode) {
            jsonWriteStr(json, F("info"), prettyMinutsTimeout(_minutesPassed));
            if (orange != 0 && red != 0 && offline != 0) {
                if (_minutesPassed < orange) {
                    jsonWriteStr(json, F("color"), "");
                }
                if (_minutesPassed >= orange && _minutesPassed < red) {
                    jsonWriteStr(json, F("color"), F("orange"));  // сделаем виджет оранжевым
                }
                if (_minutesPassed >= red && _minutesPassed < offline) {
                    jsonWriteStr(json, F("color"), F("red"));  // сделаем виджет красным
                }
                if (_minutesPassed >= offline) {
                    jsonWriteStr(json, F("info"), F("offline"));
                }
            }
        } else {
            jsonWriteStr(json, F("info"), F("awaiting"));
        }
        sendSubWidgetsValues(id, json);
    }

    ~MySensorsNode(){};
};

void* getAPI_MySensorsGate(String subtype, String param) {
    if (subtype == F("MySensorsGate")) {
        return new MySensorsGate(param);
    } else if (subtype == F("MySensorsNode")) {
        return new MySensorsNode(param);
    } else {
        return nullptr;
    }
}