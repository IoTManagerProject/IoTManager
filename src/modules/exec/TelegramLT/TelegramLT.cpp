#include "Global.h"
#include "classes/IoTItem.h"

class TelegramLT : public IoTItem {
   public:
    String _prevMsg = "";
    String _token;
    String _chatID;

    TelegramLT(String parameters) : IoTItem(parameters) {
        jsonRead(parameters, "token", _token);
        jsonRead(parameters, "chatID", _chatID);
    }

    void sendTelegramMsg(bool often, String msg) {
        if (WiFi.status() == WL_CONNECTED && (often || !often && _prevMsg != msg)) {
            WiFiClient client;
            HTTPClient http;
            http.begin(client, "http://live-control.com/iotm/telegram.php");
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            String httpRequestData = "url=https://api.telegram.org/bot" + _token + "/sendmessage?chat_id=" + _chatID + "&text=" + msg;
            int httpResponseCode = http.POST(httpRequestData);
            String payload = http.getString();
            SerialPrint("<-", F("Telegram"), "chat ID: " + _chatID + ", msg: " + msg);
            SerialPrint("->", F("Telegram"), "chat ID: " + _chatID + ", server: " + httpResponseCode);

            if (!strstr(payload.c_str(), "{\"ok\":true")) {
                value.valD = 0;
                Serial.printf("Telegram error, msg from server: %s\n", payload.c_str());
                regEvent(value.valD, payload);
            } else {
                value.valD = 1;
                regEvent(value.valD, payload);
            }
            http.end();
            _prevMsg = msg;
        }
    }

    IoTValue execute(String command, std::vector<IoTValue> &param) {
        if (param.size() == 1) {
            String strTmp;
            if (param[0].isDecimal && param[0].valS == "")
                strTmp = param[0].valD;
            else
                strTmp = param[0].valS;

            if (command == "sendMsg") {
                if (param.size()) {
                    sendTelegramMsg(false, strTmp);
                }
            } else if (command == "sendOftenMsg") {
                if (param.size()) {
                    sendTelegramMsg(true, strTmp);
                }
            }
        }
        return {};
    }

    ~TelegramLT(){};
};

void *getAPI_TelegramLT(String subtype, String param) {
    if (subtype == F("TelegramLT")) {
        return new TelegramLT(param);
    } else {
        return nullptr;
    }
}