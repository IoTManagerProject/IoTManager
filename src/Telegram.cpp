#include "Telegram.h"


CTBot* myBot{ nullptr };

void telegramInit() {
    if (isTelegramEnabled()) {
        telegramInitBeen = true;
        sCmd.addCommand("telegram", sendTelegramMsg);
        String token = jsonReadStr(configSetupJson, "telegramApi");
        if (!myBot) {
            myBot = new CTBot();
        }
        myBot->setTelegramToken(token);
        myBot->enableUTF8Encoding(true);
        if (myBot->testConnection()) {
            SerialPrint("I", "Telegram", "Connected");
        }
        else {
            SerialPrint("E", "Telegram", "Not connected");
        }
    }
}

void handleTelegram() {
    if (telegramInitBeen) {
        if (isTelegramEnabled()) {
            TBMessage msg;
            static unsigned long prevMillis;
            unsigned long currentMillis = millis();
            unsigned long difference = currentMillis - prevMillis;
            if (difference >= 5000) {
                prevMillis = millis();
                if (myBot->getNewMessage(msg)) {
                    SerialPrint("->", "Telegram", "chat ID: " + String(msg.sender.id) + ", msg: " + String(msg.text));
                    jsonWriteInt(configSetupJson, "chatId", msg.sender.id);
                    saveConfig();
                    telegramMsgParse(String(msg.text));
                }
            }
        }
    }
}

void telegramMsgParse(String msg) {
    if (msg.indexOf("set") != -1) {
        msg = deleteBeforeDelimiter(msg, "_");
        msg.replace("_", " ");
        orderBuf += String(msg) + ",";
        myBot->sendMessage(jsonReadInt(configSetupJson, "chatId"), "order done");
        SerialPrint("<-", "Telegram", "chat ID: " + String(jsonReadInt(configSetupJson, "chatId")) + ", msg: " + String(msg));
    }
    else if (msg.indexOf("get") != -1) {
        msg = deleteBeforeDelimiter(msg, "_");
        myBot->sendMessage(jsonReadInt(configSetupJson, "chatId"), getValue(msg)); //jsonReadStr(configLiveJson , msg));
        SerialPrint("<-", "Telegram", "chat ID: " + String(jsonReadInt(configSetupJson, "chatId")) + ", msg: " + String(msg));
    }
    else if (msg.indexOf("all") != -1) {
        String list = returnListOfParams();
        myBot->sendMessage(jsonReadInt(configSetupJson, "chatId"), list);
        SerialPrint("<-", "Telegram", "chat ID: " + String(jsonReadInt(configSetupJson, "chatId")) + "\n" + list);
    }
    else {
        myBot->sendMessage(jsonReadInt(configSetupJson, "chatId"), F("Wrong order, use /all to get all values, /get_id to get value, or /set_id_value to set value"));
    }
}

void sendTelegramMsg() {
    String msg = sCmd.next();
    String type = sCmd.next();
    msg.replace("#", " ");
    if (type == "1") {
        static String prevMsg;
        if (prevMsg != msg) {
            prevMsg = msg;
            myBot->sendMessage(jsonReadInt(configSetupJson, "chatId"), msg);
            SerialPrint("<-", "Telegram", "chat ID: " + String(jsonReadInt(configSetupJson, "chatId")) + ", msg: " + msg);
        }
    } else if (type == "2") {
        myBot->sendMessage(jsonReadInt(configSetupJson, "chatId"), msg);
        SerialPrint("<-", "Telegram", "chat ID: " + String(jsonReadInt(configSetupJson, "chatId")) + ", msg: " + msg);
    } 
}

bool isTelegramEnabled() {
    return jsonReadBool(configSetupJson, "telegonof");
}


String returnListOfParams() {
    String cmdStr = readFile(DEVICE_CONFIG_FILE, 4096);
    cmdStr += "\r\n";
    cmdStr.replace("\r\n", "\n");
    cmdStr.replace("\r", "\n");
    int count = 0;
    String out;
    while (cmdStr.length()) {
        String buf = selectToMarker(cmdStr, "\n");
        count++;
        if (count > 1) {
            String id = selectFromMarkerToMarker(buf, ";", 2);
            String value = getValue(id); //jsonReadStr(configLiveJson , id);
            String page = selectFromMarkerToMarker(buf, ";", 4);
            page.replace("#", " ");
            String name = selectFromMarkerToMarker(buf, ";", 5);
            name.replace("#", " ");
            out += page + " " + " " + name + " " + value + "\n";
        }
        cmdStr = deleteBeforeDelimiter(cmdStr, "\n");
    }
    return out;
}