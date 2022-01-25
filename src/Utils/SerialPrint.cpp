#include "Utils/SerialPrint.h"
#include "Global.h"
void SerialPrint(String errorLevel, String module, String msg) {
    //if (module == "MySensor") {
        Serial.println(prettyMillis(millis()) + " [" + errorLevel + "] [" + module + "] " + msg);
         String logtoWS = jsonReadStr(configSetupJson, "logtoWS");
         if (logtoWS == "true"){
             #ifdef WEBSOCKET_ENABLED
               ws.textAll(" [" + errorLevel + "] [" + module + "] " + msg);
            #endif 
       
         }
        String logtoMQTT = jsonReadStr(configSetupJson, "logtoMQTT");
         if (logtoMQTT == "true"){
              publishAnyJsonKey("log", "log", errorLevel + " " + module + " " + msg); 
         }
    //}
}