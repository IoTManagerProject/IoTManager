# подготавливаем папку для локального сервера обновлений
# для этого  компилируем проект и получаем бинарные файлы:
# firmware.bin, littlefs.bin и ver.json

import shutil
import configparser
import os, json, sys, getopt
from pathlib import Path

config = configparser.ConfigParser()  # создаём объекта парсера INI
config.read("platformio.ini")
deviceName = config["platformio"]["default_envs"]

homeDir = os.path.expanduser('~')
os.system(homeDir + "\.platformio\penv\Scripts\pio run")
os.system(homeDir + "\.platformio\penv\Scripts\pio run -t buildfs")

firmwareSrcPath = ".pio/build/" + deviceName + "/firmware.bin"
littlefsSrcPath = ".pio/build/" + deviceName + "/littlefs.bin"

if Path(firmwareSrcPath).is_file():
    print("Ok...... \"" + firmwareSrcPath + "\" Exist!")
    firmwareDstPath = "iotm/" + deviceName + "/400/firmware.bin"
    Path(firmwareDstPath).parent.mkdir(parents=True, exist_ok=True)
    shutil.copy(firmwareSrcPath, firmwareDstPath)

    if Path(littlefsSrcPath).is_file():
        print("Ok...... \"" + littlefsSrcPath + "\" Exist!")
        littlefsDstPath = "iotm/" + deviceName + "/400/littlefs.bin"
        Path(littlefsDstPath).parent.mkdir(parents=True, exist_ok=True)
        shutil.copy(littlefsSrcPath, littlefsDstPath)
        
        versionsJson = json.loads('{"' + deviceName + '": {"0": "400"}}')
        with open("iotm/ver.json", "w", encoding='utf-8') as write_file:
            json.dump(versionsJson, write_file, ensure_ascii=False, indent=4, sort_keys=False)
            
        print(f"\x1b[1;31;42m Сервер для обновления 'по воздуху' для " + deviceName + " подготовлен. Не забудьте установить расширение Live Server, запустить его и прописать в ESP IP вашего компьютера. Подробнее смотрите в WIKI: https://iotmanager.org/wiki.\x1b[0m")
        
    else:
        print("Error...... \"" + littlefsSrcPath + "\" Not exist!")

else:
    print("Error...... \"" + firmwareSrcPath + "\" Not exist!")