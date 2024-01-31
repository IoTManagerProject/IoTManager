# PrepareProject.py - инструмент для подготовки проекта к компиляции.
# Необходимо вызвать при изменении персональных настроек или состава модулей.
# 
# При отсутствии файла с персональными настройками, myProfile.json будет создан автоматически
# python PrepareProject.py
# 
# Если myProfile.json уже существует, можно запустить PrepareProject.py с параметром -u или --update для обновления списка модулей. 
# Данная функция будет полезна для разработчиков при добавлении модуля в папку src/modules
# python PrepareProject.py --update 
# python PrepareProject.py -u 
# 
# Возможно использовать несколько вариантов персональных настроек и уточнять имя файла при запуске с использованием параметра -p или -profile
# python PrepareProject.py --profile <ИмяФайла>
# python PrepareProject.py -p <ИмяФайла>
# 
# поддерживаемые контроллеры (профили):
# esp8266_4mb
# esp8266_16mb
# esp32_4mb
# esp32cam_4mb
# esp32_16mb
# esp32s2_4mb
# esp8266_1mb
# esp8266_1mb_ota
# esp8285_1mb
# esp8285_1mb_ota
# esp8266_2mb
# esp8266_2mb_ota


import configparser
import os, json, sys, getopt
from pathlib import Path
import shutil


config = configparser.ConfigParser()  # создаём объекта парсера INI

def printHelp():
    print('''Usage:
        PrepareProject.py
        -p --profile <file.json_in_root_folder>
        -u --update
        -h --help''')


def updateModulesInProfile(profJson):
    profJson["modules"] = {}
    for root,d_names,f_names in os.walk("src/modules"):
        for fname in f_names:
            if fname == "modinfo.json":
                with open(os.path.join(root, fname), "r", encoding='utf-8') as read_file:
                    modinfoJson = json.load(read_file)
                    # проверяем есть ли уже узловой элемент и если нет, то создаем
                    if not modinfoJson['menuSection'] in profJson["modules"]:
                        listFromFirstElement = {modinfoJson['menuSection']: []}
                        listFromFirstElement.update(profJson["modules"])
                        profJson["modules"] = listFromFirstElement
                    # добавляем информацию о модуле в узловой элемент
                    profJson["modules"][modinfoJson['menuSection']].append({
                        'path': os.path.normpath(root).replace("\\", "/"),
                        'active': modinfoJson['defActive']
                    })





update = False              # признак необходимости обновить список модулей
profile = 'myProfile.json'  # имя профиля. Будет заменено из консоли, если указано при старте
            
argv = sys.argv[1:]
try:
    opts, args = getopt.getopt(argv, 'hp:u', ['help', 'profile=', 'update'])
except getopt.GetoptError:
    print('Ошибка обработки параметров!')
    printHelp()
    sys.exit(2)

for opt, arg in opts:
    if opt in ("-h", "--help"):
        printHelp()
        sys.exit()
    elif opt in ("-p", "--profile"):
        profile = arg
    elif opt in ("-u", "--update"):
        update = True
        

if Path(profile).is_file():
    # подтягиваем уже существующий профиль
    with open(profile, "r", encoding='utf-8') as read_file:
        profJson = json.load(read_file)  
    # если хотим обновить список модулей в существующем профиле
    if update:
        updateModulesInProfile(profJson)
        
        # sortedListNames = sorted(profJson["modules"])
        # sortedModules = {}
        # for sortedModulName in sortedList:
            
        # print(profJson)
        
        with open(profile, "w", encoding='utf-8') as write_file:
            json.dump(profJson, write_file, ensure_ascii=False, indent=4, sort_keys=False)
else:
    # если файла нет - создаем по образу настроек из проекта
    profJson = json.loads('{}')
    # копируем параметры IOTM из settings.json в новый профиль
    with open("data_svelte/settings.json", "r", encoding='utf-8') as read_file:
        profJson['iotmSettings'] = json.load(read_file)
    # устанавливаем параметры сборки
    profJson['projectProp'] = {
        'platformio': {
            'default_envs': 'esp8266_4mb'
        }
    }
    # загружаем список модулей для сборки
    updateModulesInProfile(profJson)
    # сохраняем новый профиль
    with open(profile, "w", encoding='utf-8') as write_file:
        json.dump(profJson, write_file, ensure_ascii=False, indent=4, sort_keys=False)


