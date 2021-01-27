#include "Tests.h"

#include "Global.h"
#include "ItemsList.h"
#include "Macro.h"
#include "Utils/StringUtils.h"
#include "BufferExecute.h"

void testsPerform() {
    Serial.println("====some tests section====");

    String str = "0;1;2;3;4";
    char* mychar = new char[str.length() + 1];
    strcpy(mychar, str.c_str());
    test(mychar);

    //Serial.println(isDigitDotCommaStr("-12552.5555"));

    //String str = "Geeks for Geeks ";

    //Serial.println(itemsCount2(str, " "));

    Serial.println("==========end============");
}