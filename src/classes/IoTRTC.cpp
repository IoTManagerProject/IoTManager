#include "classes/IoTRTC.h"

IoTRTC::~IoTRTC() {

}


iarduino_RTC_NTP::iarduino_RTC_NTP() {

}

void iarduino_RTC_NTP::begin(void) {	
	
}
		
uint8_t	iarduino_RTC_NTP::funcReadTimeIndex(uint8_t i) {
    static tm localTimeVar;	
    if (i == 0) {
        Serial.println(F("Read time from system.")); 
        time_t timeNowVar; 
        time(&timeNowVar);
        localTimeVar = *localtime(&timeNowVar); 
    }

    switch (i)
    {
        case 0: return funcConvertNumToCode(localTimeVar.tm_sec);
        case 1: return funcConvertNumToCode(localTimeVar.tm_min);
        case 2: return funcConvertNumToCode(localTimeVar.tm_hour);
        case 3: return funcConvertNumToCode(localTimeVar.tm_mday);
        case 4: return funcConvertNumToCode(localTimeVar.tm_mon+1);
        case 5: return funcConvertNumToCode(localTimeVar.tm_year-100);
        case 6: return funcConvertNumToCode(localTimeVar.tm_wday);
    }
    
    return 0;
}	
	
void iarduino_RTC_NTP::funcWriteTimeIndex(uint8_t i, uint8_t j) {

}

