#include "Telegram.h"
#include "Global.h"


CTBot* myBot{ nullptr };

void telegramInit() {
    String token = jsonReadStr(configSetupJson, "telegramApi");
    if (!myBot) {
        myBot = new CTBot();
    }
    myBot->setTelegramToken(token);
    if (myBot->testConnection()) {
        SerialPrint("I", "Telegram", "Connected");
    }
    else {
        SerialPrint("E", "Telegram", "Not connected");
    }
}

void handleTelegram() {

    TBMessage msg;

    static unsigned long prevMillis;
    unsigned long currentMillis = millis();
    unsigned long difference = currentMillis - prevMillis;
    if (difference >= 1000) {
        prevMillis = millis();
        if (myBot->getNewMessage(msg)) {
            Serial.println(msg.text);
            myBot->sendMessage(msg.sender.id, msg.text);
        }
    }
}