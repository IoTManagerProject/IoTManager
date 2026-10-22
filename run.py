# Скрипт для простой прошивки ESP с учетом профиля из myProfile.json и поиска нужной кнопочки в интерфейсе PlatformIO
# Если ничего не указано в параметрах, то выполняется последовательно набор команд:
# 1. run PrepareProject.py
# 2. platformio run -t clean
# 3. platformio run -t uploadfs -е default_envs
# 4. platformio run -t upload -е default_envs
# 5. platformio run -t monitor
# где default_envs - это параметр default_envs из myProfile.json
# 
# Если указан параметр -p или --profile <ИмяФайла>, то выполняется первая команда PrepareProject.py -p <ИмяФайла>
# Если указан параметр -b или --board <board_name>, то выполняется первая команда PrepareProject.py -b <board_name>
# Если указан парамтер -l или --lite, то пропускаются команды 1, 2 и 5 с предварительной компиляцией
# Если указан параметр -d или --debug, то выполняется только команда 4 с предварительной компиляцией

import os
import shutil
import subprocess
import sys
import json

def get_platformio_path():
    """
    Возвращает путь к исполняемому файлу PlatformIO (pio/platformio).

    Сначала ищем команду в PATH, затем резервно — в стандартном каталоге установки.
    """
    # 1. Поиск в PATH (pio или platformio)
    for name in ('pio', 'platformio'):
        found = shutil.which(name)
        if found:
            return found
    # 2. Резерв: стандартный каталог установки PlatformIO
    if os.name == 'nt':  # Windows
        exe_names = ['platformio.exe', 'pio.exe']
        base = os.path.join(os.environ['USERPROFILE'], '.platformio', 'penv', 'Scripts')
    else:  # Linux/MacOS
        exe_names = ['pio', 'platformio']
        base = os.path.join(os.environ['HOME'], '.platformio', 'penv', 'bin')
    for name in exe_names:
        candidate = os.path.join(base, name)
        if os.path.isfile(candidate):
            return candidate
    # Если ни один не найден — возвращаем дефолт, чтобы вызвать понятную ошибку запуска
    return os.path.join(base, exe_names[0])

def load_default_envs(profile_path="myProfile.json"):
    """
    Загружает значение default_envs из файла myProfile.json.
    """
    if not os.path.isfile(profile_path):
        print(f"Файл профиля {profile_path} не найден.")
        sys.exit(1)

    try:
        with open(profile_path, 'r', encoding='utf-8') as file:
            profile_data = json.load(file)
            return profile_data["projectProp"]["platformio"]["default_envs"]
    except KeyError:
        print("Не удалось найти ключ 'default_envs' в myProfile.json.")
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"Ошибка при чтении файла {profile_path}: некорректный JSON.")
        sys.exit(1)

def run_command(command):
    """
    Выполняет указанную команду в subprocess.
    """
    try:
        print(f"Выполнение команды: {' '.join(command)}")
        subprocess.run(command, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Ошибка при выполнении команды: {e}")
        sys.exit(e.returncode)

def run_platformio():
    """
    Основная логика выполнения команд в зависимости от параметров.
    """
    pio_path = get_platformio_path()

    # Проверяем, существует ли PlatformIO
    if not os.path.isfile(pio_path):
        print(f"PlatformIO не найден по пути: {pio_path}")
        sys.exit(1)
    # print(f"PlatformIO найден по пути: {pio_path}")
    
   # Читаем аргументы командной строки
    args = sys.argv[1:]
    lite_mode = '-l' in args or '--lite' in args
    debug_mode = '-d' in args or '--debug' in args
    profile_index = next((i for i, arg in enumerate(args) if arg in ('-p', '--profile')), None)
    profile_file = args[profile_index + 1] if profile_index is not None and len(args) > profile_index + 1 else "myProfile.json"
    
    # Загружаем default_envs из myProfile.json, если не указан параметр -b, который имеет больший приоритет
    board_index = next((i for i, arg in enumerate(args) if arg in ('-b', '--board')), None)
    default_envs = args[board_index + 1] if board_index is not None and len(args) > board_index + 1 else load_default_envs(profile_path=profile_file)

    print(f"Используем default_envs: {default_envs}")
    print(f"Режим Lite: {lite_mode}, Режим отладки: {debug_mode}")
    print(f"Профиль: {profile_file}")

    # Выполнение команд в зависимости от параметров
    if not lite_mode and not debug_mode:
        # Полный набор команд
        run_command(['python', 'PrepareProject.py', '-p', profile_file])
        
        # Добавляем сообщение о необходимости дождаться завершения обновления конфигурации
        input(f"\x1b[1;31;42m Подождите завершения обновления конфигурации PlatformIO, затем нажмите Ввод для продолжения...\x1b[0m")

        run_command([pio_path, 'run', '-t', 'clean'])
        run_command([pio_path, 'run', '-t', 'uploadfs', '--environment', default_envs])
        run_command([pio_path, 'run', '-t', 'upload', '--environment', default_envs])
        run_command([pio_path, 'run', '-t', 'monitor'])
    elif lite_mode:
        # Упрощенный режим (пропускаем команды 1, 2 и 5)
        run_command([pio_path, 'run', '-t', 'buildfs', '--environment', default_envs])
        run_command([pio_path, 'run', '-t', 'uploadfs', '--environment', default_envs])
        run_command([pio_path, 'run', '--environment', default_envs])
        run_command([pio_path, 'run', '-t', 'upload', '--environment', default_envs])
    elif debug_mode:
        # Режим отладки (только команда 4)
        run_command([pio_path, 'run', '--environment', default_envs])
        run_command([pio_path, 'run', '-t', 'upload', '--environment', default_envs])

if __name__ == "__main__":
    run_platformio()

