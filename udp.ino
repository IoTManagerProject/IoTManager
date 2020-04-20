void UDP_init() {
  server.on("/udp", HTTP_GET, [](AsyncWebServerRequest * request) {
    String value;
    if (request->hasArg("arg")) {
      value = request->getParam("arg")->value();
    }
    if (value == "1") {
      mqtt_send_settings_to_udp = true;
      request->send(200, "text/text", "ok");
    }
    if (value == "2") {
      SPIFFS.remove("/dev.csv");
      addFile("dev.csv", "device id;device name;ip address");
      request->redirect("/?dev");
    }
    if (value == "3") {
      request->redirect("/?dev");
    }
  });
  server.on("/name", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("arg")) {
      jsonWriteStr(configSetup, "name", request->getParam("arg")->value());
      jsonWriteStr(configJson, "name", request->getParam("arg")->value());
      saveConfig();
    }
    request->send(200, "text/text", "OK");
  });

  SPIFFS.remove("/dev.csv");
  addFile("dev.csv", "device id;device name;ip address");

#ifdef ESP8266
  Udp.begin(udp_port);
#endif

  handleUdp_esp32();

  ts.add(UDP, 30000, [&](void*) {
    if (WiFi.status() == WL_CONNECTED) {
      if (!udp_busy) {
        String line_to_send = chipID + ";" + jsonRead(configSetup, "name");
#ifdef ESP8266
        Udp.beginPacketMulticast(udp_multicastIP, udp_port, WiFi.localIP());
        Udp.write(line_to_send.c_str());
        Udp.endPacket();
#endif
#ifdef ESP32
        udp.broadcast(line_to_send.c_str());
#endif
        Serial.println("[UDP<=] dev info send");
      }
    }
  }, nullptr, false);
}

void handleUdp() {
#ifdef ESP8266
  if (WiFi.status() == WL_CONNECTED) {
    int packetSize = Udp.parsePacket();
    if (packetSize) {
      char udp_incomingPacket[255];
      Serial.printf("[UDP=>] Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
      received_ip = Udp.remoteIP().toString();
      int len = Udp.read(udp_incomingPacket, 255);
      if (len > 0) {
        udp_incomingPacket[len] = 0;
      }
      received_udp_line = String(udp_incomingPacket);
      udp_data_parse = true;
    }
  }
#endif
}

void handleUdp_esp32() {
#ifdef ESP32
  if (udp.listenMulticast(udp_multicastIP, udp_port)) {
    udp.onPacket([](AsyncUDPPacket packet) {
      received_udp_line = (char*)packet.data();
      received_ip = packet.remoteIP().toString();
      udp_data_parse = true;
    });
  }
#endif
}

void do_udp_data_parse() {
  if (udp_data_parse) {
    udp_data_parse = false;
    Serial.print("[UDP=>] " + received_ip);
    Serial.print(" ");
    Serial.println(received_udp_line);
    if (received_udp_line.indexOf("mqttServer") > 0) {
      jsonWriteStr(configSetup, "mqttServer", jsonRead(received_udp_line, "mqttServer"));
      jsonWriteInt(configSetup, "mqttPort", jsonReadtoInt(received_udp_line, "mqttPort"));
      jsonWriteStr(configSetup, "mqttPrefix", jsonRead(received_udp_line, "mqttPrefix"));
      jsonWriteStr(configSetup, "mqttUser", jsonRead(received_udp_line, "mqttUser"));
      jsonWriteStr(configSetup, "mqttPass", jsonRead(received_udp_line, "mqttPass"));
      saveConfig();
      Serial.println("[V] new mqtt setting received from udp and saved");
      mqtt_connection = true;
    } else {
      add_dev_in_list("dev.csv", selectFromMarkerToMarker(received_udp_line, ";", 0), selectFromMarkerToMarker(received_udp_line, ";", 1), received_ip);
    }
  }
}

void add_dev_in_list(String fileName, String id, String dev_name, String ip) {
  File configFile = SPIFFS.open("/" + fileName, "r");
  if (!configFile.find(id.c_str())) {
    addFile(fileName, id + ";" + dev_name + "; <a href=\"http://" + ip + "\" target=\"_blank\"\">" + ip + "</a>");
  }
}

void send_mqtt_to_udp() {
  if (WiFi.status() == WL_CONNECTED) {
    udp_busy = true;
    String mqtt_data = "{}";
    jsonWriteStr(mqtt_data, "mqttServer", jsonRead(configSetup, "mqttServer"));
    jsonWriteInt(mqtt_data, "mqttPort", jsonReadtoInt(configSetup, "mqttPort"));
    jsonWriteStr(mqtt_data, "mqttPrefix", jsonRead(configSetup, "mqttPrefix"));
    jsonWriteStr(mqtt_data, "mqttUser", jsonRead(configSetup, "mqttUser"));
    jsonWriteStr(mqtt_data, "mqttPass", jsonRead(configSetup, "mqttPass"));
    Serial.println(mqtt_data);
#ifdef ESP8266
    Udp.beginPacketMulticast(udp_multicastIP, udp_port, WiFi.localIP());
    Udp.write(mqtt_data.c_str());
    Udp.endPacket();
#endif
#ifdef ESP32
    udp.broadcast(mqtt_data.c_str());
#endif
    Serial.println("[UDP<=] mqtt info send");
    udp_busy = false;
  }
}
