#pragma once

#include <Arduino.h>

#include <DallasTemperature.h>

#include "OneWireBus.h"

// OneWire commands
#define STARTCONVO 0x44       // Tells device to take a temperature reading and put it on the scratchpad
#define COPYSCRATCH 0x48      // Copy EEPROM
#define READSCRATCH 0xBE      // Read EEPROM
#define WRITESCRATCH 0x4E     // Write to EEPROM
#define RECALLSCRATCH 0xB8    // Reload from last known
#define READPOWERSUPPLY 0xB4  // Determine if device needs parasite power
#define ALARMSEARCH 0xEC      // Query bus for devices with an alarm condition
// Error Codes
#define DEVICE_DISCONNECTED_C -127
#define DEVICE_DISCONNECTED_F -196.6
#define DEVICE_DISCONNECTED_RAW -7040
// Scratchpad locations
#define TEMP_LSB 0
#define TEMP_MSB 1
#define HIGH_ALARM_TEMP 2
#define LOW_ALARM_TEMP 3
#define CONFIGURATION 4
#define INTERNAL_BYTE 5
#define COUNT_REMAIN 6
#define COUNT_PER_C 7
#define CRC_DATA_BYTE 8

// Device resolution
#define TEMP_9_BIT 0x1F   //  9 bit
#define TEMP_10_BIT 0x3F  // 10 bit
#define TEMP_11_BIT 0x5F  // 11 bit
#define TEMP_12_BIT 0x7F  // 12 bit

enum DallasSensorState {
    DALLAS_IDLE,
    DALLAS_CONVERSATION
};
class DallasSensor {
   public:
    DallasSensor(const char* name, OneWireAddress addr);
    const char* name();
    OneWireAddress* addr();
    bool update();

    void setAddr(OneWireAddress addr);

    bool isParasitePowerMode(void);

    bool requestTemperature();
    uint16_t getWaitTimeForConversion();
    bool isConnected();
    bool isConversionComplete();

    int16_t getTemp();

    float getTempC();

    float getTempF();

    int16_t calculateTemperature();

    // convert from Celsius to Fahrenheit
    static float toFahrenheit(float);

    // convert from Fahrenheit to Celsius
    static float toCelsius(float);

    // convert from raw to Celsius
    static float rawToCelsius(int16_t);

    // convert from raw to Fahrenheit
    static float rawToFahrenheit(int16_t);

   private:
    bool isValid();
    bool isZeros(uint8_t* data, const size_t len = 9);
    bool readRawData();

   private:
    OneWire* _wire;
    OneWireAddress _addr;
    uint8_t _bitResolution;
    DallasSensorState _state;
    bool _parasite;
    uint8_t _rawData[9];

    float _value;
    char _name[16];
    bool _hasValue;
    unsigned long _requestConversationTime;
};
