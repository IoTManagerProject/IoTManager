/******************************************************************
  Used Adafruit INA219 Current Sensor
  Support for INA219
  https://github.com/adafruit/Adafruit_INA219

  adapted for version 4dev @Serghei63
 ******************************************************************/

#include "Global.h"
#include "classes/IoTItem.h"

#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

 // shuntvoltage = ina219.getShuntVoltage_mV();       // Получение напряжение на шунте 
 // busvoltage = ina219.getBusVoltage_V();            // Получение значение напряжения V
 // current_mA = ina219.getCurrent_mA();              // Получение значение тока в мА
 // power_mW = ina219.getPower_mW();                  // Получение значение мощности
 // loadvoltage = busvoltage + (shuntvoltage / 1000); // Расчет напряжение на нагрузки

    class Ina219loadvoltage : public IoTItem {
   public:
    Ina219loadvoltage(String parameters) : IoTItem(parameters) {

       // Wire.begin(2,0);       // Инициализация шины I2C для модуля E01
    }

    void doByInterval() {

        loadvoltage = busvoltage + (shuntvoltage / 1000);
        value.valD = loadvoltage;
        
            regEvent(value.valD, "Ina219loadvoltage"); 
    }

    ~Ina219loadvoltage(){};
};

  class Ina219busvoltage : public IoTItem {
   public:
    Ina219busvoltage(String parameters) : IoTItem(parameters) {

       // Wire.begin(2,0);       // Инициализация шины I2C для модуля E01
    }

    void doByInterval() {

        busvoltage = ina219.getBusVoltage_V();
        value.valD = busvoltage;
        
            regEvent(value.valD, "Ina219busvoltage"); 
    }

    ~Ina219busvoltage(){};
};


class Ina219curr : public IoTItem {
   public:
    Ina219curr(String parameters) : IoTItem(parameters) {

       // Wire.begin(2,0);       // Инициализация шины I2C для модуля E01
    }
    void doByInterval() {

        current_mA = ina219.getCurrent_mA();
        value.valD = current_mA;
     
            regEvent(value.valD, "Ina219curr"); 
    }

    ~Ina219curr(){};
};

class Ina219shuntvoltage : public IoTItem {
   public:
    Ina219shuntvoltage(String parameters) : IoTItem(parameters) {

       // Wire.begin(2,0);       // Инициализация шины I2C для модуля E01
    }
    void doByInterval() {

         shuntvoltage = ina219.getShuntVoltage_mV();
        value.valD = shuntvoltage;
       
            regEvent(value.valD, "Ina219shuntvoltage"); 
    }

    ~Ina219shuntvoltage(){};
};

class Power_mW : public IoTItem {
   public:
    Power_mW(String parameters) : IoTItem(parameters) {

       // Wire.begin(2,0);       // Инициализация шины I2C для модуля E01
    }
    void doByInterval() {

        power_mW = ina219.getPower_mW();
        value.valD = power_mW;
      
            regEvent(value.valD, "Ina219power");  // TODO: найти способ понимания ошибки получения данных

    }

    ~Power_mW(){};
};
void* getAPI_Ina219(String subtype, String param) {
    if (subtype == F("Ina219curr")) {
        ina219.begin();
        return new Ina219curr(param);
    } else if (subtype == F("Ina219shuntvoltage")) {
        ina219.begin();
        return new Ina219shuntvoltage(param);
    } else if  (subtype == F("power_mW")) {
        ina219.begin();
        return new Power_mW(param);
    } else if  (subtype == F("Ina219busvoltage")) {
        ina219.begin();
        return new Ina219busvoltage(param);
    } else if  (subtype == F("Ina219loadvoltage")) {
        ina219.begin();
        return new Ina219loadvoltage(param);
        } else {
       return nullptr;
    }
}
