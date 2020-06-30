#include "WebClient.h"

#include <ESP8266HTTPClient.h>

namespace WebClient {

const String get(const String& url) {
    String res = "";
    HTTPClient http;
    http.begin(url);
    if (http.GET() == HTTP_CODE_OK) {
        res = http.getString();
    } else {
        res = "error";
    }
    http.end();
    return res;
}

}  // namespace WebClient
