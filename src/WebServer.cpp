#include "HttpServer.h"
#include "BufferExecute.h"
#include "Utils/FileUtils.h"
#include "Utils/WebUtils.h"
#include "FSEditor.h"
#include "Upgrade.h"
#include "Class/NotAsync.h"
#include "items/vLogging.h"

int   flagq ;
AsyncWebSocket ws ("/ws");
AsyncEventSource events("/events");
namespace HttpServer {


/* Forward declaration */
void initOta();
void initMDNS();
void initWS();

void init() {
    String login = jsonReadStr(configSetupJson, "weblogin");
    String pass = jsonReadStr(configSetupJson, "webpass");
#ifdef ESP32
    server.addHandler(new FSEditor(FileFS, login, pass));
#else
    server.addHandler(new FSEditor(login, pass));
#endif

    server.serveStatic("/css/", FileFS, "/css/").setCacheControl("max-age=600");
    server.serveStatic("/js/", FileFS, "/js/").setCacheControl("max-age=600");
    server.serveStatic("/favicon.ico", FileFS, "/favicon.ico").setCacheControl("max-age=600");
    server.serveStatic("/icon.jpeg", FileFS, "/icon.jpeg").setCacheControl("max-age=600");
    server.serveStatic("/edit", FileFS, "/edit").setCacheControl("max-age=600");

#ifdef svelte
    server.serveStatic("/", FileFS, "/").setDefaultFile("index.html").setAuthentication(login.c_str(), pass.c_str());
#else
    server.serveStatic("/", FileFS, "/").setDefaultFile("index.htm").setAuthentication(login.c_str(), pass.c_str());
#endif

    server.onNotFound([](AsyncWebServerRequest *request) {
        SerialPrint("[E]","WebServer","not found:\n" + getRequestInfo(request));
        request->send(404);
    });

    server.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
        // TODO
        if (!index) {
            SerialPrint("I","WebServer","start upload " + filename);
        }
        if (final) {
            SerialPrint("I","WebServer","finish upload: " + prettyBytes(index + len));
        }
    });

    // динамические данные
    server.on("/config.live.json", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", configLiveJson);
    });

    server.on("/config.store.json", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", configStoreJson);
    });

    // данные не являющиеся событиями
    server.on("/config.option.json", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", configOptionJson);
    });

    // для хранения постоянных данных
    server.on("/config.setup.json", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", configSetupJson);
    });

    server.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request) {
        String cmdStr = request->getParam("command")->value();
        SerialPrint("I","WebServer","do: " + cmdStr);
        loopCmdAdd(cmdStr);
        request->send(200, "text/html", "OK");
    });

    

    server.begin();

    initOta();
    initMDNS();
    initWS();

    SerialPrint("I", F("HTTP"), F("HttpServer Init"));
     sCmd.addCommand("GET", sendGetMsg);
}
///////////

    String ID_name = "";
    String ID_value = "";
    String GetMassage = "";

