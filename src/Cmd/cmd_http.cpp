#include "Cmd.h"

#include "WebClient.h"

void cmd_http() {
    String host = sCmd.next();
    String param = sCmd.next();

    param.replace("_", "%20");

    String url = "http://" + host + "/cmd?command=" + param;

    WebClient::get(url);
}
