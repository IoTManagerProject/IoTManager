#include "Telegram.h"
#include "Global.h"

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
            if (difference >= 1000) {
                prevMillis = millis();
                if (myBot->getNewMessage(msg)) {
                    SerialPrint("->", "Telegram", "chat ID: " + String(msg.sender.id) + ", msg: " + String(msg.text));
                    jsonWriteInt(configSetupJson, "chatId", msg.sender.id);
                    saveConfig();
                    orderBuf += String(msg.text) + ",";
                    myBot->sendMessage(jsonReadInt(configSetupJson, "chatId"), msg.text);
                    SerialPrint("<-", "Telegram", "chat ID: " + String(jsonReadInt(configSetupJson, "chatId")) + ", msg: " + String(msg.text));
                }
            }
        }    
    }
}

void sendTelegramMsg() {
    String msg = sCmd.next();
    msg.replace("#"," ");
    myBot->sendMessage(jsonReadInt(configSetupJson, "chatId"), msg);
    SerialPrint("<-", "Telegram", "chat ID: " + String(jsonReadInt(configSetupJson, "chatId")) + ", msg: " + msg);
}

bool isTelegramEnabled() {
    return jsonReadBool(configSetupJson, "telegonof");
}