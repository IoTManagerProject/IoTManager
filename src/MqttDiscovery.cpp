#include "MqttDiscovery.h"

namespace Discovery {

static const char json_is_defined[] = "is_defined";
static const char jsonId[] = "id";
static const char jsonBatt[] = "batt";
static const char jsonLux[] = "lux";
static const char jsonPres[] = "pres";
static const char jsonFer[] = "fer";
static const char jsonMoi[] = "moi";
static const char jsonHum[] = "hum";
static const char jsonTemp[] = "tem";
static const char jsonStep[] = "steps";
static const char jsonWeight[] = "weight";
static const char jsonPresence[] = "presence";
static const char jsonAltim[] = "altim";
static const char jsonAltif[] = "altift";
static const char jsonTempf[] = "tempf";
static const char jsonMsg[] = "message";
static const char jsonVal[] = "value";
static const char jsonVolt[] = "volt";
static const char jsonCurrent[] = "current";
static const char jsonPower[] = "power";
static const char jsonGpio[] = "gpio";
static const char jsonFtcd[] = "ftcd";
static const char jsonWm2[] = "wattsm2";
static const char jsonAdc[] = "adc";
static const char jsonPa[] = "pa";

const String getValueJson(const char* str) {
    char buf[32];
    sprintf(buf, "{{ value_json.%s }}", str);
    return buf;
}

void createDiscovery(
    const char* type, const char* name, const char* clazz,
    const char* value_template, const char* payload_on, const char* payload_off,
    const char* maasure_unit, int off_delay, const char* has_payload, const char* no_payload,
    const char* avail_topi, const char* cmd_topic, const char* state_topic, bool child) {
   //const char* unique_id = getUniqueId(name).c_str();
}

void createADC(const char* name) {
    createDiscovery(
        "Type", "Name", "Clazz",
        "Value", "Payload", "NoPayload",
        "Measure", 0, "HasPayload", "NoPayload",
        "", "", "", false);
}

void createSwitch(const char* name) {
    createDiscovery(
        "Type", "Name", "Clazz",
        "Value", "Payload", "NoPayload",
        "Measure", 0, "HasPayload", "NoPayload",
        "", "", "", false);
}
// component,
// type,
// name,
// availability topic,
// device class,
// value template, payload on, payload off, unit of measurement
const char* BMEsensor[6][8] = {
    {"sensor", "tempc", "bme", "temperature", "", "", "°C"},  //jsonTemp
    {"sensor", "tempf", "bme", "temperature", "", "", "°F"},  //jsonTempf
    {"sensor", "pa", "bme", "", "", "", "hPa"},               //jsonPa
    {"sensor", "hum", "bme", "humidity", "", "", "%"},        // jsonHum
    {"sensor", "altim", "bme", "", "", "", "m"},              //jsonAltim
    {"sensor", "altift", "bme", "", "", "", "ft"}             // jsonAltif
};

}  // namespace Discovery
