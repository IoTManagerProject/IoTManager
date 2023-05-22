#include "Global.h"
#include "classes/IoTItem.h"


#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32

class SDcard : public IoTItem {
   private:

   public:
    SDcard(String parameters): IoTItem(parameters) {
        // jsonRead(parameters, "useLed", _useLed);    // используем = 1 или нет = 0 подсветку (вспышку)
        // jsonRead(parameters, "ticker", _ticker);    // тикать = 1 - сообщаем всем, что сделали снимок и он готов
        // jsonRead(parameters, "webTicker", _webTicker);    // сообщать всем, что через веб попросили отдать картинку с камеры

        Serial.println("Starting SD Card");
        if(!SD_MMC.begin()){
            Serial.println("SD Card Mount Failed");
            return;
        }
        
        uint8_t cardType = SD_MMC.cardType();
        if(cardType == CARD_NONE){
            Serial.println("No SD Card attached");
            return;
        } 
    }

    void savePicture(String path, IoTValue srcValue) {
        // if (srcValue.extBinInfoSize) {       
        //     fs::FS &fs = SD_MMC;    
        //     File file = fs.open(path.c_str(), FILE_WRITE);
        //     if(!file){
        //         Serial.println("Failed to open file in writing mode");
        //     } 
        //     else {
        //         file.write(srcValue.extBinInfo, srcValue.extBinInfoSize); // payload (image), payload length
        //         Serial.printf("Picture file name: %s | bufsize: %d\n", path.c_str(), srcValue.extBinInfoSize);
        //     }
        //     file.close();
        // }
    }

    void doByInterval() {

    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (command == "saveBin") { 
            if (param.size() == 2) {
                savePicture (param[0].valS, param[1]);
            }
        }

        return {};  
    }

    ~SDcard() {

    };
};

void* getAPI_SDcard(String subtype, String param) {
    if (subtype == F("SDcard")) {
        return new SDcard(param);
    } else {
        return nullptr;
    }
}
