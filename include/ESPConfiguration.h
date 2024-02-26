#pragma once
#include "Global.h"
#include "classes/IoTItem.h"

extern std::list<IoTItem*> IoTItems;  // вектор ссылок базового класса IoTItem - список всех запущенных сенсоров

extern void configure(String path);
void clearConfigure();
extern IoTItem* myIoTItem;
extern IoTBench* myIoTBernch;