void sendGetMsg() {
    String sabject = sCmd.next();
    String msg = sCmd.next();
    if ((WiFi.status() == WL_CONNECTED)) {
    String mes1 = "";    
    String mes2 = "";


    if(msg){
        // второе значение просто текст
        SerialPrint("GET", F("есть второе значение "), msg);
        GetMassage = msg;
        }
    if(getValue(msg) != "no value"){
    // получено значение виджета
    SerialPrint("GET", F("получено значение виджета "),  getValue(msg));
    GetMassage = getValue(msg);
        }


if (msg.indexOf("+") != -1) {
    // если сложение
    mes1 = selectFromMarkerToMarker(msg, "+", 0);
    mes2 = selectFromMarkerToMarker(msg, "+", 1);
    if (getValue(mes1)  != "no value" and getValue(mes2)  != "no value" ){
       SerialPrint("GET", F("Оба значения виджеты"),  getValue(msg));
       GetMassage = (getValue(mes1).toInt()+getValue(mes2).toInt()); 
    } else if (getValue(mes1)  != "no value"){
        SerialPrint("GET", F("первое значение витджет второе число "),  getValue(mes1));
        GetMassage = (getValue(mes1).toInt() + mes2.toInt()); 
    } else if (getValue(mes1)  != "no value"){
        SerialPrint("GET", F("первое значение число второе витджет "),  getValue(mes2));
        GetMassage = (mes1.toInt()) + getValue(mes2).toInt(); 
    }
}else if (msg.indexOf("-") != -1) {
    // если вычитание 
    mes1 = selectFromMarkerToMarker(msg, "-", 0);
    mes2 = selectFromMarkerToMarker(msg, "-", 1);
    if (getValue(mes1)  != "no value" and getValue(mes2)  != "no value" ){
        GetMassage = (getValue(mes1).toInt()-getValue(mes2).toInt()); 
    } else if (getValue(mes1)  != "no value"){
        GetMassage = (getValue(mes1).toInt() - mes2.toInt()); 
    } else if (getValue(mes1)  != "no value"){
        GetMassage = (mes1.toInt()) - getValue(mes2).toInt(); 
    }
}else if (msg.indexOf("*") != -1) {
    // если умножение 
    mes1 = selectFromMarkerToMarker(msg, "*", 0);
    mes2 = selectFromMarkerToMarker(msg, "*", 1);
    if (getValue(mes1)  != "no value" and getValue(mes2)  != "no value" ){
        GetMassage = (getValue(mes1).toInt() * getValue(mes2).toInt()); 
    } else if (getValue(mes1)  != "no value"){
        GetMassage = (getValue(mes1).toInt() * mes2.toInt()); 
    } else if (getValue(mes1)  != "no value"){
        GetMassage = (mes1.toInt()) * getValue(mes2).toInt(); 
    }
}else if (msg.indexOf("/") != -1) {
    // если деление 
    mes1 = selectFromMarkerToMarker(msg, "/", 0);
    mes2 = selectFromMarkerToMarker(msg, "/", 1);
    if (getValue(mes1)  != "no value" and getValue(mes2)  != "no value" ){
        GetMassage = (getValue(mes1).toInt() / getValue(mes2).toInt()); 
    } else if (getValue(mes1)  != "no value"){
        GetMassage = (getValue(mes1).toInt() / mes2.toInt()); 
    } else if (getValue(mes1)  != "no value"){
        GetMassage = (mes1.toInt()) / getValue(mes2).toInt(); 
    }
}

    String res = getURL(sabject+GetMassage);
    if (res != "error") {
     SerialPrint("<-", F("GET"), "result  " + res);
    }else{
        SerialPrint("<-", F("GET ошибка "),  sabject+GetMassage);
    }

      
     }
}


#ifndef LAYOUT_IN_RAM
void publishWidgetsWS() {
     String str="";
    auto file = seekFile("layout.txt");
    if (!file) {
        SerialPrint("E", F("WS"), F("no file layout.txt"));
        return;
    }
    while (file.available()) {
        
         String payload = file.readStringUntil('\n');
        if (str==""){
        str += payload;
        }else {str += "," + payload;}
    
    }
    file.close();
     SerialPrint("I", "WS", "widgets: " + str);
     ws.textAll(str);

     
 
}

