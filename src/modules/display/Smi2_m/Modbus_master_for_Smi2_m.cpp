#include "Modbus_master_for_Smi2_m.h"

Smi_display::Smi_display() {
}

// Modbus Master
void Smi_display::modbus_update() {
    idle();
}

void Smi_display::idle() {
    static unsigned int packet_index;

    unsigned int failed_connections = 0;

    unsigned char current_connection;

    do {
        if (packet_index == total_no_of_packets)  // wrap around to the beginning
            packet_index = 0;

        // proceed to the next packet
        packet = &packetArray[packet_index];

        // get the current connection status
        current_connection = packet->connection;

        if (!current_connection) {
            // If all the connection attributes are false return
            // immediately to the main sketch
            if (++failed_connections == total_no_of_packets)
                return;
        }
        packet_index++;

        // if a packet has no connection get the next one
    } while (!current_connection);

    constructPacket();
}

void Smi_display::constructPacket() {
    packet->requests++;
    frame[0] = packet->id;
    frame[1] = packet->function;
    frame[2] = packet->address >> 8;    // address Hi
    frame[3] = packet->address & 0xFF;  // address Lo
    frame[4] = packet->data >> 8;       // MSB
    frame[5] = packet->data & 0xFF;     // LSB

    unsigned char frameSize;

    // construct the frame according to the modbus function
    if (packet->function == PRESET_MULTIPLE_REGISTERS)
        frameSize = construct_F16();
    else                // else functions 1,2,3,4,5 & 6 is assumed. They all share the exact same request format.
        frameSize = 8;  // the request is always 8 bytes in size for the above mentioned functions.

    unsigned int crc16 = calculateCRC(frameSize - 2);
    frame[frameSize - 2] = crc16 >> 8;  // split crc into 2 bytes
    frame[frameSize - 1] = crc16 & 0xFF;
    sendPacket(frameSize);
}

unsigned char Smi_display::construct_F16() {
    unsigned char no_of_bytes = packet->data * 2;

    // first 6 bytes of the array + no_of_bytes + 2 bytes CRC
    frame[6] = no_of_bytes;   // number of bytes
    unsigned char index = 7;  // user data starts at index 7
    unsigned char no_of_registers = packet->data;
    /*unsigned*/ int temp;

    for (unsigned char i = 0; i < no_of_registers; i++) {
        temp = register_array[packet->local_start_address + i];  // get the data
        frame[index] = temp >> 8;
        index++;
        frame[index] = temp & 0xFF;
        index++;
    }
    unsigned char frameSize = (9 + no_of_bytes);  // first 7 bytes of the array + 2 bytes CRC + noOfBytes
    return frameSize;
}

void Smi_display::modbus_configure(HardwareSerial* SerialPort,
                                   long baud,
                                   unsigned char byteFormat,
                                   int rx,
                                   int tx,
                                   unsigned int _TxEnablePin,
                                   Packet* _packets,
                                   unsigned int _total_no_of_packets,
                                   /*unsigned*/ int* _register_array) {
    TxEnablePin = _TxEnablePin;
    total_no_of_packets = _total_no_of_packets;
    packetArray = _packets;
    register_array = _register_array;

    ModbusPort = SerialPort;
    (*ModbusPort).begin(baud, byteFormat, rx, tx);

    pinMode(TxEnablePin, OUTPUT);
    digitalWrite(TxEnablePin, LOW);
}

void Smi_display::modbus_construct(Packet* _packet,
                                   unsigned char id,
                                   unsigned char function,
                                   unsigned int address,
                                   unsigned int data,
                                   unsigned int local_start_address) {
    _packet->id = id;
    _packet->function = function;
    _packet->address = address;
    _packet->data = data;
    _packet->local_start_address = local_start_address;
    _packet->connection = 1;
}

unsigned int Smi_display::calculateCRC(unsigned char bufferSize) {
    unsigned int temp, temp2, flag;
    temp = 0xFFFF;
    for (unsigned char i = 0; i < bufferSize; i++) {
        temp = temp ^ frame[i];
        for (unsigned char j = 1; j <= 8; j++) {
            flag = temp & 0x0001;
            temp >>= 1;
            if (flag)
                temp ^= 0xA001;
        }
    }
    // Reverse byte order.
    temp2 = temp >> 8;
    temp = (temp << 8) | temp2;
    temp &= 0xFFFF;
    // the returned value is already swapped
    // crcLo byte is first & crcHi byte is last
    return temp;
}

void Smi_display::sendPacket(unsigned char bufferSize) {
    digitalWrite(TxEnablePin, HIGH);

    for (unsigned char i = 0; i < bufferSize; i++)
        (*ModbusPort).write(frame[i]);

    (*ModbusPort).flush();

    digitalWrite(TxEnablePin, LOW);
}