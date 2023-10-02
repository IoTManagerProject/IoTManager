#include "Global.h"
#include "classes/IoTItem.h"

class ExternalMQTT : public IoTItem
{
private:
    String _MAC;
    String _sensor;
    IoTItem *tmp;
    int _minutesPassed = 0;
    String json = "{}";
    int orange = 0;
    int red = 0;
    int offline = 0;
    bool dataFromNode = false;
    String _topic = "";
    bool _isJson;
    bool _addPrefix;
    bool _debug;
    bool sendOk = false;

public:
    ExternalMQTT(String parameters) : IoTItem(parameters)
    {
        _sensor = jsonReadStr(parameters, "sensor");
        jsonRead(parameters, F("orange"), orange);
        jsonRead(parameters, F("red"), red);
        jsonRead(parameters, F("offline"), offline);
        _topic = jsonReadStr(parameters, "topic");
        jsonRead(parameters, "isJson", _isJson);
        jsonRead(parameters, "addPrefix", _addPrefix);
        jsonRead(parameters, "debug", _debug);
        dataFromNode = false;
        if (mqttIsConnect())
            sendOk = true;
        mqttSubscribeExternal(_topic, _addPrefix);
    }
    char *TimeToString(unsigned long t)
    {
        static char str[12];
        long h = t / 3600;
        t = t % 3600;
        int m = t / 60;
        int s = t % 60;
        sprintf(str, "%02ld:%02d:%02d", h, m, s);
        return str;
    }
    void onMqttRecive(String &topic, String &msg)
    {
        if (msg.indexOf("HELLO") == -1)
        {
            if (_debug)
            {
                SerialPrint("i", "onMqttRecive", "Прилетело  " + topic + " msg: " + msg);
                //  SerialPrint("i", "onMqttRecive", "Прилетело  " + msg);
            }
            String dev = selectToMarkerLast(topic, "/");
            dev.toUpperCase();
            dev.replace(":", "");
            if (_topic != topic)
            {
                return;
            }

            if (_isJson)
            {
                DynamicJsonDocument doc(JSON_BUFFER_SIZE);
                DeserializationError error = deserializeJson(doc, msg);
                if (error)
                {
                    SerialPrint("E", F("onMqttRecive"), error.f_str());
                }
                JsonObject jsonObject = doc.as<JsonObject>();

                for (JsonPair kv : jsonObject)
                {
                    String key = kv.key().c_str();
                    String val = kv.value();
                    if (_debug)
                    {
                        SerialPrint("i", "onMqttRecive", "Прилетело MAC: " + dev + " key=" + key + " val=" + val);
                    }
                    if (_sensor == key)
                    {
                        dataFromNode = true;
                        _minutesPassed = 0;
                        setValue(val);
                        // setNewWidgetAttributes();
                    }

                    //   Serial.println("Key: " + key);
                    //   Serial.println("Value: " + val);
                }
            }
            else
            {
                if (_debug)
                {
                    SerialPrint("i", "onMqttRecive", "Прилетело MAC: " + dev + " val=" + msg);
                }
                dataFromNode = true;
                _minutesPassed = 0;
                setValue(msg);
                // setNewWidgetAttributes();
            }
        }
    }

    void doByInterval()
    {
        _minutesPassed++;
        setNewWidgetAttributes();
        if (mqttIsConnect() && !sendOk)
        {
            sendOk = true;
            mqttSubscribeExternal(_topic, _addPrefix);
        }
    }
    void onMqttWsAppConnectEvent()
    {
        setNewWidgetAttributes();
    }

    void setNewWidgetAttributes()
    {

        jsonWriteStr(json, F("info"), prettyMinutsTimeout(_minutesPassed));
        if (dataFromNode)
        {
            if (orange != 0 && red != 0 && offline != 0)
            {
                if (_minutesPassed < orange)
                {
                    jsonWriteStr(json, F("color"), "");
                }
                if (_minutesPassed >= orange && _minutesPassed < red)
                {
                    jsonWriteStr(json, F("color"), F("orange")); // сделаем виджет оранжевым
                }
                if (_minutesPassed >= red && _minutesPassed < offline)
                {
                    jsonWriteStr(json, F("color"), F("red")); // сделаем виджет красным
                }
                if (_minutesPassed >= offline)
                {
                    jsonWriteStr(json, F("info"), F("offline"));
                }
            }
        }
        else
        {
            jsonWriteStr(json, F("info"), F("awaiting"));
        }
        sendSubWidgetsValues(_id, json);
    }
    /*
        IoTValue execute(String command, std::vector<IoTValue> &param)
        {
            if (command == "mqttSubscribe")
            {
                if (param.size() == 2)
                {
                    if (!param[0].isDecimal && param[1].isDecimal)
                    {
                        mqttSubscribeExternal(param[0].valS, (bool)param[0].valD);
                    }
                }
            }
            return {};
        }
    */
    ~ExternalMQTT(){};
};

void *getAPI_ExternalMQTT(String subtype, String param)
{
    if (subtype == F("ExternalMQTT"))
    {
        return new ExternalMQTT(param);
    }
    else
    {
        return nullptr;
    }
}
