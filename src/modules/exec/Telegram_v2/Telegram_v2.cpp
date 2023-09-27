#include "Global.h"
#include "classes/IoTItem.h"

// #define FB_NO_UNICODE
// #define FB_NO_URLENCODE
// #define FB_NO_OTA
// #define FB_DYNAMIC
// #include <GyverGFX.h>
// #include <CharPlot.h>
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

class Telegram_v2 : public IoTItem
{
private:
    bool _receiveMsg;
    String _prevMsg = "";

public:
    Telegram_v2(String parameters) : IoTItem(parameters)
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
        _myBot->setChatID(_chatID);
        //        _myBot->showMenuText("help","help",false);
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
        else if (command == "sendPinMsg")
        {
            if (param.size())
            {
                String strTmp;
                if (param[0].isDecimal)
                    strTmp = param[0].valD;
                else
                    strTmp = param[0].valS;
                _myBot->sendMessage(strTmp, _chatID);
                _myBot->pinMessage(_myBot->lastBotMsg());

                SerialPrint("<-", F("Telegram"), "chat ID: " + _chatID + ",pin msg: " + strTmp);
            }
        }
        else if (command == "editMsg")
        {
            if (param.size())
            {
                String strTmp;
                if (param[0].isDecimal)
                    strTmp = param[0].valD;
                else
                    strTmp = param[0].valS;
                _myBot->editMessage(_myBot->lastBotMsg(), strTmp);
                SerialPrint("<-", F("Telegram"), "chat ID: " + _chatID + ",edit msg: " + strTmp);
            }
        }
        else if (command == "sendFile")
        {
            if (param.size() && !param[0].isDecimal)
            {
                //     String path = filepath(filename);
                auto file = FileFS.open(param[0].valS, FILE_READ);
                if (!file)
                {
                    SerialPrint("X", F("Telegram"), "Fail send file: " + param[0].valS);
                    return {};
                }
                //  File file = LittleFS.open(param[0].valS, "r"); // /test.png
                // selectToMarkerLast(msg.text, "_")
                uint8_t res = _myBot->sendFile(file, (FB_FileType)param[1].valD, selectToMarkerLast(param[0].valS, "/"), _chatID);
                file.close();
                SerialPrint("<-", F("Telegram"), "chat ID: " + _chatID + ", sendFile: " + param[0].valS + " res: " + String(res));
            }
        }

        else if (command == "editFile")
        {
            if (param.size() && !param[0].isDecimal)
            {
                //     String path = filepath(filename);
                auto file = FileFS.open(param[0].valS, FILE_READ);
                if (!file)
                {
                    SerialPrint("X", F("Telegram"), "Fail edit file: " + param[0].valS);
                    return {};
                }
                //  File file = LittleFS.open(param[0].valS, "r"); // /test.png
                // selectToMarkerLast(msg.text, "_")
                uint8_t res = _myBot->editFile(file, (FB_FileType)param[1].valD, selectToMarkerLast(param[0].valS, "/"), _myBot->lastBotMsg(), _chatID);
                file.close();
                SerialPrint("<-", F("Telegram"), "chat ID: " + _chatID + ", editFile: " + param[0].valS + " res: " + String(res));
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
                    if (it == IoTItems.begin())
                    {
                        out = "get_" + (*it)->getID();
                    }
                    else
                    {
                        out = out + " \n " + "get_" + (*it)->getID();
                    }
                    vctr.push_back(atoff((*it)->getValue().c_str()));
                    //  _myBot->sendMessage((*it)->getID() + ": " + (*it)->getValue(),  _chatID);
                }
            }
            _myBot->showMenuText("select Id", out, true);
            SerialPrint("<-", F("Telegram"), "chat ID: " + _chatID + "\n" + out);
            //    _myBot->sendMessage(CharPlot<LINE_X2>(&vctr[0], vctr.size(), 5),  _chatID);
            //    SerialPrint("<-", F("Telegram"), CharPlot<LINE_X2>(&vctr[0], vctr.size(), 10));
        }

        else if (msg.text.indexOf("file") != -1 && msg.chatID == _chatID)
        {
            msg.text = deleteBeforeDelimiter(msg.text, "_");
            SerialPrint("<-", F("Telegram"), "chat ID: " + _chatID + ", get file: " + String(msg.text));
            auto file = FileFS.open(selectToMarker(msg.text, "_"), FILE_READ); // /test.png
            if (!file)
            {
                SerialPrint("X", F("Telegram"), "Fail send file: " + selectToMarker(msg.text, "_"));
                return;
            }
            int type = atoi(selectToMarkerLast(msg.text, "_").c_str());
            _myBot->sendFile(file, (FB_FileType)type, selectToMarker(msg.text, "_"), _chatID);
            file.close();
        }
        else if (msg.isFile)
        {
            if (msg.text.indexOf("download") != -1 && msg.chatID == _chatID)
            {
                String path = '/' + msg.fileName;          // вида /filename.xxx
                auto file = FileFS.open(path, FILE_WRITE); // открываем для записи
                if(!_myBot->downloadFile(file, msg.fileUrl))
                {
                    SerialPrint("X", F("Telegram"), "download from: error write" );
                    return;
                }
                SerialPrint("<-", F("Telegram"), "download from: " + _chatID + ", file: " + String(msg.fileUrl));
                _myBot->sendMessage("download from: " + _chatID + ", file: " + String(msg.fileName), _chatID);
            }
        }
        else if (msg.text.indexOf("help") != -1)
        {
            _myBot->sendMessage("ID: " + chipId, _chatID);
            _myBot->sendMessage("chatID: " + _chatID, _chatID);
            _myBot->sendMessage(F("Wrong order, use /all to get all values, /get_id to get value, /set_id_value to set value, or /file_name_type or send file msg=download"), _chatID);
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

    ~Telegram_v2(){

    };
};

void *getAPI_Telegram_v2(String subtype, String param)
{
    if (subtype == F("Telegram_v2"))
    {
        return new Telegram_v2(param);
    }
    else
    {
        return nullptr;
    }
}
