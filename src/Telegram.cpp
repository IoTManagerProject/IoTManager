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
    SerialPrint("<-", "Telegram", "chat ID: " + String(jsonReadInt(configSetupJson, "chatId")) + ", msg: " + String(msg));
    if (msg.indexOf("order") != -1) {
        msg = deleteBeforeDelimiter(msg, " ");
        orderBuf += String(msg) + ",";
        myBot->sendMessage(jsonReadInt(configSetupJson, "chatId"), "order done");
    }
    else if (msg.indexOf("get") != -1) {
        msg = deleteBeforeDelimiter(msg, " ");
        myBot->sendMessage(jsonReadInt(configSetupJson, "chatId"), jsonReadStr(configLiveJson, msg));
    }
    else {
        myBot->sendMessage(jsonReadInt(configSetupJson, "chatId"), "wrong order, use 'get id' to get value, or 'order id value' to send order");
    }
}

void sendTelegramMsg() {
    String msg = sCmd.next();
    msg.replace("#", " ");
    myBot->sendMessage(jsonReadInt(configSetupJson, "chatId"), msg);
    SerialPrint("<-", "Telegram", "chat ID: " + String(jsonReadInt(configSetupJson, "chatId")) + ", msg: " + msg);
}

bool isTelegramEnabled() {
    return jsonReadBool(configSetupJson, "telegonof");
}