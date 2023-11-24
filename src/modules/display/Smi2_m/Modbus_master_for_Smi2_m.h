#pragma once
#include "HardwareSerial.h"

#define PRESET_MULTIPLE_REGISTERS 16
#define BUFFER_SIZE 64

typedef struct
{
    unsigned char id;
    unsigned char function;
    unsigned int address;

    unsigned int data;
    unsigned int local_start_address;

    unsigned int requests;
    unsigned int successful_requests;
    unsigned int failed_requests;
    unsigned int exception_errors;
    unsigned int retries;

    unsigned char connection;

} Packet;

class Smi_display {
   public:
    Smi_display();

    ~Smi_display();

   public:
    void modbus_update();
    void modbus_construct(Packet* _packet,
                          unsigned char id,
                          unsigned char function,
                          unsigned int address,
                          unsigned int data,
                          unsigned _local_start_address);

    void modbus_configure(HardwareSerial* SerialPort,
                          long baud,
                          unsigned char byteFormat,
                          int rx,
                          int tx,
                          unsigned int _TxEnablePin,
                          Packet* _packets,
                          unsigned int _total_no_of_packets,
                          /*unsigned*/ int* _register_array);

   private:
    void idle();
    void constructPacket();
    unsigned char construct_F16();
    unsigned int calculateCRC(unsigned char bufferSize);
    void sendPacket(unsigned char bufferSize);

   private:
    unsigned char state;
    unsigned char retry_count;
    unsigned int TxEnablePin;
    unsigned char frame[BUFFER_SIZE];
    unsigned char buffer;
    unsigned int T1_5;
    unsigned int frameDelay;
    unsigned int total_no_of_packets;
    Packet* packetArray;
    Packet* packet;
    /*unsigned*/ int* register_array;
    HardwareSerial* ModbusPort;
};
