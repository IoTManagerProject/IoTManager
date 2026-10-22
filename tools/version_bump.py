# Автоувеличение счётчика версии прошивки (include/Const.h, FIRMWARE_VERSION).
# Подключён через extra_scripts во всех окружениях platformio.ini и выполняется
# перед каждой сборкой прошивки.
#
# Формат версии: "<ручная часть>.<число коммитов git>", например "463.2225":
#   - первое число поднимается вручную при значимых изменениях (правится в Const.h,
#     скрипт его сохраняет);
#   - второе вычисляется как `git rev-list --count HEAD` и перезаписывается этим
#     скриптом автоматически — руками строку FIRMWARE_VERSION править не нужно.
# Вне git-репозитория (или при недоступном git) версия остаётся как есть — сборка
# не ломается. Понимает и старый целочисленный формат (#define FIRMWARE_VERSION 463)
# и сам мигрирует на строковый.
#
# Файл правится на уровне байтов (заменяется только сама строка define),
# поэтому кодировка и переводы строк Const.h не затрагиваются.

import re
import subprocess

CONST_H = "include/Const.h"

# старый формат: #define FIRMWARE_VERSION 463
# новый формат:  #define FIRMWARE_VERSION "463.2225"
VERSION_RE = re.compile(rb'#define\s+FIRMWARE_VERSION\s+"?(\d+)(?:\.(\d+))?"?')

# таргеты без компиляции прошивки — версию не трогаем (сборка ФС, замер размера)
SKIP_TARGETS = {"buildfs", "uploadfs", "uploadfsota", "size"}


def get_commit_count():
    # число коммитов HEAD; None — git недоступен (не репозиторий, нет git в PATH)
    try:
        out = subprocess.check_output(
            ["git", "rev-list", "--count", "HEAD"], stderr=subprocess.STDOUT)
        return int(out.strip())
    except Exception as e:
        print("FIRMWARE_VERSION: git недоступен (%s) — счётчик не обновлён" % e)
        return None


def update_firmware_version():
    if BUILD_TARGETS and set(BUILD_TARGETS) <= SKIP_TARGETS:
        return
    try:
        with open(CONST_H, "rb") as f:
            data = f.read()
    except OSError as e:
        print("FIRMWARE_VERSION: %s - %s" % (e.filename, e.strerror))
        return
    m = VERSION_RE.search(data)
    if not m:
        print("FIRMWARE_VERSION: #define не найден в %s" % CONST_H)
        return
    count = get_commit_count()
    if count is None:
        return
    manual = int(m.group(1))
    new_line = b'#define FIRMWARE_VERSION "%d.%d"' % (manual, count)
    if new_line == m.group(0):
        print("FIRMWARE_VERSION актуальна: %s" % new_line.decode())
        return
    data = data[:m.start()] + new_line + data[m.end():]
    with open(CONST_H, "wb") as f:
        f.write(data)
    print("FIRMWARE_VERSION обновлена: %s" % new_line.decode())


update_firmware_version()
