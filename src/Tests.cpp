#include "Tests.h"

#include "BufferExecute.h"
#include "Class/TCircularBuffer.h"
#include "Global.h"
#include "ItemsList.h"
#include "Macro.h"
#include "MySensorsDataParse.h"
#include "Utils/StringUtils.h"

void testsPerform() {
    Serial.println("====some tests section====");
    //ТЕСТ КОЛЬЦЕВОГО БУФЕРА=============================================================================
    // myTCircularBuffer = new TCircularBuffer<char*, 1024>;
    // char* buf = "text";
    // for (int i = 1; i <= 5; i++) {
    //    myTCircularBuffer->push(buf);
    //}
    // char* item;
    // while (myTCircularBuffer->pop(item)) {
    //    Serial.println(item);
    //}
    //===================================================================================================
    // String str = "0;1;2;3;4";
    // char* mychar = new char[str.length() + 1];
    // strcpy(mychar, str.c_str());
    // test(mychar);
    //===========================================================================
    // String myJson;
    // const int capacity = JSON_ARRAY_SIZE(2) + 3 * JSON_OBJECT_SIZE(3);
    // StaticJsonBuffer<capacity> jb;
    // JsonArray& arr = jb.createArray();
    // JsonObject& obj1 = jb.createObject();
    // obj1["test1"] = 1;
    // obj1["test2"] = 2;
    // obj1["test3"] = 3;
    // arr.add(obj1);
    // arr.printTo(myJson);
    // Serial.println(myJson);
    //
    //
    //
    //===========================================================================
    // Serial.println(isDigitDotCommaStr("-12552.5555"));
    // String str = "Geeks for Geeks ";
    // Serial.println(itemsCount2(str, " "));
    //
    Serial.println("==========end============");
}

void testLoop() {
    // char* item;
    // myCircularBuffer->pop(item);
    // Serial.println(item);
}