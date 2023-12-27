#include "Global.h"
#include "classes/IoTItem.h"

#include <SoftwareSerial.h>

#define R_LEN 7
#define C_LEN 8




class S8co : public IoTItem {
  private:
    byte cmd_s8[8]        = {0xFE, 0x04, 0x00, 0x03, 0x00, 0x01, 0xD5, 0xC5};
    //byte abc_s8[8]        = {0xFE, 0x03, 0x00, 0x1F, 0x00, 0x01, 0xA1, 0xC3};

    SoftwareSerial* s8Serial;

    unsigned int _s8_co2;
    int _s8_co2_mean = 0;
    int _s8_co2_mean2 = 0;

    float smoothing_factor = 0.5;
    float smoothing_factor2 = 0.15;

    byte _response_s8[7]   = {0, 0, 0, 0, 0, 0, 0};

    void s8Request(byte cmd[]) {
      if (!s8Serial) {
        SerialPrint("E", "Sensor S8_uart", "Serial not found!");
        return;
      }

      while(!s8Serial->available()) {
        s8Serial->write(cmd, C_LEN); 
        delay(50); 
      }
      
      int timeout=0;
      while(s8Serial->available() < R_LEN) {
        timeout++; 
        if(timeout > 10) {
          while(s8Serial->available()) {
            s8Serial->read();
            break;
          }
        } 
        delay(50); 
      } 
      
      for (int i=0; i < R_LEN; i++) { 
        _response_s8[i] = s8Serial->read(); 
      }
      
      //s8Serial->end();
    }                     

    void co2_measure() {
      s8Request(cmd_s8);    // запрашиваем значение загрязнения
      _s8_co2 = _response_s8[3] * 256 + _response_s8[4];   // полученный недобайт-ответ преобразуем в значение загрязнения
      
      if (!_s8_co2_mean) _s8_co2_mean = _s8_co2;
      _s8_co2_mean = _s8_co2_mean - smoothing_factor*(_s8_co2_mean - _s8_co2);
      
      if (!_s8_co2_mean2) _s8_co2_mean2 = _s8_co2;
      _s8_co2_mean2 = _s8_co2_mean2 - smoothing_factor2*(_s8_co2_mean2 - _s8_co2);

      Serial.printf("CO2 value: %d, M1Value: %d, M2Value: %d\n", _s8_co2, _s8_co2_mean, _s8_co2_mean2);
    }

  public:
    S8co(String parameters): IoTItem(parameters) { 
      int S8_RX_PIN, S8_TX_PIN;
      jsonRead(parameters, "rxPin", S8_RX_PIN);
      jsonRead(parameters, "txPin", S8_TX_PIN);

      s8Serial = new SoftwareSerial(S8_RX_PIN, S8_TX_PIN);
      if (!s8Serial) return;
      s8Serial->begin(9600);
    }
    
    void doByInterval() {
      co2_measure();
      value.valD = _s8_co2;
      if (value.valD < 15000)
        regEvent(value.valD, "S8co");
      else
        SerialPrint("E", "Sensor S8_uart", "Error");
    } 

    ~S8co() {
      if (s8Serial) delete s8Serial;
    };
};

void* getAPI_S8(String subtype, String param) {
  if (subtype == F("S8co")) {
    return new S8co(param);
  } else {
    return nullptr;
  }
}
