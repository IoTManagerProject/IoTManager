# PrepareProject.py - инструмент для подготовки проекта к компиляции.
# Необходимо вызвать при изменении персональных настроек или состава модулей.
# 
# Скрипт выполняется из корня репозитория IoTManager (cwd = PROJECT_ROOT),
# поэтому относительные пути src/modules, data_full, data_lite и пути модулей
# из профиля разрешаются относительно корня.
#
# Папка data_svelte используется из каталога, в котором находится myProfile.json
# (profileDir) — то есть у каждого проекта своя data_svelte.
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
# Используя параметры -b или --board <board_name> можно уточнить для какой платы нужно подготовить проект
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
        -h --help
        -b --board <board_name>''')
    with open('myProfile.json', "r", encoding='utf-8') as read_file:
        profJson = json.load(read_file)  
    print ('')
    print ('Choose a board from the list:')
    # print(profJson['projectProp']['platformio']['comments_default_envs'])
    print ('        ', end='')
    cnt = 0
    for envs in profJson['projectProp']['platformio']['envs']:
        if cnt == 5:
            cnt = 0
            print('')
            print('        ', end='')
        print(envs['name'] + ', ', end='')
        cnt = cnt + 1


def updateModulesInProfile(profJson):
    profJson["modules"] = {}
    for root,d_names,f_names in os.walk("src/modules"):
        for fname in f_names:
            if fname == "modinfo.json":
                with open(os.path.join(root, fname), "r", encoding='utf-8') as read_file:
                    modinfoJson = json.load(read_file)
                    # добавляем информацию о модуле в узловой элемент
                    if modinfoJson['menuSection'] not in profJson["modules"]:
                        profJson["modules"][modinfoJson['menuSection']] = []
                    profJson["modules"][modinfoJson['menuSection']].append({
                        'path': os.path.normpath(root).replace("\\", "/"),
                        'active': modinfoJson['defActive']
                    })


def copy_missing(src_dir, dst_dir):
    """Копирует файлы из src_dir в dst_dir, НЕ перезаписывая существующие.

    Нужно, чтобы папка data_svelte проекта была полной (settings.json и пр.),
    но при этом сохранялись уже имеющиеся данные проекта.
    """
    if not os.path.isdir(src_dir):
        return
    os.makedirs(dst_dir, exist_ok=True)
    for root, dirs, files in os.walk(src_dir):
        rel = os.path.relpath(root, src_dir)
        target = dst_dir if rel == "." else os.path.join(dst_dir, rel)
        os.makedirs(target, exist_ok=True)
        for fname in files:
            sp = os.path.join(root, fname)
            dp = os.path.join(target, fname)
            if not os.path.exists(dp):
                shutil.copy2(sp, dp)





def _detect_indent(path, default=4):
    """Определяет ширину отступа в существующем JSON-файле."""
    try:
        with open(path, "r", encoding='utf-8') as f:
            for line in f:
                if line.startswith(' ') and not line.strip().startswith('{'):
                    return len(line) - len(line.lstrip())
    except Exception:  # noqa: BLE001
        pass
    return default


def _write_profile_json(path, data):
    """Записывает профиль, сохраняя исходный отступ и порядок полей."""
    indent = _detect_indent(path, 4)
    with open(path, "w", encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=indent, sort_keys=False)


update = False              # признак необходимости обновить список модулей
profile = 'myProfile.json'  # имя профиля. Будет заменено из консоли, если указано при старте
selectDevice = ''           # имя платы для которой хотим собрать, если её указали к командной строке -b <board>

argv = sys.argv[1:]
try:
    opts, args = getopt.getopt(argv, 'hp:ub:', ['help', 'profile=', 'update', 'board='])
except getopt.GetoptError:
    print('Ошибка обработки параметров!')
    printHelp()
    sys.exit(2)

for opt, arg in opts:
    if opt in ("-h", "--help"):
        printHelp()
        sys.exit()
    elif opt in ("-p", "--profile"):
        print('Загрузка профиля из файла:' + arg)
        profile = arg
    elif opt in ("-u", "--update"):
        print('Создание новой конфигурации по исходным файлам!')
        update = True
    elif opt in ("-b", "--board"):
        print('Создание профиля для платы:' + arg)
        selectDevice = arg

# определяем каталог, в котором находится файл профиля,
# чтобы читать и изменять platformio.ini в той же папке что и myProfile.json
profileDir = str(Path(profile).parent)
# Папка данных прошивки — data_svelte. Итоговое расположение — корень проекта (<проект>/data_svelte),
# рядом с myProfile.json. deviceName ниже не влияет на расположение data_svelte.
DATA_DIR = os.path.join(profileDir, "data_svelte")

# data_svelte проекта заполняется ниже, в зависимости от выбранного устройства.

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
        
        _write_profile_json(profile, profJson)
else:
    # если файла нет - создаем по образу настроек из проекта
    profJson = json.loads('{}')
    # копируем параметры IOTM из settings.json в новый профиль
    with open(os.path.join(DATA_DIR, "settings.json"), "r", encoding='utf-8') as read_file:
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
    _write_profile_json(profile, profJson)

deviceName = ''
if selectDevice == '':
    # определяем какое устройство используется в профиле
    deviceName = profJson['projectProp']['platformio']['default_envs']  
else:
    for envs in profJson['projectProp']['platformio']['envs']:
        if envs['name'] == selectDevice:
            deviceName = selectDevice
    if deviceName == '':
        deviceName = profJson['projectProp']['platformio']['default_envs']
        print(f"\x1b[1;31;31m Board ", selectDevice, " not found in ",profile,"!!! Use ",deviceName,"  \x1b[0m")

# Папка данных прошивки всегда лежит в корне проекта (рядом с myProfile.json),
# в том числе и для корневого PlatformIO-проекта (profileDir == корень репозитория).
DATA_DIR = os.path.join(profileDir, "data_svelte")

# заполняем папку /data файлами прошивки в зависимости от устройства
is_ota_lite = deviceName in ('esp8266_1mb_ota', 'esp8285_1mb_ota', 'esp8266_2mb_ota')
data_source = "data_lite" if is_ota_lite else "data_full"

if is_ota_lite:
    # Для OTA-плат формируем чистую минимальную ФС: только data_lite + обязательные
    # конфиги. Не копируем из корневой data_svelte полный веб-набор (edit.htm.gz,
    # полные бандлы), чтобы не превысить маленький раздел littlefs (~64 КБ).
    if os.path.isdir(DATA_DIR):
        shutil.rmtree(DATA_DIR)
    shutil.copytree(data_source, DATA_DIR, symlinks=False, ignore=None, ignore_dangling_symlinks=False, dirs_exist_ok=True)
    config_seeds = [
        "config.json", "dev_conf.txt", "items.json", "layout.json",
        "ota.json", "scenario.txt", "settings.json", "values.json", "widgets.json",
    ]
    os.makedirs(DATA_DIR, exist_ok=True)
    for fname in config_seeds:
        src = os.path.join("data_svelte", fname)
        dst = os.path.join(DATA_DIR, fname)
        if os.path.isfile(src) and not os.path.exists(dst):
            shutil.copy2(src, dst)
else:
    # Для остальных плат дозаполняем data_svelte недостающими файлами из корневой
    # data_svelte (settings.json, items.json, flashProfile.json и пр.), сохраняя
    # существующие данные проекта, затем применяем полный набор прошивки.
    copy_missing("data_svelte", DATA_DIR)
    shutil.copytree(data_source, DATA_DIR, symlinks=False, ignore=None, ignore_dangling_symlinks=False, dirs_exist_ok=True)

deviceType = 'esp32*'
if not 'esp32' in deviceName:
    deviceType = 'esp82*'
if 'bk72' in deviceName:
    deviceType = 'bk72*'
# генерируем файлы проекта на основе подготовленного профиля
# заполняем конфигурационный файл прошивки параметрами из профиля
settings_path = os.path.join(DATA_DIR, "settings.json")
if not os.path.isfile(settings_path):
    # Если settings.json отсутствует (например, корневой data_svelte был очищен) —
    # восстанавливаем его из настроек профиля, чтобы сборка не падала.
    os.makedirs(DATA_DIR, exist_ok=True)
    with open(settings_path, "w", encoding='utf-8') as wf:
        json.dump(dict(profJson.get('iotmSettings', {})), wf, ensure_ascii=False, indent=4)
with open(settings_path, "r", encoding='utf-8') as read_file:
    iotmJson = json.load(read_file)
for key, value in profJson['iotmSettings'].items():
    iotmJson[key] = value
with open(os.path.join(DATA_DIR, "settings.json"), "w", encoding='utf-8') as write_file:
    json.dump(iotmJson, write_file, ensure_ascii=False, indent=4)


        
# собираем меню прошивки из модулей
# параллельно формируем список имен активных модулей
# параллельно собираем необходимые активным модулям библиотеки для включения в компиляцию для текущего типа устройства (esp8266_4m, esp32_4mb, esp8266_1m, esp8266_1m_ota) 
activeModulesName = []  # список имен активных модулей
allLibs = ""            # подборка всех библиотек необходимых модулям для дальнейшей записи в конфигурацию platformio
allDefs = "\n"            # для каждого модуля создаем глобальный define
itemsCount = 1
includeDirs = ""        # подборка путей ко всем модулям для дальнейшей записи в конфигурацию platformio
itemsJson = json.loads('[{"name": "Выберите элемент", "num": 0}]')
for section, modules in profJson['modules'].items():
    itemsJson.append({"header": section})
    for module in modules:
        if module['active']:
            modinfo_path = module['path'] + "/modinfo.json"
            if not os.path.isfile(modinfo_path):
                print(f"Пропуск отсутствующего модуля (нет {modinfo_path}): путь в профиле устарел")
                continue
            with open(modinfo_path, "r", encoding='utf-8') as read_file:
                moduleJson = json.load(read_file)
                if 'moduleDefines' in moduleJson['about']:
                    allDefs = allDefs + "\n".join("-D" + d for d in moduleJson['about']['moduleDefines'])
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
                            configItemsJson['moduleName'] = moduleJson['about']['moduleName']
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
                            configItemsJson['moduleName'] = moduleJson['about']['moduleName']     

with open(os.path.join(DATA_DIR, "items.json"), "w", encoding='utf-8') as write_file:
    json.dump(itemsJson, write_file, ensure_ascii=False, indent=4)


# учитываем вызовы модулей в API.cpp
allAPI_head = ""
allAPI_exec = ""
for activModuleName in activeModulesName:
    allAPI_head = allAPI_head + "\nvoid* getAPI_" + activModuleName + "(String subtype, String params);"
    allAPI_exec = allAPI_exec + "\nif ((tmpAPI = getAPI_" + activModuleName + "(subtype, params)) != nullptr) foundAPI = tmpAPI;"
apicpp = '#include "ESPConfiguration.h"\n'
apicpp = apicpp + allAPI_head
apicpp = apicpp + '\n\nvoid* getAPI(String subtype, String params) {\nvoid* tmpAPI; void* foundAPI = nullptr;'
apicpp = apicpp + allAPI_exec
apicpp = apicpp + '\nreturn foundAPI;\n}'
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
ini_path = os.path.join(profileDir, "platformio.ini")
# Если у проекта нет platformio.ini — создаём его из корневого шаблона
if not os.path.isfile(ini_path):
    if os.path.isfile("platformio.ini"):
        shutil.copy("platformio.ini", ini_path)
        print(f"Создан platformio.ini проекта из корневого шаблона: {ini_path}")
    else:
        open(ini_path, "w", encoding="utf-8").close()
config.clear()
try:
    config.read(ini_path, encoding='utf-8')
except Exception:  # noqa: BLE001  старые ini могли быть записаны в кодировке cp1251
    config.clear()
    config.read(ini_path, encoding='cp1251')
# Защитно добавляем недостающие секции
fromitems_sec = "env:" + deviceName + "_fromitems"
if not config.has_section("platformio"):
    config.add_section("platformio")
if not config.has_section(fromitems_sec):
    config.add_section(fromitems_sec)
if not config.has_section("env:" + deviceName):
    config.add_section("env:" + deviceName)
    config.set("env:" + deviceName, "build_flags", "")
config[fromitems_sec]["lib_deps"] = allLibs
config[fromitems_sec]["build_src_filter"] = includeDirs
config[fromitems_sec]["build_flags"] = allDefs
config["platformio"]["default_envs"] = deviceName
if "${env:" + deviceName + "_fromitems.build_flags}" not in config["env:" + deviceName]["build_flags"]:
    config["env:" + deviceName]["build_flags"] += "\n${env:" + deviceName + "_fromitems.build_flags}"
# Папка данных прошивки — data_svelte в корне проекта (<проект>/data_svelte).
# Путь относительный (от корня репозитория — cwd, где запускается pio), чтобы в
# platformio.ini не было абсолютных путей и кириллических полных адресов диска.
config["platformio"]["data_dir"] = os.path.relpath(DATA_DIR).replace(os.sep, "/")
with open(ini_path, 'w', encoding='utf-8') as configFile:
    config.write(configFile)
    
    
# сохраняем часть применяемого профиля в папку data_svelte для загрузки на контроллер и дальнейшего переиспользования
print(f"Saving profile {profile} in {DATA_DIR}/flashProfile.json")
shortProfJson = json.loads('{}')
shortProfJson['projectProp'] = {
        'platformio': {
            'default_envs': deviceName
        }
    }
shortProfJson['modules'] = profJson['modules']
with open(os.path.join(DATA_DIR, "flashProfile.json"), "w", encoding='utf-8') as write_file:
    json.dump(shortProfJson, write_file, ensure_ascii=False, indent=4)
    
    
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