# определяем какое устройство используется в профиле
deviceName = profJson['projectProp']['platformio']['default_envs']  

# заполняем папку /data файлами прошивки в зависимости от устройства
if deviceName == 'esp8266_1mb_ota' or deviceName == 'esp8285_1mb_ota' or deviceName == 'esp8266_2mb_ota': 
    shutil.copytree("data_lite", "data_svelte", symlinks=False, ignore=None, ignore_dangling_symlinks=False, dirs_exist_ok=True)
else:
    shutil.copytree("data_full", "data_svelte", symlinks=False, ignore=None, ignore_dangling_symlinks=False, dirs_exist_ok=True)

deviceType = 'esp32*'
if not 'esp32' in deviceName:
    deviceType = 'esp82*'

# генерируем файлы проекта на основе подготовленного профиля
# заполняем конфигурационный файл прошивки параметрами из профиля
with open("data_svelte/settings.json", "r", encoding='utf-8') as read_file:
    iotmJson = json.load(read_file)
for key, value in profJson['iotmSettings'].items():
    iotmJson[key] = value
with open("data_svelte/settings.json", "w", encoding='utf-8') as write_file:
    json.dump(iotmJson, write_file, ensure_ascii=False, indent=4, sort_keys=False)


        
# собираем меню прошивки из модулей
# параллельно формируем список имен активных модулей
# параллельно собираем необходимые активным модулям библиотеки для включения в компиляцию для текущего типа устройства (esp8266_4m, esp32_4mb, esp8266_1m, esp8266_1m_ota) 
activeModulesName = []  # список имен активных модулей
allLibs = ""            # подборка всех библиотек необходимых модулям для дальнейшей записи в конфигурацию platformio
itemsCount = 1;
includeDirs = ""        # подборка путей ко всем модулям для дальнейшей записи в конфигурацию platformio
itemsJson = json.loads('[{"name": "Выберите элемент", "num": 0}]')
for section, modules in profJson['modules'].items():
    itemsJson.append({"header": section})
    for module in modules:
        if module['active']:
            with open(module['path'] + "/modinfo.json", "r", encoding='utf-8') as read_file:
                moduleJson = json.load(read_file)
                if deviceName in moduleJson['usedLibs']:   # проверяем поддерживает ли модуль текущее устройство
                    if not 'exclude' in moduleJson['usedLibs'][deviceName]: # смотрим не нужно ли исключить данный модуль из указанной платы deviceName
                        activeModulesName.append(moduleJson['about']['moduleName'])     # запоминаем имена для использования на след шагах
                        includeDirs = includeDirs + "\n+<" + module['path'].replace("src/", "") + ">"  # запоминаем пути к модулям для компиляции
                        for libPath in moduleJson['usedLibs'][deviceName]:               # запоминаем библиотеки необходимые модулю для текущей платы
                            allLibs = allLibs + "\n" + libPath       
                        for configItemsJson in moduleJson['configItem']:
                            configItemsJson['num'] = itemsCount
                            configItemsJson['name'] = str(itemsCount) + ". " + configItemsJson['name']
                            itemsCount = itemsCount + 1
                            itemsJson.append(configItemsJson)    
                else: # В первую очередь ищем по имени deviceName, чтобы для данной платы можно было уточнить либы. Если не нашли плату по имени в usedLibs пробуем найти её по типу deviceType
                    if deviceType in moduleJson['usedLibs']:   # проверяем поддерживает ли модуль текущее устройство
                        activeModulesName.append(moduleJson['about']['moduleName'])     # запоминаем имена для использования на след шагах
                        includeDirs = includeDirs + "\n+<" + module['path'].replace("src/", "") + ">"  # запоминаем пути к модулям для компиляции
                        for libPath in moduleJson['usedLibs'][deviceType]:               # запоминаем библиотеки необходимые модулю для текущей платы
                            allLibs = allLibs + "\n" + libPath       
                        for configItemsJson in moduleJson['configItem']:
                            configItemsJson['num'] = itemsCount
                            configItemsJson['name'] = str(itemsCount) + ". " + configItemsJson['name']
                            itemsCount = itemsCount + 1
                            itemsJson.append(configItemsJson)    

