void Push_init() {
  server.on("/pushingboxDate", HTTP_GET, [](AsyncWebServerRequest * request) {

    if (request->hasArg("pushingbox_id")) {
      jsonWriteStr(configSetup, "pushingbox_id", request->getParam("pushingbox_id")->value());
    }

    saveConfig();

    request->send(200, "text/text", "ok"); // отправляем ответ о выполнении
  });
}

void pushControl() {

  String title = sCmd.next();
  title.replace("#", " ");
  String body = sCmd.next();
  body.replace("#", " ");

  static String body_old;

  const char* logServer = "api.pushingbox.com";
  String deviceId = jsonRead(configSetup, "pushingbox_id");

  Serial.println("- starting client");

  WiFiClient client;

  Serial.println("- connecting to pushing server: " + String(logServer));
  if (client.connect(logServer, 80)) {
    Serial.println("- succesfully connected");

    String postStr = "devid=";
    postStr += String(deviceId);

    postStr += "&title=";
    postStr += String(title);

    postStr += "&body=";
    postStr += String(body);

    postStr += "\r\n\r\n";

    Serial.println("- sending data...");

    client.print("POST /pushingbox HTTP/1.1\n");
    client.print("Host: api.pushingbox.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();
  Serial.println("- stopping the client");
}
