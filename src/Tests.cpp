#include "Tests.h"
#include "Macro.h"
#include "Global.h"
#include "ItemsList.h"

void testsPerform() {
    Serial.println("====some tests section====");

    #ifdef esp32_4mb
    Serial.println("esp32_4mb defined");
    #endif
    #ifdef esp8266_1mb
    Serial.println("esp32_4mb defined");
    #endif

    //Serial.println(STR(esp32_4mb));

    Serial.println("==========end============");
}