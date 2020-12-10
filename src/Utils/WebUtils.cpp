#include "Utils/WebUtils.h"
#include "ESPAsyncWebServer.h"

String getURL(const String& urls) {
    String res = "";
    HTTPClient http;
    http.begin(urls);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        res = http.getString();
    }
    else {
        res = "error";
    }
    http.end();
    return res;
}



const String getMethodName(AsyncWebServerRequest* request) {
    String res = F("UNKNOWN");
    if (request->method() == HTTP_GET)
        res = F("GET");
    else if (request->method() == HTTP_POST)
        res = F("POST");
    else if (request->method() == HTTP_DELETE)
        res = F("DELETE");
    else if (request->method() == HTTP_PUT)
        res = F("PUT");
    else if (request->method() == HTTP_PATCH)
        res = F("PATCH");
    else if (request->method() == HTTP_HEAD)
        res = F("HEAD");
    else if (request->method() == HTTP_OPTIONS)
        res = F("OPTIONS");
    return res;
}

const String getRequestInfo(AsyncWebServerRequest* request) {
    String res = getMethodName(request);
    res += ' ';
    res += "http://";
    res += request->host();
    res += request->url();
    res += '\n';
    if (request->contentLength()) {
        res += "content-type: ";
        res += request->contentType();
        res += " content-lenght: ";
        res += prettyBytes(request->contentLength());
        res += '\n';
    }

    if (request->headers()) {
        res += "headers:\n";
        for (size_t i = 0; i < request->headers(); i++) {
            AsyncWebHeader* h = request->getHeader(i);
            res += h->name();
            res += '=';
            res += h->value();
            res += '\n';
        }
    }

    if (request->params()) {
        res += "params:\n";
        for (size_t i = 0; i < request->params(); i++) {
            AsyncWebParameter* p = request->getParam(i);
            if (p->isFile()) {
                res += "FILE";
            }
            else if (p->isPost()) {
                res += "POST";
            }
            else {
                res += "GET";
            }
            res += ' ';
            res += p->name();
            res += ':';
            res += p->value();
            if (p->isFile()) {
                res += " size:";
                res += p->size();
            }
            res += '\n';
        }
    }
    return res;
}

void wifiSignalInit() {
    ts.add(
        SYGNAL, 1000 * 60, [&](void*) {
            SerialPrint("I", "System", printMemoryStatus());
            switch (RSSIquality()) {
            case 0:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='red'>не подключено к роутеру</font>"));
                break;
            case 1:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='red'>нет сигнала</font>"));
                break;
            case 2:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='red'>очень низкий</font>"));
                break;
            case 3:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='orange'>низкий</font>"));
                break;
            case 4:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='green'>хороший</font>"));
                break;
            case 5:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='green'>очень хороший</font>"));
                break;
            case 6:
                jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='green'>отличный</font>"));
                break;
            }
        },
        nullptr, true);
}
