/******************************************************************
  Simple library for NTC thermistors
  https://github.com/GyverLibs/GyverNTC

  adapted for version 4 @Serghei63
 ******************************************************************/


#include "Global.h"
#include "classes/IoTItem.h"


class NTCt : public IoTItem {
   private:

	unsigned int _pin;
	float R1 = 10000.0;   // voltage divider resistor value
	float Beta = 3950.0;  // Beta value
	float To = 25.;    // Temperature Celsius
	float Ro = 10000.0;   // Resistance of Thermistor at 25 degree Celsius
	float adcMax = 1023.0;
	float Vs = 3.3;
   public:
      NTCt(String parameters): IoTItem(parameters) {

	_pin = jsonReadInt(parameters, "pin");
 	 R1 = jsonReadInt(parameters, "R1");   // voltage divider resistor value
	 Beta = jsonReadInt(parameters, "Beta");  // Beta value
	 To = jsonReadInt(parameters, "T0");    // Temperature degree Celsius
	 Ro = jsonReadInt(parameters, "R0");   // Resistance of Thermistor at 25 degree Celsius
    Vs = jsonReadInt(parameters, "Vs");   // Resistance of Thermistor at 25 degree Celsius
     To = To+273.15;
     
#if defined ESP8266
    adcMax = 1023.0;  
#elif defined ESP32      
    adcMax = 4095.0; 
#endif

     }
    
    void doByInterval() {
		
		float Vout, Rt = 0;
		float T = 0;
		float adc = 0;
        for (int i = 0; i < 10; i++)
        {
            adc += IoTgpio.analogRead(_pin);
        }
        adc /= 10;

		Vout = adc * Vs/adcMax;
		Rt = R1 * Vout / (Vs - Vout);
		T = 1/(1/To + log(Rt/Ro)/Beta);    // Temperature in Kelvin
        value.valD = T - 273.15; // Celsius
		
        SerialPrint("i", F("Ntc"), "adc = " + String(adc)+ " ,Vout = " + String(Vout)+ " ,T = " + String(value.valD));
        if (String(value.valD) != "nan") regEvent(value.valD, "Ntc");
		else
            SerialPrint("E", "Ntc", "Error");
    }

    ~NTCt() {};
};

void* getAPI_Ntc(String subtype, String param) {
    if (subtype == F("Ntc")) {

        return new NTCt(param);

    } else {
        return nullptr;
    }
}
