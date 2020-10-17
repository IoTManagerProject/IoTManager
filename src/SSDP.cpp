
#include <SSDP.h>
#include "Global.h"
#ifdef SSDP_EN
#ifdef ESP8266
        #include <ESP8266SSDP.h>
#endif
#ifdef ESP32
        #include <ESP32SSDP.h>
#endif
//39164
//457684
void SsdpInit() {
    server.on("/description.xml", HTTP_GET, [](AsyncWebServerRequest* request) {
        String ssdpSend = F("<root xmlns=\"urn:schemas-upnp-org:device-1-0\">");
        String ssdpHeder = xmlNode(F("major"), "1");
        ssdpHeder += xmlNode(F("minor"), "0");
        ssdpHeder = xmlNode(F("specVersion"), ssdpHeder);
        ssdpHeder += xmlNode(F("URLBase"), "http://" + WiFi.localIP().toString());
        String ssdpDescription = xmlNode(F("deviceType"), F("upnp:rootdevice"));
        ssdpDescription += xmlNode(F("friendlyName"), jsonReadStr(configSetupJson, F("name")));
        ssdpDescription += xmlNode(F("presentationURL"), "/");
        ssdpDescription += xmlNode(F("serialNumber"), getChipId());
#ifdef ESP8266
        ssdpDescription += xmlNode(F("modelName"), F("ESP8266"));
#endif
#ifdef ESP32
        ssdpDescription += xmlNode(F("modelName"), F("ESP32"));
#endif
        ssdpDescription += xmlNode(F("modelNumber"), getChipId());
        ssdpDescription += xmlNode(F("modelURL"), F("https://github.com/IoTManagerProject/IoTManager/wiki"));
        ssdpDescription += xmlNode(F("manufacturer"), F("Borisenko Dmitry"));
        ssdpDescription += xmlNode(F("manufacturerURL"), F("https://github.com/IoTManagerProject/IoTManager"));
        ssdpDescription += xmlNode(F("UDN"), "uuid:38323636-4558-4dda-9188-cda0e6" + decToHex(ESP.getChipId(), 6));
        ssdpDescription = xmlNode("device", ssdpDescription);
        ssdpHeder += ssdpDescription;
        ssdpSend += ssdpHeder;
        ssdpSend += "</root>";
        Serial.println("->!!!SSDP Get request received");
        request->send(200, "text/xml", ssdpSend);
    });
    //Если версия  2.0.0 закаментируйте следующую строчку
    SSDP.setDeviceType(F("upnp:rootdevice"));
    SSDP.setSchemaURL(F("description.xml"));
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
#endif