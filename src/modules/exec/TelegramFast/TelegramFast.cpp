#include "Global.h"
#include "classes/IoTItem.h"

//#define FB_NO_UNICODE
//#define FB_NO_URLENCODE
//#define FB_NO_OTA
//#define FB_DYNAMIC
//#include <GyverGFX.h>
//#include <CharPlot.h>
#include <FastBot.h>
#include <vector>

// FastBot _myBot;
FastBot *_myBot = nullptr;
FastBot *instanceBot()
{
    if (!_myBot)
    {
        _myBot = new FastBot();
        // ot->begin();
    }
    return _myBot;
}


     String _token;
     String _chatID;
     bool _autos;

class TelegramFast : public IoTItem
{
private:

    bool _receiveMsg;
    String _prevMsg = "";

public:
    TelegramFast(String parameters) : IoTItem(parameters)
    {
        jsonRead(parameters, "token", _token);
        jsonRead(parameters, "autos", _autos);
        jsonRead(parameters, "receiveMsg", _receiveMsg);
        jsonRead(parameters, "chatID", _chatID);
        instanceBot();
        _myBot->attach(telegramMsgParse);

#ifdef ESP32
        //          _myBot->useDNS(true);
#endif

        _myBot->setToken(_token);
        //  _myBot->enableUTF8Encoding(true);
    }

    void loop()
    {
        if (_receiveMsg)
        {
            _myBot->tick();
        }
        // Далее вызов doByInterval для обработки комманд
        if (enableDoByInt)
        {
            currentMillis = millis();
            difference = currentMillis - prevMillis;
            if (difference >= _interval)
            {
                prevMillis = millis();
                this->doByInterval();
            }
        }
    }

    void doByInterval()
    {
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {
        if (command == "sendMsg")
        {
            if (param.size())
            {
                String strTmp;
                if (param[0].isDecimal)
                    strTmp = param[0].valD;
                else
                    strTmp = param[0].valS;
                sendTelegramMsg(false, strTmp);
            }
        }
        else if (command == "sendOftenMsg")
        {
            if (param.size())
            {
                String strTmp;
                if (param[0].isDecimal)
                    strTmp = param[0].valD;
                else
                    strTmp = param[0].valS;
                sendTelegramMsg(true, strTmp);
            }
        }

        return {};
    }

    void static telegramMsgParse(FB_msg &msg)
    {
        //     FB_msg msg;
        SerialPrint("->", F("Telegram"), "chat ID: " + msg.chatID + ", msg: " + msg.text);
        //  _myBot->setChatID(_chatID);
        if (_autos)
        {
            _chatID = msg.chatID;
        }
        if (msg.text.indexOf("set") != -1)
        {
            msg.text = deleteBeforeDelimiter(msg.text, "_");
            generateOrder(selectToMarker(msg.text, "_"), selectToMarkerLast(msg.text, "_"));
            _myBot->replyMessage("order done", msg.messageID, _chatID);
            SerialPrint("<-", F("Telegram"), "chat ID: " + _chatID + ", msg: " + String(msg.text));
        }
        else if (msg.text.indexOf("get") != -1)
        {
            msg.text = deleteBeforeDelimiter(msg.text, "_");
            IoTItem *item = findIoTItem(msg.text);
            if (item)
            {
                _myBot->replyMessage(item->getValue(), msg.messageID, _chatID);
                SerialPrint("<-", F("Telegram"), "chat ID: " + _chatID + ", msg: " + String(msg.text));
            }
        }
        else if (msg.text.indexOf("all") != -1)
        {
           // String list = returnListOfParams();
            String out;
            std::vector<float> vctr;
            for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it)
            {
                if ((*it)->iAmLocal)
                {
                    out = out + (*it)->getID() + ": " + (*it)->getValue() + "\n";
                    vctr.push_back(atoff((*it)->getValue().c_str()));
                  //  _myBot->sendMessage((*it)->getID() + ": " + (*it)->getValue(),  _chatID);
                }
            }
            //_myBot->sendMessage(list,  _chatID);
            SerialPrint("<-", F("Telegram"), "chat ID: " + _chatID + "\n" + out);
        //    _myBot->sendMessage(CharPlot<LINE_X2>(&vctr[0], vctr.size(), 5),  _chatID);
        //    SerialPrint("<-", F("Telegram"), CharPlot<LINE_X2>(&vctr[0], vctr.size(), 10));
        }
        else if (msg.text.indexOf("help") != -1)
        {
            _myBot->sendMessage("ID: " + chipId, _chatID);
            _myBot->sendMessage("chatID: " + _chatID,  _chatID);
            _myBot->sendMessage(F("Wrong order, use /all to get all values, /get_id to get value, or /set_id_value to set value"), _chatID);
        }
        else
        {
          //  setValue(msg.text);
        }
    }
/*
    String static returnListOfParams()
    {
        String out;
        for (std::list<IoTItem *>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it)
        {
            if ((*it)->iAmLocal)
                out = out + (*it)->getID() + ": " + (*it)->getValue() + "\n";
        }
        return out;
    }
*/
    void sendTelegramMsg(bool often, String msg)
    {
        if (often)
        {
            _myBot->sendMessage(msg, _chatID);
            SerialPrint("<-", F("Telegram"), "chat ID: " + _chatID + ", msg: " + msg);
        }
        else
        {
            if (_prevMsg != msg)
            {
                _prevMsg = msg;
                _myBot->sendMessage(msg, _chatID);
                SerialPrint("<-", F("Telegram"), "chat ID: " + _chatID + ", msg: " + msg);
            }
        }
    }

    ~TelegramFast(){

    };
};

void *getAPI_TelegramFast(String subtype, String param)
{
    if (subtype == F("TelegramFast"))
    {
        return new TelegramFast(param);
    }
    else
    {
        return nullptr;
    }
}

