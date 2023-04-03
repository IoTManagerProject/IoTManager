#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>

#define PZEM_DEFAULT_ADDR 0xF8

struct PZEM_Info {
    float voltage;
    float current;
    float power;
    float energy;
    float freq;
    float pf;
    uint16_t alarms;
    PZEM_Info() : voltage{0}, current{0}, power{0}, energy{0}, freq{0}, pf{0}, alarms{0} {};
};

class PZEMSensor {
   public:
    PZEMSensor(Stream *serial, uint16_t addr = PZEM_DEFAULT_ADDR);

    ~PZEMSensor();
    PZEM_Info *values(bool &online);
    bool setAddress(uint8_t addr);
    uint8_t getAddress();
    bool setPowerAlarm(uint16_t watts);
    bool getPowerAlarm();
    bool reset();
    bool search();
    // Get most up to date values from device registers and cache them
    bool refresh();
    void updateSerial(Stream *serial) {_serial = serial;}

   private:
    void init(void); 

   private:
    PZEM_Info _values;   // Measured values
    Stream *_serial;     // Serial interface
    bool _isSoft;        // Is serial interface software
    uint8_t _addr;       // Device address
    uint64_t _lastRead;  // Last time values were updated

    void init(uint8_t addr);                                                                                     // Init common to all constructors
    uint16_t recieve(uint8_t *resp, uint16_t len);                                                               // Receive len bytes into a buffer
    bool sendCmd8(uint8_t cmd, uint16_t rAddr, uint16_t val, bool check = false, uint16_t slave_addr = 0xFFFF);  // Send 8 byte command
    void setCRC(uint8_t *buf, uint16_t len);                                                                     // Set the CRC for a buffer
    bool checkCRC(const uint8_t *buf, uint16_t len);                                                             // Check CRC of buffer
    uint16_t CRC16(const uint8_t *data, uint16_t len);                                                           // Calculate CRC of buffer
};
