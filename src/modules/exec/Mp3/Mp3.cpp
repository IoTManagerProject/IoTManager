#include "Global.h"
#include "classes/IoTItem.h"

#include "SoftwareSerial.h"               // Подключаем библиотеку SoftwareSerial
#include "DFRobotDFPlayerMini.h"            // Подключаем библиотеку DFPlayerMini_Fast


class Mp3 : public IoTItem {
   private:
    SoftwareSerial* mySerial;
    DFRobotDFPlayerMini* myMP3;

   public:
    Mp3(String parameters): IoTItem(parameters) {
        String tmpstr;
        int volumetmp;
        jsonRead(parameters, "pins", tmpstr);
        int pinRx = selectFromMarkerToMarker(tmpstr, ",", 0).toInt();  
        int pinTx = selectFromMarkerToMarker(tmpstr, ",", 1).toInt();  
        mySerial = new SoftwareSerial(pinRx, pinTx);
        pinMode(pinRx, INPUT);
        pinMode(pinTx, OUTPUT);

        jsonRead(parameters, "volume", volumetmp);

        if (mySerial) {
            mySerial->begin(9600);
            myMP3 = new DFRobotDFPlayerMini();
        }
        if (myMP3) {
            myMP3->begin(*mySerial);
            myMP3->volume(volumetmp);
        } 

        value.isDecimal = false;    // значение объекта всегда будет строка
    }

    void doByInterval() {
        if (myMP3 && myMP3->available()) {
            switch (myMP3->readType()) {
                case TimeOut:
                value.valS = F("Time Out!");
                break;
                case WrongStack:
                value.valS = F("Stack Wrong!");
                break;
                case DFPlayerCardInserted:
                value.valS = F("Card Inserted!");
                break;
                case DFPlayerCardRemoved:
                value.valS = F("Card Removed!");
                break;
                case DFPlayerCardOnline:
                value.valS = F("Card Online!");
                break;
                case DFPlayerPlayFinished:
                value.valS = F("Play Finished!");
                break;
                case DFPlayerError:
                    switch (myMP3->read()) {
                        case Busy:
                        value.valS = F("Card not found");
                        break;
                        case Sleeping:
                        value.valS = F("Sleeping");
                        break;
                        case SerialWrongStack:
                        value.valS = F("Get Wrong Stack");
                        break;
                        case CheckSumNotMatch:
                        value.valS = F("Check Sum Not Match");
                        break;
                        case FileIndexOut:
                        value.valS = F("File Index Out of Bound");
                        break;
                        case FileMismatch:
                        value.valS = F("Cannot Find File");
                        break;
                        case Advertise:
                        value.valS = F("In Advertise");
                        break;
                        default:
                        break;
                    }
                break;
                default:
                break;
            }
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        // реакция на вызов команды модуля из сценария
        // String command - имя команды после ID. (ID.Команда())
        // param - вектор ("массив") значений параметров переданных вместе с командой: ID.Команда("пар1", 22, 33) -> param[0].ValS = "пар1", param[1].ValD = 22

        if (myMP3) {
            if (command == "enableLoop") { 
                myMP3->enableLoop();
            } else if (command == "disableLoop") { 
                myMP3->disableLoop();
            } else if (command == "randomAll") { 
                myMP3->randomAll();
            } else if (command == "stop") { 
                myMP3->stop();
            } else if (command == "volume") { 
                if (param.size()) {
                    myMP3->volume(param[0].valD);
                }
            } else if (command == "playFolder") { 
                if (param.size()) {
                    myMP3->playFolder(param[0].valD, param[1].valD);    // (folderNum, fileNum)
                }
            } else if (command == "play") { 
                myMP3->play(1);  //Play the first mp3
            } else if (command == "next") { 
                myMP3->next();  //Play next mp3
            } else if (command == "previous") { 
                myMP3->previous();  //Play previous mp3
            }
        }

        return {};  // команда поддерживает возвращаемое значения. Т.е. по итогу выполнения команды или общения с внешней системой, можно вернуть значение в сценарий для дальнейшей обработки
    }

    ~Mp3() {};
};

void* getAPI_Mp3(String subtype, String param) {
    if (subtype == F("Mp3")) {
        return new Mp3(param);
    } else {
        return nullptr;
    }
}
