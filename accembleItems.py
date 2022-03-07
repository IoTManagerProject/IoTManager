import configparser
import os
import json, pprint

config = configparser.ConfigParser()  # создаём объекта парсера INI
allLibs_esp8266_4mb = ""
allLibs_esp32_4mb = ""
allMenuItems = json.loads('[]')
allMenuItemsCount = 1;


def getDirs(path):
    for file in os.listdir(path):
        if os.path.isdir(os.path.join(path, file)):
            yield file


def getPIOLibs(patch):
    global allLibs_esp8266_4mb, allLibs_esp32_4mb
    for dir in getDirs(patch):  
        if (config.read(patch + dir + "/platformio.ini")):
            print(patch + dir + "/platformio.ini")
            allLibs_esp8266_4mb = allLibs_esp8266_4mb + config["env:esp8266_4mb"]["lib_deps"]
            allLibs_esp32_4mb = allLibs_esp32_4mb + config["env:esp32_4mb"]["lib_deps"]

               
def getMenuItems(patch):
    global allMenuItems, allMenuItemsCount
    with open(patch + "items.json", "r") as read_file:
        allMenuItems = allMenuItems + json.load(read_file)
    
    for dir in getDirs(patch):  
        with open(patch + dir + "/items.json", "r") as read_file:
            print(patch + dir + "/items.json")
            data = json.load(read_file)
            for item in data:
                item["name"] = str(allMenuItemsCount) + ". " + item["name"]
                item["num"] = allMenuItemsCount
                allMenuItemsCount = allMenuItemsCount + 1
            allMenuItems = allMenuItems + data
            
            
            
# читаем и запоминаем все либы мз каждого модуля
getPIOLibs("src/modules/system/") 
getPIOLibs("src/modules/exec/")
getPIOLibs("src/modules/sensors/")  
getPIOLibs("src/modules/lcd/")
     

# сохраняем собранные либы в настройках PIO
config.read("platformio.ini")
config["env:esp8266_4mb_fromitems"]["lib_deps"] = allLibs_esp8266_4mb
config["env:esp32_4mb_fromitems"]["lib_deps"] = allLibs_esp32_4mb
with open("platformio.ini", 'w') as configfile:
    config.write(configfile)



# готовим первый элемент меню
with open("src/modules/items.json", "r") as read_file:
    allMenuItems = allMenuItems + json.load(read_file)
            
# читаем и запоминаем пункты меню модуелей
getMenuItems("src/modules/system/") 
getMenuItems("src/modules/exec/")
getMenuItems("src/modules/sensors/")  
getMenuItems("src/modules/lcd/")
   
# сохраняем пункты меню в общий файл
with open('data_svelte/items.json', 'w') as f:
        json.dump(allMenuItems, f, ensure_ascii=False, indent=4, sort_keys=False)