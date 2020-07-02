#include "Sensors/DHTSensor.h"

#include "Global.h"
#include "Events.h"
#include "MqttClient.h"
#include "Utils/PrintMessage.h"

namespace DHTSensor {

DHTesp dht;
String dhtT_value_name = "";
String dhtH_value_name = "";

const String comfortStr(ComfortState value) {
    String res;
    switch (value) {
        case Comfort_OK:
            res = F("Отлично");
            break;
        case Comfort_TooHot:
            res = F("Очень жарко");
            break;
        case Comfort_TooCold:
            res = F("Очень холодно");
            break;
        case Comfort_TooDry:
            res = F("Очень сухо");
            break;
        case Comfort_TooHumid:
            res = F("Очень влажно");
            break;
        case Comfort_HotAndHumid:
            res = F("Жарко и влажно");
            break;
        case Comfort_HotAndDry:
            res = F("Жарко и сухо");
            break;
        case Comfort_ColdAndHumid:
            res = F("Холодно и влажно");
            break;
        case Comfort_ColdAndDry:
            res = F("Холодно и сухо");
            break;
        default:
            res = F("Неизвестно");
            break;
    };
    return res;
}

const String perceptionStr(byte value) {
    String res;
    switch (value) {
        case 0:
            res = F("Сухой воздух");
            break;
        case 1:
            res = F("Комфортно");
            break;
        case 2:
            res = F("Уютно");
            break;
        case 3:
            res = F("Хорошо");
            break;
        case 4:
            res = F("Неудобно");
            break;
        case 5:
            res = F("Довольно неудобно");
            break;
        case 6:
            res = F("Очень неудобно");
            break;
        case 7:
            res = F("Невыносимо");
        default:
            res = F("Unknown");
            break;
    }
    return res;
}

void dhtT_reading() {
    float value = 0;
    static int counter;
    if (dht.getStatus() != 0 && counter < 5) {
        MqttClient::publishStatus(dhtT_value_name, String(dht.getStatusString()));
        counter++;
    } else {
        counter = 0;
        value = dht.getTemperature();
        if (String(value) != "nan") {
            Events::fire(dhtT_value_name);
            liveData.write(dhtT_value_name, String(value));
            MqttClient::publishStatus(dhtT_value_name, String(value));
            Serial.println("[I] sensor '" + dhtT_value_name + "' data: " + String(value));
        }
    }
}

void dhtH_reading() {
    float value = 0;
    static int counter;
    if (dht.getStatus() != 0 && counter < 5) {
        MqttClient::publishStatus(dhtH_value_name, String(dht.getStatusString()));
        counter++;
    } else {
        counter = 0;
        value = dht.getHumidity();
        if (String(value) != "nan") {
            Events::fire(dhtH_value_name);
            liveData.write(dhtH_value_name, String(value));
            MqttClient::publishStatus(dhtH_value_name, String(value));
            Serial.println("[I] sensor '" + dhtH_value_name + "' data: " + String(value));
        }
    }
}

void dhtP_reading() {
    byte value;
    if (dht.getStatus() != 0) {
        MqttClient::publishStatus("dhtPerception", String(dht.getStatusString()));
    } else {
        value = dht.computePerception(liveData.read(dhtT_value_name).toFloat(), liveData.read(dhtH_value_name).toFloat(), false);
        String final_line = perceptionStr(value);
        liveData.write("dhtPerception", final_line);
        Events::fire("dhtPerception");

        if (MqttClient::isConnected()) {
            MqttClient::publishStatus("dhtPerception", final_line);
            Serial.println("[I] sensor 'dhtPerception' data: " + final_line);
        }
    }
}

void dhtC_reading() {
    ComfortState cf;
    if (dht.getStatus() != 0) {
        MqttClient::publishStatus("dhtComfort", String(dht.getStatusString()));
    } else {
        dht.getComfortRatio(cf,
                            liveData.read(dhtT_value_name).toFloat(),
                            liveData.read(dhtH_value_name).toFloat(), false);
        String final_line = comfortStr(cf);
        liveData.write("dhtComfort", final_line);
        Events::fire("dhtComfort");
        MqttClient::publishStatus("dhtComfort", final_line);
        Serial.println("[I] sensor 'dhtComfort' send date " + final_line);
    }
}

void dhtD_reading() {
    float value;
    if (dht.getStatus() != 0) {
        MqttClient::publishStatus("dhtDewpoint", String(dht.getStatusString()));
    } else {
        value = dht.computeDewPoint(
            liveData.read(dhtT_value_name).toFloat(),
            liveData.read(dhtH_value_name).toFloat(), false);
        liveData.writeFloat("dhtDewpoint", value);
        Events::fire("dhtDewpoint");
        MqttClient::publishStatus("dhtDewpoint", String(value));
        Serial.println("[I] sensor 'dhtDewpoint' data: " + String(value));
    }
}

}  // namespace DHTSensor