
import os
import configparser

def before_build(): # source, target, env
    print("Current Build targets", BUILD_TARGETS)
    # Это всё потому что не работает "buildprog". При сборке прошивки Targets пустой, на всякий случай исключим все остальные
    if (BUILD_TARGETS == ['upload'] or
        BUILD_TARGETS == ['buildfs'] or 
        BUILD_TARGETS == ['uploadfs'] or 
        BUILD_TARGETS == ['uploadfsota'] or 
        BUILD_TARGETS == ['size']):
            return

    print("Clear BUILD_TIME, delete main.o !")
    config = configparser.ConfigParser()  # создаём объекта парсера INI
    config.read("platformio.ini")
    deviceName = config["platformio"]["default_envs"]
    # удаляем объектный файл где используется время сборки, для обновления
    try:
        os.remove("./.pio/build/"+deviceName+"/src/Main.cpp.o")
    except OSError as e:
        # If it fails, inform the user.
        print("Error: %s - %s." % (e.filename, e.strerror))

#env.AddPreAction("buildprog", before_build) # $BUILD_DIR/src/main.cpp.o

before_build()