void publishStateWS() {
 String mystr="";
     String str;
    if (configLiveJson != "{}") {
        str += configLiveJson;
    }
    if (configStoreJson != "{}") {
        str += "," + configStoreJson;
    }
  


    str.replace("{", "");
    str.replace("}", "");
    str.replace("\"", "");
    str += ",";

    while (str.length() != 0) {
        String tmp = selectToMarker(str, ",");
        String topic = selectToMarker(tmp, ":");
        String state = deleteBeforeDelimiter(tmp, ":");


        if (topic != "" && state != "") {
         
        String json = "{}";
    jsonWriteStr(json, "status", state);
    jsonWriteStr(json, "topic", mqttRootDevice + "/" + topic + "/status"); 
    SerialPrint("I", "ws", json.c_str());
       if (mystr==""){
        mystr += json;
        }else {mystr += "," + json;}
    
           
        }
        str = deleteBeforeDelimiter(str, ",");
    }
    ws.textAll(mystr);

}
/*
boolean publishStatusWS(const String& topic, const String& data)  {
    String path = mqttRootDevice + "/" + topic + "/status";
    String json = "{}";
    jsonWriteStr(json, "status", data);
    String MyJson = json; 
    jsonWriteStr(MyJson, "topic", path); 
    ws.textAll(MyJson);
  
}
*/
 boolean publishAnyJsonKeyWS(const String& topic, const String& key, const String& data) {
    String path = mqttRootDevice + "/" + topic + "/status";
    String json = "{}";
    jsonWriteStr(json, key, data);
   //добавляем топик, выводим в ws
     String MyJson = json; 
        jsonWriteStr(MyJson, "topic", path); 
        ws.textAll(MyJson);
}
#endif

/////////
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
#ifdef WEBSOCKET_ENABLED
    if (type == WS_EVT_CONNECT) {
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    //    client->printf(json.c_str(), client->id());
        //client->ping();
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
    } else if (type == WS_EVT_ERROR) {
        Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
    } else if (type == WS_EVT_PONG) {
        Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        String msg = "";
        if (info->final && info->index == 0 && info->len == len) {
            //the whole message is in a single frame and we got all of it's data
            Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

            if (info->opcode == WS_TEXT) {
                for (size_t i = 0; i < info->len; i++) {
                    msg += (char)data[i];
                }
            } else {
                char buff[3];
                for (size_t i = 0; i < info->len; i++) {
                    sprintf(buff, "%02x ", (uint8_t)data[i]);
                    msg += buff;
                }
            }
            Serial.printf("%s\n", msg.c_str());
   //->
//если пришло HELLO публикуем виджеты и статусы
        if (msg.startsWith("HELLO")) {
        SerialPrint("I", F("WS"), F("Full update"));
        publishWidgetsWS();
        publishStateWS();
        flagq = 1;
          }
//если пришло UPGRADE
        if (msg.startsWith("upgrade")) {
    // SerialPrint("I", F("WS"), F("do_UPGRADEstart"));
     myNotAsyncActions->make(do_UPGRADE); 
       String temp = jsonWriteInt(msg, "upgrade", 1 );
    ws.textAll(temp);
          }
 
       String upgrade = jsonReadStr(msg, "upgrade");
    
        if (upgrade != ""){
      //  SerialPrint("I", F("WS"), F("do_UPGRADE"));
        myNotAsyncActions->make(do_UPGRADE);    
     String temp = jsonWriteInt(msg, "upgrade", 1 );
    ws.textAll(temp);
          }


 
//отправляем конфигурацию  на ESP 
    if (msg.startsWith("getconfigsetupjson")) {
        ws.textAll(configSetupJson);
       // Serial.printf("%s\n", configSetupJson.c_str());
    }
//отправляем NetworkMap в ESP
  if (msg.startsWith("getNetworkMap")) {
        String  NetworkMap= readFile("NetworkMap.json", 409600);
       // NetworkMap = NetworkMap.replace("\r\n", "");
        ws.textAll(NetworkMap);
 
        }

//отправляем файл в ESP
    String getFileName = jsonReadStr(msg, "getFileName");
   if (getFileName !=""){
          String  getFile= readFile(getFileName, 409600);
       // getFile.replace("\n", " ");
        getFile="{getFileText:'"+getFile+"', getFileName: '"+getFileName+"'}";
        jsonWriteStr(getFile, "getFileName", getFileName); 
         ws.textAll(getFile);
         Serial.printf("%s\n getFileName: ", getFileName.c_str());
         Serial.printf("%s\n getFile: ", getFile.c_str());
  }
//получаем файл  от ESP
    
    String setFileName = jsonReadStr(msg, "setFileName");
   if (setFileName !=""){
      

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(msg);

    root.remove("setFileName");
    String newmsg;  
    root.printTo(newmsg);
    
    
     writeFile(setFileName,newmsg);
     //Serial.printf("%s\n setFileName: ", setFileName.c_str());
     Serial.printf("%s\n text: ", newmsg.c_str());
     }        

//получаем NetworkMap от ESP
    String NetworkMap = jsonReadStr(msg, "NetworkMap");
   if (NetworkMap !=""){
     writeFile("NetworkMap.json", NetworkMap.c_str());
    // ws.textAll(NetworkMap);
   }
       //перезагрузка
 String command = jsonReadStr(msg, "command");
   if (command !="" && jsonReadStr(msg, "command")=="reboot"){
 ESP.restart();   
   }
 


//получаем конфигурацию от ESP

    String changeConf = jsonReadStr(msg, "setconfigsetupjson");
   if (changeConf = "1"){

        jsonWriteStr(configSetupJson, jsonReadStr(msg, "name"), jsonReadStr(msg, "val"));
        saveConfig();
        Serial.printf("%s\n",  jsonReadStr(msg, "name").c_str(), jsonReadStr(msg, "val").c_str());
   }
   


// если пришло в control управляем 
    String path = jsonReadStr(msg, "path");
    String status = jsonReadStr(msg, "status"); 
      
if (path.indexOf("control") != -1) {
        String key = selectFromMarkerToMarker(path, "/", 3);

        String order;
        order += key;
        order += " ";
        order += status;
        order += ",";

        loopCmdAdd(order);
      
    }
          



            if (info->opcode == WS_TEXT)
                client->text("{}");
            else
                client->binary("{}");
        } else {
            //message is comprised of multiple frames or the frame is split into multiple packets
            if (info->index == 0) {
                if (info->num == 0)
                    Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
            }

            Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

            if (info->opcode == WS_TEXT) {
                for (size_t i = 0; i < len; i++) {
                    msg += (char)data[i];
                }
            } else {
                char buff[3];
                for (size_t i = 0; i < len; i++) {
                    sprintf(buff, "%02x ", (uint8_t)data[i]);
                    msg += buff;
                }
            }
            Serial.printf("%s\n", msg.c_str());

            if ((info->index + len) == info->len) {
                Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
                if (info->final) {
                    Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                    if (info->message_opcode == WS_TEXT)
                        client->text("I got your text message");
                    else
                        client->binary("I got your binary message");
                }
            }
        }
    }
