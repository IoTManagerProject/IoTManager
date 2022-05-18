import configparser
import os
import json, pprint

config = configparser.ConfigParser()  # создаём объекта парсера INI
allLibs_esp8266_4mb = ""
allLibs_esp32_4mb = ""

allMenuItems = json.loads('[]')
allMenuItemsCount = 1

allAPI_head = ""
allAPI_exec = ""

excludeDirs = ""


def getDirs(path):
    global excludeDirs
    for file in os.listdir(path):
        maybeDir = os.path.join(path, file)
        if os.path.isdir(maybeDir):
            config.clear()
            if config.read(maybeDir + "/platformio.ini"):
                if config.getboolean("iotm", "exclude", fallback=False):
                    print("Excluded: " + maybeDir)
                    maybeDir = maybeDir.replace("src/", "")
                    excludeDirs = excludeDirs + "\n-<" + maybeDir + ">"
                else:
                    yield file


def getPIOLibs(patch):
    global allLibs_esp8266_4mb, allLibs_esp32_4mb
    for dir in getDirs(patch):  
        config.clear()
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
            
            
def getAPI(patch):
    global allAPI_head, allAPI_exec
    for dir in getDirs(patch):  
        print(patch + dir)
        allAPI_head = allAPI_head + "\nvoid* getAPI_" + dir + "(String subtype, String params);"
        allAPI_exec = allAPI_exec + "\nif ((tmpAPI = getAPI_" + dir + "(subtype, params)) != nullptr) return tmpAPI;"
            
            
            
# читаем и запоминаем все либы мз каждого модуля
getPIOLibs("src/modules/system/") 
getPIOLibs("src/modules/exec/")
getPIOLibs("src/modules/sensors/")  
getPIOLibs("src/modules/lcd/")

# сохраняем собранные либы в настройках PIO
config.clear()
config.read("platformio.ini")
config["env:esp8266_4mb_fromitems"]["lib_deps"] = allLibs_esp8266_4mb
config["env:esp32_4mb_fromitems"]["lib_deps"] = allLibs_esp32_4mb
config["env:esp8266_4mb_fromitems"]["src_filter"] = excludeDirs
config["env:esp32_4mb_fromitems"]["src_filter"] = excludeDirs
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
        
        
# собираем списки API для интеграции вызовов модулей
getAPI("src/modules/system/")
getAPI("src/modules/exec/")
getAPI("src/modules/sensors/")  
getAPI("src/modules/lcd/")

# сохраняем все API в API.cpp
apicpp = '#include "ESPConfiguration.h"\n'
apicpp = apicpp + allAPI_head
apicpp = apicpp + '\n\nvoid* getAPI(String subtype, String params) {\nvoid* tmpAPI;'
apicpp = apicpp + allAPI_exec
apicpp = apicpp + 'return nullptr;\n}'
with open('src/modules/API.cpp', 'w') as f:
    f.write(apicpp)