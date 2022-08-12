//#pragma once
//#include <Arduino.h>
//#include <iarduino_RTC.h>
//
// class iarduino_RTC_NTP: public iarduino_RTC_BASE {
//	public:
//		iarduino_RTC_NTP();
//        ~iarduino_RTC_NTP();
//		void	begin(void);
//		uint8_t	funcReadTimeIndex(uint8_t i);
//		void	funcWriteTimeIndex(uint8_t i, uint8_t j);
//        uint8_t	funcConvertNumToCode	(uint8_t i)	{return ((i/10) << 4) + (i%10);}
//};
//
// class IoTRTC : public iarduino_RTC {
//    public:
//        IoTRTC(uint8_t i, uint8_t j=SS, uint8_t k=SCK, uint8_t n=MOSI) : iarduino_RTC(i, j, k, n) {
//            if (i == 0) objClass = new iarduino_RTC_NTP();
//        };
//
//        ~IoTRTC();
//};