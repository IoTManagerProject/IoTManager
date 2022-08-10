#include "Global.h"
#include "classes/IoTItem.h"

#include "CTBot.h"

String uint64ToString(uint64_t input) {
  String result = "";
  uint8_t base = 10;

  do {
    char c = input % base;
    input /= base;

    if (c < 10)
      c +='0';
    else
      c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}


class Telegram : public IoTItem {
   private:
    CTBot _myBot;
    String _token;
    bool _autos;
    bool _receiveMsg;
    int64_t _chatID;

    String _prevMsg = "";

    void telegramMsgParse(String msg) {
        if (msg.indexOf("set") != -1) {
            msg = deleteBeforeDelimiter(msg, "_");
            generateOrder(selectToMarker(msg, "_"), selectToMarkerLast(msg, "_"));
            _myBot.sendMessage(_chatID, "order done");
            SerialPrint("<-", F("Telegram"), "chat ID: " + uint64ToString(_chatID) + ", msg: " + String(msg));
        } else if (msg.indexOf("get") != -1) {
            msg = deleteBeforeDelimiter(msg, "_");
            IoTItem* item = findIoTItem(msg);
            if (item) {
                _myBot.sendMessage(_chatID, item->getValue()); 
                SerialPrint("<-", F("Telegram"), "chat ID: " + uint64ToString(_chatID) + ", msg: " + String(msg));
            }
        } else if (msg.indexOf("all") != -1) {
            String list = returnListOfParams();
            _myBot.sendMessage(_chatID, list);
            SerialPrint("<-", F("Telegram"), "chat ID: " + uint64ToString(_chatID) + "\n" + list);
        } else if (msg.indexOf("help") != -1) {
            _myBot.sendMessage(_chatID, "ID: " + chipId);
            _myBot.sendMessage(_chatID, "chatID: " + uint64ToString(_chatID));
            _myBot.sendMessage(_chatID, F("Wrong order, use /all to get all values, /get_id to get value, or /set_id_value to set value"));
        } else {
            setValue(msg);
        }
    }

    String returnListOfParams() {
        String out;
        for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
            if ((*it)->iAmLocal) out = out + (*it)->getID() + ": " + (*it)->getValue() + "\n";
        }
        return out;
    }   

   public:
    Telegram(String parameters): IoTItem(parameters) {
        jsonRead(parameters, "token", _token);
        jsonRead(parameters, "autos", _autos);
        jsonRead(parameters, "receiveMsg", _receiveMsg);

        String tmp;
        jsonRead(parameters, "chatID", tmp);
        _chatID = atoll(tmp.c_str());
        
        #ifdef ESP32
             _myBot.useDNS(true);
        #endif

        _myBot.setTelegramToken(_token);
        _myBot.enableUTF8Encoding(true);
    }

    void doByInterval() {
        if (_receiveMsg) {
            TBMessage msg;
            if (_myBot.getNewMessage(msg)) {
                SerialPrint("->", F("Telegram"), "chat ID: " + uint64ToString(msg.sender.id) + ", msg: " + msg.text);
                if (_autos) {
                    _chatID = msg.sender.id;
                }
                telegramMsgParse(String(msg.text));
            }
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (command == "sendMsg") { 
            if (param.size()) {
                String strTmp;
                if (param[0].isDecimal) strTmp = param[0].valD; else strTmp = param[0].valS;
                sendTelegramMsg(false, strTmp);
            } 
        } else if (command == "sendOftenMsg") {
            if (param.size()) {
                String strTmp;
                if (param[0].isDecimal) strTmp = param[0].valD; else strTmp = param[0].valS;
                sendTelegramMsg(true, strTmp);
            }
        }

        return {}; 
    }

    void sendTelegramMsg(bool often, String msg) {
        if (often) {
            _myBot.sendMessage(_chatID, msg);
            SerialPrint("<-", F("Telegram"), "chat ID: " + uint64ToString(_chatID) + ", msg: " + msg);
        } else {
            if (_prevMsg != msg) {
                _prevMsg = msg;
                _myBot.sendMessage(_chatID, msg);
                SerialPrint("<-", F("Telegram"), "chat ID: " + uint64ToString(_chatID) + ", msg: " + msg);
            }
        }
    }

    ~Telegram() {

    };
};

void* getAPI_Telegram(String subtype, String param) {
    if (subtype == F("Telegram")) {
        return new Telegram(param);
    } else {
        return nullptr;
    }
}
