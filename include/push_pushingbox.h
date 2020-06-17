#pragma once

#include "main.h"
#include "set.h"

inline void Push_init() {
    server.on("/pushingboxDate", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (request->hasArg("pushingbox_id")) {
            jsonWriteStr(configSetup, "pushingbox_id", request->getParam("pushingbox_id")->value());
        }

        saveConfig();

        request->send(200, "text/text", "ok");  // отправляем ответ о выполнении
    });
}

inline void pushControl() {
    String title = sCmd.next();
    title.replace("#", " ");
    String body = sCmd.next();
    body.replace("#", " ");

    static String body_old;

    const char* logServer = "api.pushingbox.com";
    String deviceId = jsonReadStr(configSetup, "pushingbox_id");

    Serial.println("- starting client");

    WiFiClient client_push;

    Serial.println("- connecting to pushing server: " + String(logServer));
    if (!client_push.connect(logServer, 80)) {
        Serial.println("- not connected");
    } else {
        Serial.println("- succesfully connected");

        String postStr = "devid=";
        postStr += String(deviceId);

        postStr += "&title=";
        postStr += String(title);

        postStr += "&body=";
        postStr += String(body);

        postStr += "\r\n\r\n";

        Serial.println("- sending data...");

        client_push.print("POST /pushingbox HTTP/1.1\n");
        client_push.print("Host: api.pushingbox.com\n");
        client_push.print("Connection: close\n");
        client_push.print("Content-Type: application/x-www-form-urlencoded\n");
        client_push.print("Content-Length: ");
        client_push.print(postStr.length());
        client_push.print("\n\n");
        client_push.print(postStr);
    }
    client_push.stop();
    Serial.println("- stopping the client");
}