with open("data_svelte/items.json", "w", encoding='utf-8') as write_file:
    json.dump(itemsJson, write_file, ensure_ascii=False, indent=4, sort_keys=False)


# учитываем вызовы модулей в API.cpp
allAPI_head = ""
allAPI_exec = ""
for activModuleName in activeModulesName:
    allAPI_head = allAPI_head + "\nvoid* getAPI_" + activModuleName + "(String subtype, String params);"
    allAPI_exec = allAPI_exec + "\nif ((tmpAPI = getAPI_" + activModuleName + "(subtype, params)) != nullptr) return tmpAPI;"
apicpp = '#include "ESPConfiguration.h"\n'
apicpp = apicpp + allAPI_head
apicpp = apicpp + '\n\nvoid* getAPI(String subtype, String params) {\nvoid* tmpAPI;'
apicpp = apicpp + allAPI_exec
apicpp = apicpp + '\nreturn nullptr;\n}'
with open('src/modules/API.cpp', 'w') as f:
    f.write(apicpp)

# корректируем параметры platformio
# собираем пути всех отключенных модулей для исключения их из процесса компиляции
# excludeDirs = ""
# for root,d_names,f_names in os.walk("src\\modules"):
#     for fname in f_names:
#         if fname == "modinfo.json":
#             with open(root + "\\" + fname, "r", encoding='utf-8') as read_file:
#                 modinfoJson = json.load(read_file)
#                 if not modinfoJson['about']['moduleName'] in activeModulesName:
#                     excludeDirs = excludeDirs + "\n-<" + root.replace("src\\", "") + ">"

# фиксируем изменения в platformio.ini
config.clear()
config.read("platformio.ini")
config["env:" + deviceName + "_fromitems"]["lib_deps"] = allLibs
config["env:" + deviceName + "_fromitems"]["build_src_filter"] = includeDirs
config["platformio"]["default_envs"] = deviceName
# config["platformio"]["data_dir"] = profJson['projectProp']['platformio']['data_dir']
with open("platformio.ini", 'w') as configFile:
    config.write(configFile)
    
    
# сохраняем часть применяемого профиля в папку data_svelte для загрузки на контроллер и дальнейшего переиспользования
print(f"Saving profile {profile} in /data_svelte/flashProfile.json")
shortProfJson = json.loads('{}')
shortProfJson['projectProp'] = {
        'platformio': {
            'default_envs': deviceName
        }
    }
shortProfJson['modules'] = profJson['modules']
with open("data_svelte/flashProfile.json", "w", encoding='utf-8') as write_file:
    json.dump(shortProfJson, write_file, ensure_ascii=False, indent=4, sort_keys=False)
    
    
# import ctypes  # An included library with Python install.   
# if update:    
#     ctypes.windll.user32.MessageBoxW(0, "Модули профиля " + profile + " обновлены, а сам профиль применен, можно запускать компиляцию и прошивку.", "Операция завершена.", 0)
# else:
#     ctypes.windll.user32.MessageBoxW(0, "Профиль " + profile + " применен, можно запускать компиляцию и прошивку.", "Операция завершена.", 0)

if update:    
    shutil.copy(profile, "compilerProfile.json") 
    print(f"\x1b[1;31;42m Profile modules " + profile + " updated, profile applied, you can run compilation and firmware.\x1b[0m")
    
else:
    print(f"\x1b[1;31;42m Profile ", profile, " applied, you can run compilation and firmware.\x1b[0m")

# print(f"\x1b[1;32;41m Операция завершена. \x1b[0m")

        