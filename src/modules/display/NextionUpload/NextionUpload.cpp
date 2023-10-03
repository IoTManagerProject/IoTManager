
#define DEBUG_SERIAL_ENABLE
#include "Global.h"
#include "classes/IoTItem.h"
#include "ESPNexUpload.h"
bool updated = false;
// const char *host = "live-control.com";
// const char *url = "/iotm/Live-Control.tft";
extern IoTGpio IoTgpio;

class NextionUpload : public IoTItem
{
private:
    String _url;
    String _host;
    int _NEXT_RX;
    int _NEXT_TX;

public:
    NextionUpload(String parameters) : IoTItem(parameters)
    {
        _url = jsonReadStr(parameters, "url");
        _host = jsonReadStr(parameters, "host");

        _NEXT_RX = jsonReadInt(parameters, "NEXT_RX");
        _NEXT_TX = jsonReadInt(parameters, "NEXT_TX");
#define NEXT_RX _NEXT_RX // Nextion RX pin | Default 16
#define NEXT_TX _NEXT_TX // Nextion TX pin | Default 17
    }

    IoTValue execute(String command, std::vector<IoTValue> &param)
    {

        if (command == "Update")
        {
            SerialPrint("I", F("NextionUpdate"), "Update .... ");

            if (!updated)
            {
                SerialPrint("I", F("NextionUpdate"), "connecting to  " + (String)_host);
                HTTPClient http;

#if defined ESP8266
                if (!http.begin(_host, 80, _url))
                {
                    //  Serial.println("connection failed");
                    SerialPrint("I", F("NextionUpdate"), "connection failed  ");
                }
#elif defined ESP32
                if (!http.begin(String("http://") + _host + _url))
                {
                    //  Serial.println("connection failed");
                    SerialPrint("I", F("NextionUpdate"), "connection failed  ");
                }
#endif

                SerialPrint("I", F("NextionUpdate"), "Requesting file:   " + (String)_url);
                int code = http.GET();
                int contentLength = http.getSize();

                // Update the nextion display
                if (code == 200)
                {
                    SerialPrint("I", F("NextionUpdate"), "File received. Update Nextion...   ");
                    bool result;
                    ESPNexUpload nextion(115200);
                    nextion.setUpdateProgressCallback([]()
                                                      { SerialPrint("I", F("NextionUpdate"), "...   "); });

                    result = nextion.prepareUpload(contentLength);

                    if (!result)
                    {
                        SerialPrint("I", F("NextionUpdate"), "Error:    " + (String)nextion.statusMessage);
                    }
                    else
                    {
                        SerialPrint("I", F("NextionUpdate"), "Start upload. File size is:     " + (String)contentLength);
                        result = nextion.upload(*http.getStreamPtr());

                        if (result)
                        {
                            updated = true;
                            SerialPrint("I", F("NextionUpdate"), "Succesfully updated Nextion!     ");
                        }
                        else
                        {
                            SerialPrint("I", F("NextionUpdate"), "Error updating Nextion:      " + (String)nextion.statusMessage);
                        }

                        nextion.end();
                    }
                }
                else
                {
                    SerialPrint("I", F("NextionUpdate"), "HTTP error:    " + (String)http.errorToString(code).c_str());
                }

                http.end();
                SerialPrint("I", F("NextionUpdate"), "Closing connection    ");
            }
        }

        return {};
    }

    ~NextionUpload(){};
};

void *getAPI_NextionUpload(String subtype, String param)
{
    if (subtype == F("NextionUpload"))
    {
        return new NextionUpload(param);
    }
    else
    {
        return nullptr;
    }
}
