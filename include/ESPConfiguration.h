#pragma once
#include "Global.h"
#include "Classes/IoTItem.h"

extern std::vector<IoTItem*> IoTItems;  // вектор ссылок базового класса IoTItem - список всех запущенных сенсоров

extern void configure(String path);
extern IoTItem* myIoTItem;
