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
#

import configparser
import os, json, sys, getopt
from pathlib import Path


config = configparser.ConfigParser()  # создаём объекта парсера INI

def printHelp():
    print('''Usage:
        PrepareProject.py
        -p --profile <file.json_in_root_folder>
        -u --update
        -h --help''')


def updateModulesInProfile(profJson):
    profJson["modules"] = {}
    for root,d_names,f_names in os.walk("src\\modules"):
        for fname in f_names:
            if fname == "modinfo.json":
                with open(root + "\\" + fname, "r", encoding='utf-8') as read_file:
                    modinfoJson = json.load(read_file)
                    # проверяем есть ли уже узловой элемент и если нет, то создаем
                    if not modinfoJson['menuSection'] in profJson["modules"]:
                        profJson["modules"][modinfoJson['menuSection']] = []
                    # добавляем информацию о модуле в узловой элемент
                    profJson["modules"][modinfoJson['menuSection']].append({
                        'path': root,
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
            'default_envs': 'esp8266_4mb',
            'data_dir': 'data_svelte'
        }
    }
    # загружаем список модулей для сборки
    updateModulesInProfile(profJson)
    # сохраняем новый профиль
    with open(profile, "w", encoding='utf-8') as write_file:
        json.dump(profJson, write_file, ensure_ascii=False, indent=4, sort_keys=False)
    
    
# генерируем файлы проекта на основе подготовленного профиля
# заполняем конфигурационный файл прошивки параметрами из профиля
with open("data_svelte/settings.json", "r", encoding='utf-8') as read_file:
    iotmJson = json.load(read_file)
for key, value in profJson['iotmSettings'].items():
    iotmJson[key] = value
with open("data_svelte/settings.json", "w", encoding='utf-8') as write_file:
    json.dump(iotmJson, write_file, ensure_ascii=False, indent=4, sort_keys=False)

# определяем какое устройство используется в профиле
deviceName = profJson['projectProp']['platformio']['default_envs']
        
# собираем меню прошивки из модулей
# параллельно формируем список имен активных модулей
# параллельно собираем необходимые активным модулям библиотеки для включения в компиляцию для текущего типа устройства (esp8266_4m, esp32_4mb) 
activeModulesName = []  # список имен активных модулей
allLibs = ""            # подборка всех библиотек необходимых модулям для дальнейшей записи в конфигурацию platformio
itemsCount = 1;
includeDirs = ""        # подборка путей ко всем модулям для дальнейшей записи в конфигурацию platformio
itemsJson = json.loads('[{"name": "Выберите элемент", "num": 0}]')
for section, modules in profJson['modules'].items():
    itemsJson.append({"header": section})
    for module in modules:
        if module['active']:
            with open(module['path'] + "\\modinfo.json", "r", encoding='utf-8') as read_file:
                moduleJson = json.load(read_file)
                if deviceName in moduleJson['devices']:   # проверяем поддерживает ли модуль текущее устройство
                    activeModulesName.append(moduleJson['about']['moduleName'])     # запоминаем имена для использования на след шагах
                    includeDirs = includeDirs + "\n+<" + module['path'].replace("src\\", "") + ">"  # запоминаем пути к модулям для компиляции
                    for libPath in moduleJson['devices'][deviceName]:               # запоминаем библиотеки необходимые модулю для текущей платы
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
config["platformio"]["data_dir"] = profJson['projectProp']['platformio']['data_dir']
with open("platformio.ini", 'w') as configFile:
    config.write(configFile)


        