#endif
    ;
}

void initMDNS() {
#ifdef MDNS_ENABLED
    MDNS.addService("http", "tcp", 80);
    // TODO Add Adduino OTA
#endif
    ;
}

void initOta() {
#ifdef OTA_UPDATES_ENABLED
    ArduinoOTA.onStart([]() {
        events.send("Update Start", "ota");
    });
    ArduinoOTA.onEnd([]() {
        events.send("Update End", "ota");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        char p[32];
        sprintf(p, "Progress: %u%%\n", (progress / (total / 100)));
        events.send(p, "ota");
    });
    ArduinoOTA.onError([](ota_error_t error) {
        if (error == OTA_AUTH_ERROR)
            events.send("Auth Failed", "ota");
        else if (error == OTA_BEGIN_ERROR)
            events.send("Begin Failed", "ota");
        else if (error == OTA_CONNECT_ERROR)
            events.send("Connect Failed", "ota");
        else if (error == OTA_RECEIVE_ERROR)
            events.send("Recieve Failed", "ota");
        else if (error == OTA_END_ERROR)
            events.send("End Failed", "ota");
    });
    ArduinoOTA.setHostname(hostName);
    ArduinoOTA.begin();
#endif
    ;
}

void initWS() {
#ifdef WEBSOCKET_ENABLED
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    events.onConnect([](AsyncEventSourceClient *client) {
        client->send("", NULL, millis(), 1000);
    });
    server.addHandler(&events);
#endif
    ;
}

}  // namespace HttpServer