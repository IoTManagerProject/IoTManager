#pragma once

enum TimerTask_t { WIFI_SCAN,
                   WIFI_MQTT_CONNECTION_CHECK,
                   SENSORS,
                   STEPPER1,
                   STEPPER2,
                   LOG1,
                   LOG2,
                   LOG3,
                   LOG4,
                   LOG5,
                   TIMER_COUNTDOWN,
                   TIME,
                   TIME_SYNC,
                   STATISTICS,
                   UPTIME,
                   UDP,
                   UDP_DB,
                   TEST };

enum ErrorType_t {
    ET_NONE,
    ET_FUNCTION,
    ET_MODULE,
    ET_SYSTEM
};

enum ErrorLevel_t {
    EL_NONE,
    EL_INFO,
    EL_WARNING,
    EL_ERROR
};

enum LedStatus_t {
    LED_OFF,
    LED_ON,
    LED_SLOW,
    LED_FAST
};

enum ConfigType_t {
    CT_CONFIG,
    CT_SCENARIO
};

enum Item_t {
    RELAY,
    PWM,
    DHT11,
    DHT22,
    ANALOG,
    BMP280,
    BME280,
    DALLAS,
    TERMOSTAT,
    ULTRASONIC,
    MOTION,
    STEPPER,
    SERVO,
    FIRMWARE,
    NUM_ITEMS
};