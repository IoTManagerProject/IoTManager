#include "Global.h"
#include "classes/IoTItem.h"
#include "ESPConfiguration.h"
#include <Adafruit_NeoPixel.h>


class Ws2812b : public IoTItem
{
private:
    Adafruit_NeoPixel *_strip;
    int _pin;
    int _numLeds;
    int _brightness;
    int correctLed;
    int _min = 0;
    int _max = 100;
    int PrevValidShow = 0;
    int FlagFN = 1;
    int PreFlagFN = 1;
    String idshow;

public:
    Ws2812b(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, F("pin"), _pin);
        jsonRead(parameters, F("numLeds"), _numLeds);
        jsonRead(parameters, F("idshow"), idshow);
        jsonRead(parameters, F("brightness"), _brightness);
        jsonRead(parameters, F("min"), _min);
        jsonRead(parameters, F("max"), _max);
        
        
        _strip = new Adafruit_NeoPixel(_numLeds, _pin, NEO_GRB + NEO_KHZ800);
        if (_strip != nullptr) {
            _strip->begin();
            SerialPrint("E", "Strip Ws2812b:" + _id, "begin");
            correctLed = correctPixel(_numLeds);
            _strip->setBrightness(_brightness);
            _strip->clear(); 
        }
    }

    // void loop() {
    //     if (enableDoByInt) {
    //         currentMillis = millis();
    //         difference = currentMillis - prevMillis;
    //         if (difference >= interval  || PreFlagFN != FlagFN) {
    //             prevMillis = millis();
    //             this->doByInterval();
    //         }
    //     }
    // }

    void doByInterval() {
        if (!_strip) return;

        if (!isItemExist(idshow)) {
            SerialPrint("E", F("Ws2812b"), "'" + idshow + "' detector object not exist");
        }else if (getItemValue(idshow) == ""){
            SerialPrint("E", F("Ws2812b"), "'" + idshow + "' detector value is empty");
        }else if (_min >= _max){
            SerialPrint("E", F("Ws2812b"), " the minimum (" + String(_min) + ") value must be greater than the maximum (" + String(_max) + ")");
        }else if(isItemExist(idshow) && getItemValue(idshow) != "" && _min < _max && FlagFN == 1){  
            SerialPrint("E", "Ws2812b:" + String(correctLed), " work");
            String value = getItemValue(idshow);  
                if(PrevValidShow == 0 || PrevValidShow > value.toInt() ){
                    noShow();
                }
                int t = map(value.toInt(), _min, _max, 0, _numLeds);
                for(uint16_t L = 0; L<t; L++) { 
                    _strip->setPixelColor(L,wheel(((205+(L*correctLed)) & 255)));
                } 
            PrevValidShow = value.toInt();
            _strip->show();
        }
    }
   
    int correctPixel(int _numLeds){
        if(_numLeds <= 65 && _numLeds > 60){
            correctLed = 0;
        }else if(_numLeds <= 60 && _numLeds > 55){
            correctLed = 1;
        }else if(_numLeds <= 55 && _numLeds > 50){
            correctLed = 2;
        }else if(_numLeds <= 50 && _numLeds > 40){
            correctLed = 3;
        }else if(_numLeds <= 40 && _numLeds > 35){
            correctLed = 4;
        }else if(_numLeds <= 35 && _numLeds > 24){
            correctLed = 5;
        }else if(_numLeds <= 24 && _numLeds > 16){
            correctLed = 6;
        }else if(_numLeds <= 16 && _numLeds > 12){
            correctLed = 8;
        }else if(_numLeds <= 12 && _numLeds > 8){
            correctLed = 12;
        }else if(_numLeds <= 8 && _numLeds > 4){
            correctLed = 16;
        }else{
            correctLed = 0;
            }
        return correctLed;
    }

    uint32_t wheel(byte WheelPos) { 
        if(WheelPos < 85) { 
            return _strip->Color(WheelPos * 3, 255 - WheelPos * 3, 0); 
        } 
        else if(WheelPos < 205) { 
            WheelPos -= 85; 
            return _strip->Color(255 - WheelPos * 3, 0, WheelPos * 3); 
        } 
        else { 
            WheelPos -= 205; 
            return _strip->Color(0, WheelPos * 3, 255 - WheelPos * 3); 
        } 
    } 

    void noShow(){
        if (!_strip) return;

        _strip->clear(); 
        for(int i=0; i<_numLeds; i++) {
            _strip->setPixelColor(i, _strip->Color(0, 0, 0));
            _strip->show(); 
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (!_strip) return {};

        if (command == "test") {
            for(int i=0; i<_numLeds; i++) {
                _strip->setPixelColor(i, _strip->Color(20+(i*2), 20+(i*2), 20+(i*2)));
                _strip->show(); 
            }
            SerialPrint("E", "Strip Ws2812b", "demo");
        } else if (command == "noShow"){
            noShow();
            SerialPrint("E", "Strip Ws2812b", "noShow");
        } else if(command == "noShowOne"){
            if (param.size() == 1) {
                _strip->setPixelColor(param[0].valD, _strip->Color(0, 0, 0));
                _strip->show();
                SerialPrint("E", "Strip Ws2812b", "noShowOne");
            }
        } else if (command == "showLed"){
            if (param.size() == 4) {
                _strip->setPixelColor( param[0].valD, _strip->Color(param[1].valD, param[2].valD, param[3].valD));
                _strip->show(); 
                SerialPrint("E", "Strip Ws2812b", "showLed:" + param[0].valS + " red:" + param[1].valS + " green:" + param[2].valS + " blue:" + param[3].valS);
            }
        } else if (command == "showLedAll"){
            if (param.size() == 3) {
                for(int i=0; i<_numLeds; i++) {
                _strip->setPixelColor(i, _strip->Color(param[0].valD, param[1].valD, param[2].valD));
                _strip->show(); 
                }
                SerialPrint("E", "Strip Ws2812b", "showLedAll - red:" + param[0].valS + " green:" + param[1].valS + " blue:" + param[2].valS);
            }
        } else if (command == "disableIndication"){
            FlagFN = 0;
            PreFlagFN = 1;
            SerialPrint("E", "Strip Ws2812b", "disableIndication");
        } else if (command == "enableIndication"){
            FlagFN = 1;
            PreFlagFN = 0;
            doByInterval();
            SerialPrint("E", "Strip Ws2812b", "enableIndication");
        }
        doByInterval();
        return {};
    }

    void setValue(const IoTValue& Value, bool genEvent = true){
        if (!_strip) return;

        value = Value;
        int b =  map(value.valD, 1,1024,1,255);
        _strip->setBrightness(b);
        _strip->show(); 
        regEvent(value.valD, "Ws2812b", false, genEvent);
    }

    ~Ws2812b(){};
};

void *getAPI_Ws2812b(String subtype, String param)
{
    if (subtype == F("Ws2812b")) {
        return new Ws2812b(param);
    } else {
        return nullptr;
    }
}




