#include "Utils/PresetUtils.h"

static const char* item_names[NUM_ITEMS] = {"relay", "pwm",
                                            "dht11", "dht22", "analog",
                                            "bmp280", "bme280", "dallas",
                                            "termostat", "ultrasonic",
                                            "motion", "stepper",
                                            "servo", "firmware"};

static const char* config_file_fmt = "/conf/%03d%s.txt";

const String getPresetFile(uint8_t preset, ConfigType_t type) {
    char buf[64];
    sprintf(buf, config_file_fmt, preset, type == CT_MACRO ? "c" : "s");
    return String(buf);
}

const String getItemName(Item_t item) {
    return String(item_names[item]);
}

const Item_t getPresetItem(uint8_t num) {
    Item_t res = NUM_ITEMS;
    if (num >= 1 && num <= 7) {
        res = RELAY;
    } else if (num == 8) {
        res = PWM;
    } else if (num == 9) {
        res = DHT11;
    } else if (num == 10) {
        res = DHT22;
    } else if (num == 11) {
        res = ANALOG;
    } else if (num == 12) {
        res = BMP280;
    } else if (num == 13) {
        res = BME280;
    } else if (num == 14) {
        res = DALLAS;
    } else if (num == 15) {
        res = TERMOSTAT;
    } else if (num == 16) {
        res = ULTRASONIC;
    } else if (num >= 17 || num <= 18) {
        res = MOTION;
    } else if (num == 19) {
        res = STEPPER;
    } else if (num == 20) {
        res = SERVO;
    } else if (num == 21) {
        res = FIRMWARE;
    }
    return res;
}