
# c:\Users\bmw\.platformio\penv\Scripts\pio run -t buildfs 
#c:\Users\bmw\.platformio\penv\.libretiny\Scripts\ltchiptool -v flash write lt_littlefs.bin --start 0x1db000 -f BK7231N



Import("env")

# Определите имя таргета
target_name = "buildfs"

# Директория с данными берётся из конфигурации проекта (platformio data_dir).
# Теперь data_svelte расположена в iotm/<платформа>/data_svelte.
data_dir = env.subst("$PROJECT_DATA_DIR")

# Получите путь к выходному файлу LittleFS из переменной среды env
output_file = env.subst("$PROJECT_DIR") + "/lt_littlefs.bin"

# Получите путь к mklittlefs.exe из переменной среды env
mklittlefs_path = env.subst("$PROJECT_DIR") + "/tools/mklittlefs.exe"

# Определите команду, которая будет выполнена при вызове таргета
command = "{} -c {} -s 0x25000 -p 0x100 -b 0x1000 {}".format(mklittlefs_path, data_dir, output_file)

# Добавьте кастомный таргет
env.AddCustomTarget(
    name=target_name,
    dependencies=None,
    actions=[command],
    title="Build LittleFS",
    description="Build LittleFS file system"
)


# Определите путь к исполняемому файлу ltchiptool
ltchiptool_path = "${LTCHIPTOOL}"


print("Custom target '{}' added!".format(target_name))