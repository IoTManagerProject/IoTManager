#include "Global.h"
#include "classes/IoTItem.h"

#include <WiegandNG.h>

#define PIN_D0 D5
#define PIN_D1 D6

WiegandNG wg;

void PrintBinary(WiegandNG &tempwg) {
	volatile unsigned char *buffer=tempwg.getRawData();
	unsigned int bufferSize = tempwg.getBufferSize();
	unsigned int countedBits = tempwg.getBitCounted();

	unsigned int countedBytes = (countedBits/8);
	if ((countedBits % 8)>0) countedBytes++;
	// unsigned int bitsUsed = countedBytes * 8;
	
	for (unsigned int i=bufferSize-countedBytes; i< bufferSize;i++) {
		unsigned char bufByte=buffer[i];
		for(int x=0; x<8;x++) {
			if ( (((bufferSize-i) *8)-x) <= countedBits) {
				if((bufByte & 0x80)) {
					Serial.print("1");
				}
				else {
					Serial.print("0");
				}
			}
			bufByte<<=1;
		}
	}
	Serial.println();
}

class IoTWiegand : public IoTItem {

   public:
    IoTWiegand(String parameters) : IoTItem(parameters) {
        // int pinD0;
        // int pinD1;

        // jsonRead(parameters, "pinD0", pinD0);
        // jsonRead(parameters, "pinD1", pinD1);
        
        unsigned int pinD0 = D5;
        unsigned int pinD1 = D6;
        unsigned int wiegandbits = 48;
	    unsigned int packetGap = 15;			// 25 ms between packet

        // if(!wg.begin(pinD0, pinD1, wiegandbits, packetGap)) {
        //     Serial.println("Out of memory!");
        // }
        
    }

    void loop() {
        if(wg.available()) {
            wg.pause();							// pause Wiegand pin interrupts
            Serial.print("Bits=");
            Serial.println(wg.getBitCounted());	// display the number of bits counted
            Serial.print("RAW Binary=");
            PrintBinary(wg);					// display raw data in binary form, raw data inclusive of PARITY
            wg.clear();							// compulsory to call clear() to enable interrupts for subsequent data
        }   
    }

    ~IoTWiegand(){};
};

void* getAPI_IoTWiegand(String subtype, String param) {
    if (subtype == F("IoTWiegand")) {
        return new IoTWiegand(param);
    } else {
        return nullptr;
    }
}
