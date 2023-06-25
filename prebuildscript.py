
import os
import configparser

config = configparser.ConfigParser()  # создаём объекта парсера INI
config.read("platformio.ini")
deviceName = config["platformio"]["default_envs"]
# удаляем объектный файл где используется время сборки, для обновления
try:
    os.remove("./.pio/build/"+deviceName+"/src/Main.cpp.o")
except OSError as e:
    # If it fails, inform the user.
    print("Error: %s - %s." % (e.filename, e.strerror))