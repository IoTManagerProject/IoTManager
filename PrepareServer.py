# подготавливаем папку для локального сервера обновлений
# для этого  компилируем проект и получаем бинарные файлы:
# firmware.bin, littlefs.bin, partitions.bin и ver.json

import shutil
import configparser
import os, json, sys, getopt
from pathlib import Path


def copyFileIfExist(fileName, deviceName):
    srcFilePath = ".pio/build/" + deviceName + "/" + fileName
    dstFilePath = "iotm/" + deviceName + "/400/" + fileName
    if Path(srcFilePath).is_file():
        print("Ok...... \"" + srcFilePath + "\" на месте!")
        Path(dstFilePath).parent.mkdir(parents=True, exist_ok=True)
        shutil.copy(srcFilePath, dstFilePath)   
        return True
    else:
        print("Error...... \"" + srcFilePath + "\" НЕ существует!")
        return False
        
        
config = configparser.ConfigParser()  # создаём объекта парсера INI
config.read("platformio.ini")
deviceName = config["platformio"]["default_envs"]

homeDir = os.path.expanduser('~')
os.system(homeDir + "\.platformio\penv\Scripts\pio run")
os.system(homeDir + "\.platformio\penv\Scripts\pio run -t buildfs --disable-auto-clean")

if copyFileIfExist("firmware.bin", deviceName) and copyFileIfExist("littlefs.bin", deviceName):
    copyFileIfExist("partitions.bin", deviceName)
    versionsJson = json.loads('{"' + deviceName + '": {"0": "400"}}')
    with open("iotm/ver.json", "w", encoding='utf-8') as write_file:
        json.dump(versionsJson, write_file, ensure_ascii=False, indent=4, sort_keys=False)    
    print(f"\x1b[1;31;42m Сервер для обновления 'по воздуху' для " + deviceName + " подготовлен. Не забудьте установить расширение Live Server, запустить его и прописать в ESP IP вашего компьютера. Подробнее смотрите в WIKI: https://iotmanager.org/wiki.\x1b[0m")