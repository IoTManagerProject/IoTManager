#pragma once

#include "ESPAsyncWebServer.h"

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
            } else if (p->isPost()) {
                res += "POST";
            } else {
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
