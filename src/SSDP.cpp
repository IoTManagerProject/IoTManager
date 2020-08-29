#ifdef ESP8266
        #include <ESP8266SSDP.h>
#endif
#ifdef ESP32
        #include <ESP32SSDP.h>
#endif

#include <SSDP.h>
#include "Global.h"

void SsdpInit() {
    server.on("/description.xml", HTTP_GET, [](AsyncWebServerRequest* request) {
        String ssdpSend = "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">";
        String ssdpHeder = xmlNode("major", "1");
        ssdpHeder += xmlNode("minor", "0");
        ssdpHeder = xmlNode("specVersion", ssdpHeder);
        ssdpHeder += xmlNode("URLBase", "http://" + WiFi.localIP().toString());
        String ssdpDescription = xmlNode("deviceType", "upnp:rootdevice");
        ssdpDescription += xmlNode("friendlyName", jsonReadStr(configSetupJson, "name"));
        ssdpDescription += xmlNode("presentationURL", "/");
        ssdpDescription += xmlNode("serialNumber", getChipId());
#ifdef ESP8266
        ssdpDescription += xmlNode("modelName", "ESP8266");
#endif
#ifdef ESP32
        ssdpDescription += xmlNode("modelName", "ESP32");
#endif
        ssdpDescription += xmlNode("modelNumber", getChipId());
        ssdpDescription += xmlNode("modelURL", "https://github.com/IoTManagerProject/IoTManager/wiki");
        ssdpDescription += xmlNode("manufacturer", "Borisenko Dmitry");
        ssdpDescription += xmlNode("manufacturerURL", "https://github.com/IoTManagerProject/IoTManager");
        ssdpDescription += xmlNode("UDN", "uuid:38323636-4558-4dda-9188-cda0e6" + decToHex(ESP.getChipId(), 6));
        ssdpDescription = xmlNode("device", ssdpDescription);
        ssdpHeder += ssdpDescription;
        ssdpSend += ssdpHeder;
        ssdpSend += "</root>";
        Serial.println("->!!!SSDP Get request received");
        request->send(200, "text/xml", ssdpSend);
    });
    //Если версия  2.0.0 закаментируйте следующую строчку
    SSDP.setDeviceType("upnp:rootdevice");
    SSDP.setSchemaURL("description.xml");
    SSDP.begin();
}

String xmlNode(String tags, String data) {
    String temp = "<" + tags + ">" + data + "</" + tags + ">";
    return temp;
}

String decToHex(uint32_t decValue, byte desiredStringLength) {
    String hexString = String(decValue, HEX);
    while (hexString.length() < desiredStringLength) hexString = "0" + hexString;
    return hexString;
}