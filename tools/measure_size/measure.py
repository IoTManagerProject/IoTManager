#!/usr/bin/env python3
"""
Измерение размера Flash / RAM для каждого модуля проекта IoTManager.

===============================================================================
ОБЩАЯ ЛОГИКА РАБОТЫ:
===============================================================================

1. СБОР МОДУЛЕЙ:
   Рекурсивно обходится папка src/modules/, находятся все modinfo.json,
   собираются сведения о каждом модуле (путь, имя, поддерживаемые платформы).

2. МЕНЮ ВЫБОРА РЕЖИМА:
   После запуска скрипт показывает меню:
     1 — обработка ВСЕХ модулей
     2 — обработка только модулей из myProfile.json
     3 — обработка только модулей без информации о размере

3. БАЗОВАЯ СБОРКА (baseline — без модулей):
   Для каждой целевой платформы собирается прошивка БЕЗ КАКИХ-ЛИБО модулей.
   Парсится из вывода PlatformIO:
     baseline_flash  — размер занятой Flash без модулей
     baseline_ram    — размер занятой RAM без модулей
     total_flash     — общий Flash платформы (характеристика платформы)
     total_ram       — общая RAM платформы (характеристика платформы)
   Каждый замер ДОПИСЫВАЕТСЯ в историю платформы в tools/measure_size/platforms.json
   (поля FIRMWARE_VERSION из include/Const.h и datetime времени замера).

4. ИЗМЕРЕНИЕ МОДУЛЕЙ:
   Для каждого модуля и каждой поддерживаемой платформы:
   - Собирается прошивка с базовым набором модулей + текущий модуль.
   - Парсится flash_used и ram_used из вывода pio.
   - Вычисляется дельта: flash_used - baseline_flash, ram_used - baseline_ram.
   - Дельта записывается в about.usedFLASH и about.usedRAM модуля в modinfo.json.
   - При ошибке компиляции ставится "-" в usedFLASH/usedRAM модуля в modinfo.json.

5. ЛОГИРОВАНИЕ:
   Весь вывод скрипта сохраняется в measure_size/logs/[дата_время]/log.txt.

===============================================================================
АРХИТЕКТУРА:
===============================================================================

Профиль (myProfile.json / compilerProfile.json) содержит конфигурацию сборки,
включая список активных модулей для каждого раздела. Скрипт:
  1. Копирует профиль во временный файл.
  2. Меняет в нём список активных модулей.
  3. Запускает PrepareProject.py — генерирует временную конфигурацию проекта.
  4. Запускает PlatformIO для сборки и получения размеров.
  5. Записывает результаты в modinfo.json модулей и platforms.json платформ.

platformio.ini также временно сохраняется и восстанавливается.

===============================================================================
ИСПОЛЬЗОВАНИЕ:
===============================================================================

  # Из корня IoTManager:
  python measure_size/measure.py [--env esp32c6_4mb] [--dry-run]

  # Из папки measure_size:
  python measure.py [--env esp32c6_4mb] [--dry-run]

  # dry-run     — показать список модулей без сборки
  # --env       — указать платформу (можно повторять)
  # --no-color  — отключить цветовые коды ANSI
  # --limit N   — измерить только первые N модулей на платформу
  # --profile   — указать профиль (по умолчанию myProfile.json)
"""

import argparse
import configparser
import fnmatch
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from datetime import datetime
from pathlib import Path

# IoTManager project root (parent of this script's folder)
PROJECT_ROOT = Path(__file__).resolve().parent.parent.parent
os.chdir(PROJECT_ROOT)

# Путь к platforms.json - tools\measure_size
PLATFORMS_JSON = PROJECT_ROOT / "tools" / "measure_size" / "platforms.json"

# Log styling (ANSI; disabled if not TTY or --no-color)
USE_COLOR = sys.stdout.isatty()

# Исполняемый файл PlatformIO (pio). По умолчанию — из PATH; можно переопределить через --pio.
PIO_BIN = None

# Файл-флаг мягкого прерывания (--abort-file). При появлении файла замер останавливается
# между шагами, а состояние проекта восстанавливается в finally.
ABORT_FILE = None


class _AbortError(Exception):
    """Внутреннее исключение для кооперативного прерывания замера."""
    pass


def _check_abort():
    """Выбрасывает _AbortError, если пользователь запросил прерывание (создан abort-файл)."""
    if ABORT_FILE and os.path.exists(ABORT_FILE):
        raise _AbortError()

# ----------------------------------------------------------------------------
# ФУНКЦИИ ФОРМАТИРОВАНИЯ ВЫВОДА (ЛОГИРОВАНИЕ)
# ----------------------------------------------------------------------------
# Эти функции добавляют ANSI-коды цвета к строкам для удобного чтения в
# терминале. Если вывод перенаправлен в файл или используется --no-color,
# цвета отключаются.
# ----------------------------------------------------------------------------


def style(s, color=None, bold=False):
    if not USE_COLOR or not color:
        return s
    codes = []
    if bold:
        codes.append("1")
        # ANSI-код: 1 = полужирный
    if color == "green":
        codes.append("32")
        # ANSI-код: 32 = зелёный (успех)
    elif color == "red":
        codes.append("31")
        # ANSI-код: 31 = красный (ошибка)
    elif color == "yellow":
        codes.append("33")
        # ANSI-код: 33 = жёлтый (предупреждение / шаг)
    elif color == "cyan":
        codes.append("36")
        # ANSI-код: 36 = голубой (заголовок секции)
    elif color == "dim":
        codes.append("2")
        # ANSI-код: 2 = тусклый (дополнительная информация)
    return f"\033[{';'.join(codes)}m{s}\033[0m" if codes else s
    # Формируем ANSI-последовательность: \033[код1;код2...m текст \033[0m
    # Код 0 сбрасывает все стили.


def log_section(title):
    """
    Выводит заголовок секции с декоративной линией.
    Используется для визуального разделения этапов работы скрипта.
    """
    width = 60
    line = "─" * width
    print()
    print(style(f"  {title}", "cyan", bold=True))
    print(style(line, "dim"))


def log_ok(msg):
    """Вывод успешной операции (зелёная галочка)."""
    print(style("  ✓ ", "green") + msg)


def log_fail(msg):
    """Вывод ошибки или неудачи (красный крестик)."""
    print(style("  ✗ ", "red") + msg)


def log_step(msg):
    """Вывод текущего шага работы (жёлтая точка)."""
    print(style("  ● ", "yellow") + msg)


def log_info(msg):
    """Вывод дополнительной информации (серый / тусклый текст)."""
    print(style("    ", "dim") + msg)


# ----------------------------------------------------------------------------
# ЛОГИРОВАНИЕ В ФАЙЛ
# ----------------------------------------------------------------------------
# Класс TeeWriter дублирует весь вывод stdout в файл лога.
# Лог сохраняется в measure_size/logs/[дата_время]/log.txt
# ----------------------------------------------------------------------------

LOGS_DIR = Path(__file__).resolve().parent / "logs"
_log_file = None


class TeeWriter:
    """Дублирует вывод stdout в файл лога (эффект 'tee').
    ANSI-коды цветов вырезаются при записи в файл для читаемости."""

    # Регулярное выражение для удаления ANSI-кодов
    _ANSI_RE = re.compile(r"\033\[[0-9;]*m")

    def __init__(self, stream, log_file):
        self._stream = stream
        self._log_file = log_file

    def write(self, data):
        self._stream.write(data)
        if self._log_file:
            # Вырезаем ANSI-коды для читаемого лог-файла
            clean = self._ANSI_RE.sub("", data)
            self._log_file.write(clean)

    def flush(self):
        self._stream.flush()
        if self._log_file:
            self._log_file.flush()

    def isatty(self):
        return self._stream.isatty()

    def __getattr__(self, name):
        return getattr(self._stream, name)


def start_logging():
    """
    Создаёт папку logs/[дата_время] и начинает запись лога в log.txt.
    Перенаправляет stdout через TeeWriter для дублирования вывода.
    Возвращает путь к папке лога.
    """
    global _log_file

    timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    log_dir = LOGS_DIR / timestamp
    log_dir.mkdir(parents=True, exist_ok=True)

    _log_file = open(log_dir / "log.txt", "w", encoding="utf-8")
    sys.stdout = TeeWriter(sys.stdout, _log_file)

    return log_dir


def stop_logging():
    """Закрывает файл лога и восстанавливает stdout."""
    global _log_file
    if _log_file:
        _log_file.close()
        _log_file = None
    # Восстанавливаем оригинальный stdout
    if isinstance(sys.stdout, TeeWriter):
        sys.stdout = sys.stdout._stream


# ----------------------------------------------------------------------------
# БАЗОВЫЙ НАБОР МОДУЛЕЙ
# ----------------------------------------------------------------------------
# Базовая сборка теперь выполняется БЕЗ КАКИХ-ЛИБО модулей.
# Это даёт чистый размер ядра системы без вклада модулей.
# ----------------------------------------------------------------------------
BASELINE_MODULE_PATHS = []


# ----------------------------------------------------------------------------
# ПОДДЕРЖИВАЕМЫЕ ПЛАТФОРМЫ
# ----------------------------------------------------------------------------
# Список платформ (env) для измерения.
# По умолчанию используется esp32c6_4mb. Можно переопределить через --env.
# ----------------------------------------------------------------------------
# DEFAULT_ENVS = ["esp8266_4mb", "esp32_4mb", "esp32_4mb3f", "esp8266_16mb", "esp32cam_4mb", "esp32s2_4mb", "esp32s3_16mb", "esp32c3m_4mb", "esp8266_1mb", "esp8266_1mb_ota", "esp8266_2mb", "esp8266_2mb_ota", "esp8285_1mb", "esp8285_1mb_ota", "esp32c6_4mb", "esp32c6_8mb", "bk7231n"]
# DEFAULT_ENVS = ["esp32s3_16mb", "esp32c3m_4mb", "esp32c6_4mb", "esp32_wifirep"]
DEFAULT_ENVS = []

# ----------------------------------------------------------------------------
# ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ РАБОТЫ С JSON
# ----------------------------------------------------------------------------

def load_json(path):
    """
    Загружает и парсит JSON-файл.
    Возвращает десериализованный объект (dict, list и т.д.).
    """
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def save_json(path, data):
    """
    Сохраняет объект в JSON-файл.
    Форматирование: 4 пробела, сортировка ключей, поддержка Unicode.
    """
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=4, sort_keys=False)


# ----------------------------------------------------------------------------
# СБОР МОДУЛЕЙ
# ----------------------------------------------------------------------------

def collect_modules():
    """
    Рекурсивно обходит папку src/modules/ и собирает информацию
    о каждом модуле, содержащем modinfo.json.

    Возвращает список словарей, каждый из которых содержит:
      - path:        относительный путь модуля
      - moduleName:  имя модуля из modinfo.json
      - usedLibs:    список поддерживаемых платформ
      - modinfo_path: абсолютный путь к modinfo.json
      - modinfo:     полный словарь modinfo.json (для записи изменений)
    """
    modules = []
    for root, _, names in os.walk(PROJECT_ROOT / "src" / "modules"):
        if "modinfo.json" not in names:
            continue
        path = Path(root).relative_to(PROJECT_ROOT)
        mod_path = path.as_posix()
        info_path = PROJECT_ROOT / path / "modinfo.json"
        info = load_json(info_path)
        about = info.get("about", {})
        modules.append({
            "path": mod_path,
            "moduleName": about.get("moduleName", path.name),
            "usedLibs": about.get("usedLibs", info.get("usedLibs", {})),
            "modinfo_path": info_path,
            "modinfo": info,
        })
    return modules


def filter_modules_by_profile(modules, prof_template):
    """
    Оставляет только те модули, которые АКТИВНЫ в профиле сборки (myProfile.json),
    т.е. у которых "active": true.

    Профиль содержит структуру:
      "modules": {
        "virtual_elments": [ {"path": "src/modules/virtual/Benchmark", "active": true}, ... ],
        ...
      }

    Возвращает новый список модулей, которые помечены active=true в профиле.
    """
    # Собираем множество путей АКТИВНЫХ модулей из всех секций профиля
    profile_paths = set()
    for section, mods in prof_template.get("modules", {}).items():
        for m in mods:
            # Учитываем только модули с "active": true
            if m.get("active"):
                path = m.get("path")
                if path:
                    profile_paths.add(path)

    # Оставляем только активные модули из профиля
    return [m for m in modules if m["path"] in profile_paths]


def filter_modules_without_size(modules, envs):
    """
    Оставляет только те модули, у которых НЕТ числового размера
    для текущей версии модуля на ХОТЯ БЫ ОДНОЙ из платформ envs.

    Размер берётся из modinfo.json (sizeInfo.usedFLASH / usedRAM):
      - нет ключа платформы в usedFLASH → ещё не считали;
      - значение "-" → была ошибка сборки (размер отсутствует);
      - значение число → размер есть.
    Модуль, не разрешённый для платформы (exclude в usedLibs), не учитывается.

    Возвращает новый список модулей без информации о размере.
    """
    def has_size(mod):
        """True, если у модуля есть размер хотя бы для одной из платформ."""
        return any(module_has_size_for_env(mod, env) for env in envs)

    # Оставляем только модули без размера
    return [m for m in modules if not has_size(m)]


def count_modules_for_envs(modules, envs):
    """
    Считает количество модулей, которые будут измеряться для указанных платформ.

    Модуль считается измеряемым, если он поддерживает ХОТЯ БЫ ОДНУ из платформ
    (module_supports_env). Такой модуль попадёт в модульную сборку.
    Модули, не поддерживающие ни одну платформу, в сборки не попадут.

    Возвращает количество модулей, участвующих в измерении.
    """
    count = 0
    for m in modules:
        for env in envs:
            if module_supports_env(m["usedLibs"], env):
                count += 1
                break  # модуль уже учтён для хотя бы одной платформы
    return count


def _module_matches(mod, target):
    """True, если модуль соответствует целевому пути.

    Сравниваем по полному относительному пути (mod["path"]) или по имени папки модуля.
    """
    p = mod["path"].rstrip("/")
    if p == target:
        return True
    return p.rsplit("/", 1)[-1] == target.rsplit("/", 1)[-1]


def show_menu(total_modules, profile_modules_count, modules_without_size_count):
    """
    Показывает меню выбора режима обработки и ждёт ввода пользователя.

    Возвращает:
      1 — обработать все модули
      2 — только модули из myProfile.json
      3 — только модули без информации о размере
      0 — назад (None)
    """
    log_section("Выбор режима обработки")
    print()
    print(style("  Выберите режим обработки модулей:", "cyan", bold=True))
    print(style("    1", "green", bold=True) + f" — обработка ВСЕХ модулей ({total_modules})")
    print(style("    2", "green", bold=True) + f" — обработка только модулей из myProfile.json ({profile_modules_count})")
    print(style("    3", "green", bold=True) + f" — обработка только модулей без информации о размере ({modules_without_size_count})")
    print(style("    0", "red", bold=True) + " — назад")
    print()

    while True:
        try:
            choice = input(style("  Ваш выбор (0/1/2/3): ", "yellow")).strip()
            if choice == "0":
                log_info("Назад.")
                return None
            if choice in ("1", "2", "3"):
                return int(choice)
            log_fail("Некорректный ввод. Введите 0, 1, 2 или 3.")
        except (EOFError, KeyboardInterrupt):
            print()
            log_fail("Ввод прерван. Выход.")
            sys.exit(1)


# ----------------------------------------------------------------------------
# МЕНЮ ВЫБОРА ПЛАТФОРМЫ
# ----------------------------------------------------------------------------

def get_envs_from_platformio_ini():
    """
    Извлекает список платформ (env) из platformio.ini.

    Платформы задаются секциями вида "[env:имя]" (например, "[env:esp32_4mb]").
    Секции-шаблоны с суффиксом "_fromitems" (например, "[env:esp32_4mb_fromitems]")
    пропускаются — они содержат только расширения настроек для модулей
    и платформами не являются.

    Возвращает список имён платформ (list[str]) в порядке их появления в файле.
    """
    ini_path = PROJECT_ROOT / "platformio.ini"
    if not ini_path.is_file():
        log_fail(f"Не найден файл platformio.ini: {ini_path}")
        sys.exit(1)

    envs = []
    with open(ini_path, "r", encoding="utf-8") as f:
        for line in f:
            m = re.match(r"^\s*\[env:([^\]]+)\]\s*$", line)
            if m:
                name = m.group(1).strip()
                if not name.endswith("_fromitems"):
                    envs.append(name)
    return envs


def show_top_mode_menu():
    """
    Показывает глобальное меню выбора режима измерения:

      1 — измерение ВСЕХ модулей для ОДНОЙ платформы
      2 — измерение ОДНОГО модуля для ВСЕХ платформ
      3 — измерение ОДНОГО модуля для ОДНОЙ платформы
      4 — измерение ВСЕХ модулей для ВСЕХ платформ
      0 — выход из программы

    Возвращает: выбранный режим (int: 1, 2, 3 или 4).
    """
    log_section("Выбор режима измерения")
    print()
    print(style("  Выберите режим измерения:", "cyan", bold=True))
    print()
    print(style("    1", "green", bold=True) + " — измерение всех модулей для одной платформы")
    print(style("    2", "green", bold=True) + " — измерение одного модуля для всех платформ")
    print(style("    3", "green", bold=True) + " — измерение одного модуля для одной платформы")
    print(style("    4", "green", bold=True) + " — измерение всех модулей для всех платформ")
    print(style("    0", "red", bold=True) + " — выход")
    print()

    while True:
        try:
            choice = input(style("  Ваш выбор (0/1/2/3/4): ", "yellow")).strip()
            if choice == "0":
                log_fail("Выход из программы.")
                sys.exit(0)
            if choice in ("1", "2", "3", "4"):
                log_ok(f"Выбран режим {choice}")
                return int(choice)
            log_fail("Некорректный ввод. Введите 0, 1, 2, 3 или 4.")
        except (EOFError, KeyboardInterrupt):
            print()
            log_fail("Ввод прерван. Выход.")
            sys.exit(1)


def show_platform_menu(modules):
    """
    Показывает список платформ из platformio.ini
    и ждёт выбора пользователя по номеру.

    Для каждой платформы выводится:
      номер - имя_платформы обработано X из Y
    где X — количество модулей, поддерживающих эту платформу,
        Y — общее количество модулей.

    Параметры:
      modules: список всех модулей (от collect_modules)

    Возвращает: список из одной выбранной платформы (list[str]).
    """
    # Извлекаем список платформ из platformio.ini
    platform_names = get_envs_from_platformio_ini()

    if not platform_names:
        log_fail("В platformio.ini не найдено ни одной платформы (секций [env:...])")
        sys.exit(1)

    # Загружаем platforms.json для определения измеренных базовых размеров платформ
    platforms_data = load_platforms_data()

    log_section("Выбор платформы")
    print()
    print(style("  Выберите платформу:", "cyan", bold=True))
    print()

    for i, name in enumerate(platform_names):
        # Количество разрешённых (не исключённых) модулей для платформы
        allowed = count_modules_for_envs(modules, [name])
        # Количество уже измеренных модулей для платформы (есть числовой размер в modinfo)
        supported = sum(1 for m in modules if module_has_size_for_env(m, name))
        # Количество модулей в ошибке для данной платформы (usedFLASH[name] == "-")
        failed = count_modules_in_error_for_env(modules, name)
        # Цвет имени платформы:
        #   red — платформа отсутствует в platforms.json или одно из значений
        #         (baseline_flash / baseline_ram / total_flash / total_ram) равно 0.
        name_color = "cyan"
        pdata = platforms_data.get(name)
        if pdata is None:
            name_color = "red"
        else:
            values = [
                pdata.get("baseline_flash"),
                pdata.get("baseline_ram"),
                pdata.get("total_flash"),
                pdata.get("total_ram"),
                pdata.get("total_fs"),
            ]
            # Значение 0 или отсутствующее (None) считается «не измеренным»
            if any(v is None or v == 0 for v in values):
                name_color = "red"
        print(
            style(f"    {i + 1}", "green", bold=True) +
            " — " +
            style(f"{name}", name_color) +
            f" обработано {supported} из {allowed}, " +
            style(f"в ошибке {failed}", "red")
        )

    print()
    print(style("    0", "red", bold=True) + " — назад")
    print()

    while True:
        try:
            choice = input(style("  Ваш выбор: ", "yellow")).strip()
            if choice == "0":
                log_info("Назад.")
                return None
            idx = int(choice) - 1
            if 0 <= idx < len(platform_names):
                selected = platform_names[idx]
                log_ok(f"Выбрана платформа: {selected}")
                return [selected]
            log_fail("Некорректный номер. Попробуйте снова.")
        except (EOFError, KeyboardInterrupt):
            print()
            log_fail("Ввод прерван. Выход.")
            sys.exit(1)
        except ValueError:
            log_fail("Введите номер платформы.")


def show_module_menu(modules, envs):
    """
    Показывает перечень всех модулей (независимо от платформы)
    и ждёт выбора пользователя по номеру.

    Используется при выборе опции 0 «1 модуль для всех платформ»:
    меню выбора режима обработки пропускается, а здесь пользователю
    предлагается выбрать ОДИН модуль для измерения на всех платформах.

    Для каждого модуля выводится:
      номер - имя_модуля (путь) — красным: количество платформ, где модуль в ошибке

    Параметры:
      modules: список всех модулей (от collect_modules)
      envs:    список платформ

    Возвращает: список из одного выбранного модуля (list[dict]).
    """
    log_section("Выбор модуля для измерения")
    print()
    print(style("  Выберите модуль:", "cyan", bold=True))
    print()

    for i, mod in enumerate(modules):
        # Количество платформ, где этот модуль в ошибке (usedFLASH[env] == "-")
        failed_count = sum(1 for env in envs if module_in_error_for_env(mod, env))
        err_str = style(f"в ошибке {failed_count}", "red") if failed_count else ""
        print(
            style(f"    {i + 1}", "green", bold=True) +
            " — " +
            style(f"{mod['moduleName']}", "cyan") +
            f" ({mod['path']})" +
            (f" — {err_str}" if err_str else "")
        )

    print()
    print(style("    0", "red", bold=True) + " — назад")
    print()

    while True:
        try:
            choice = input(style("  Ваш выбор: ", "yellow")).strip()
            if choice == "0":
                log_info("Назад.")
                return None
            idx = int(choice) - 1
            if 0 <= idx < len(modules):
                selected = modules[idx]
                log_ok(f"Выбран модуль: {selected['moduleName']}")
                return [selected]
            log_fail("Некорректный номер. Попробуйте снова.")
        except (EOFError, KeyboardInterrupt):
            print()
            log_fail("Ввод прерван. Выход.")
            sys.exit(1)
        except ValueError:
            log_fail("Введите номер модуля.")


def select_measure_scope(module, envs):
    """
    Спрашивает пользователя, для каких платформ измерять выбранный модуль:
      1 — для всех платформ (количество)
      2 — только для платформ в ошибке (количество)

    «В ошибке» означает, что в modinfo.json модуля для данной платформы
    в sizeInfo.usedFLASH стоит "-" (ошибка сборки).

    Параметры:
      module: словарь выбранного модуля (из show_module_menu)
      envs:   список всех платформ

    Возвращает: отфильтрованный список платформ (list[str]).
    """
    # Определяем платформы, где модуль в ошибке (usedFLASH[env] == "-")
    failed_envs = [env for env in envs if module_in_error_for_env(module, env)]

    log_section("Область измерения")
    print()
    print(style("  Модуль: ", "cyan", bold=True) + style(module["moduleName"], "yellow"))
    print()
    print(style("  Для каких платформ измерять?", "cyan", bold=True))
    print(style("    1", "green", bold=True) + f" — для всех платформ ({len(envs)})")
    print(style("    2", "green", bold=True) + f" — для платформ в ошибке ({len(failed_envs)})")
    print(style("    0", "red", bold=True) + " — назад")
    print()

    while True:
        try:
            choice = input(style("  Ваш выбор (0/1/2): ", "yellow")).strip()
            if choice == "0":
                log_info("Назад.")
                return None
            if choice == "1":
                log_ok(f"Измерение для всех платформ ({len(envs)})")
                return envs
            if choice == "2":
                if not failed_envs:
                    log_fail("Нет платформ в ошибке для этого модуля. Выберите 1.")
                else:
                    log_ok(f"Измерение только для платформ в ошибке: {', '.join(failed_envs)}")
                    return failed_envs
            log_fail("Некорректный ввод. Введите 0, 1 или 2.")
        except (EOFError, KeyboardInterrupt):
            print()
            log_fail("Ввод прерван. Выход.")
            sys.exit(1)


# ----------------------------------------------------------------------------
# ПРОВЕРКА ПОДДЕРЖКИ ПЛАТФОРМЫ МОДУЛЕМ
# ----------------------------------------------------------------------------

def module_supports_env(used_libs, env):
    """
    Определяет, поддерживает ли модуль данную платформу (env).

    used_libs — словарь { "<платформа/шаблон>": [библиотеки...] | ["exclude"] }.
    Используется fnmatch для совпадения по шаблону (glob-стиль).
    Если значение ключа содержит "exclude" — модуль НЕ совместим с платформой.

    Поддерживается и старый формат списка шаблонов ["esp32*", ...].

    Примеры:
      used_libs = {"esp32*": ["lib1"], "esp32s2_4mb": ["exclude"]}
        module_supports_env(used_libs, "esp32_4mb")   → True
        module_supports_env(used_libs, "esp32s2_4mb") → False (exclude)

      used_libs = [] → всегда False
    """
    if not used_libs:
        return False
    if isinstance(used_libs, (list, tuple)):
        # старый формат — список шаблонов
        for pattern in used_libs:
            if fnmatch.fnmatch(env, pattern):
                return True
        return False
    # словарь: ключи — платформы/шаблоны, значения — библиотеки или ["exclude"]
    for key, libs in used_libs.items():
        if fnmatch.fnmatch(env, key):
            if isinstance(libs, (list, tuple)) and "exclude" in libs:
                return False
            return True
    return False


def module_allowed_for_env(mod, env):
    """True, если модуль разрешён для платформы env (не исключён в usedLibs)."""
    return module_supports_env(mod.get("usedLibs"), env)


def get_size_entry(mod):
    """
    Возвращает запись sizeInfo текущей версии модуля (для about.moduleVersion).
    Если записи нет — возвращает None.
    """
    modinfo = mod.get("modinfo", {})
    about = modinfo.get("about", {})
    module_version = about.get("moduleVersion")
    for e in modinfo.get("sizeInfo", []) or []:
        if isinstance(e, dict) and e.get("moduleVersion") == module_version:
            return e
    return None


def module_has_size_for_env(mod, env):
    """
    True, если для платформы env у модуля есть числовой размер Flash/RAM.
    Модуль учитывается, только если он разрешён для этой платформы.
    Значение "-" (ошибка) размером не считается.
    """
    if not module_allowed_for_env(mod, env):
        return False
    entry = get_size_entry(mod)
    if entry is None:
        return False
    for key in ("usedFLASH", "usedRAM"):
        d = entry.get(key, {}) or {}
        val = d.get(env)
        if val is not None and (isinstance(val, (int, float)) or (isinstance(val, str) and val != "-")):
            return True
    return False


def module_in_error_for_env(mod, env):
    """
    True, если модуль в ошибке для платформы env: usedFLASH[env] == "-".
    Модуль учитывается, только если он разрешён для этой платформы.
    """
    if not module_allowed_for_env(mod, env):
        return False
    entry = get_size_entry(mod)
    if entry is None:
        return False
    used_flash = entry.get("usedFLASH", {}) or {}
    return used_flash.get(env) == "-"


def count_modules_in_error_for_env(modules, env):
    """Количество модулей, находящихся в ошибке для платформы env."""
    return sum(1 for m in modules if module_in_error_for_env(m, env))


def env_to_base_from_module(used_libs, env):
    """
    Возвращает ключ платформы для записи в sizeInfo.usedFLASH / sizeInfo.usedRAM.
    В текущей реализации просто возвращает env как есть.
    """
    return env


# ----------------------------------------------------------------------------
# ОБНОВЛЕНИЕ sizeInfo В modinfo.json
# ----------------------------------------------------------------------------

def update_size_info(info, module_version, base, used_flash, used_ram):
    """
    Обновляет массив sizeInfo в modinfo.json для указанной версии модуля.

    Структура sizeInfo:
      "sizeInfo": [
        {
          "moduleVersion": "1.0",
          "usedFLASH": { "esp8266_1mb": 12345, ... },
          "usedRAM":   { "esp8266_1mb": 6789, ... }
        },
        ...
      ]

    Алгоритм:
      1. Гарантируем существование массива sizeInfo.
      2. Ищем запись с moduleVersion == module_version.
      3. Если найдена — обновляем usedFLASH[base] и usedRAM[base].
      4. Если не найдена — добавляем новую запись.

    Параметры:
      info:           словарь modinfo.json (изменяется in-place)
      module_version: версия модуля из about.moduleVersion
      base:           ключ платформы (env)
      used_flash:     значение Flash (int или "-")
      used_ram:       значение RAM (int или "-")
    """
    # Гарантируем существование массива sizeInfo
    if "sizeInfo" not in info or not isinstance(info["sizeInfo"], list):
        info["sizeInfo"] = []

    # Ищем запись с matching moduleVersion
    entry = None
    for e in info["sizeInfo"]:
        if isinstance(e, dict) and e.get("moduleVersion") == module_version:
            entry = e
            break

    # Если не найдена — создаём новую
    if entry is None:
        entry = {
            "moduleVersion": module_version,
            "usedFLASH": {},
            "usedRAM": {},
        }
        info["sizeInfo"].append(entry)

    # Гарантируем существование словарей usedFLASH / usedRAM
    if "usedFLASH" not in entry or not isinstance(entry["usedFLASH"], dict):
        entry["usedFLASH"] = {}
    if "usedRAM" not in entry or not isinstance(entry["usedRAM"], dict):
        entry["usedRAM"] = {}

    # Записываем значения для платформы
    entry["usedFLASH"][base] = used_flash
    entry["usedRAM"][base] = used_ram


# ----------------------------------------------------------------------------
# СОЗДАНИЕ ПРОФИЛЯ СБОРКИ
# ----------------------------------------------------------------------------

def build_profile_with_modules(prof_template, active_paths, env):
    """
    На основе шаблона профиля создаёт новую конфигурацию,
    в которой активны только указанные модули.

    Параметры:
      prof_template:  исходный словарь профиля (myProfile.json)
      active_paths:   множество путей модулей, которые должны быть активны
      env:            целевая платформа

    Алгоритм:
      1. Глубоко копируем шаблон (чтобы не мутировать оригинал).
      2. Проходим по всем разделам модулей в профиле.
      3. Для каждого модуля устанавливаем active=True, если его путь в active_paths.
      4. Устанавливаем default_envs = env для платформы.
      5. Возвращаем новый словарь профиля.
    """
    prof = json.loads(json.dumps(prof_template))
    for section, mods in prof.get("modules", {}).items():
        for m in mods:
            m["active"] = m["path"] in active_paths
    prof["projectProp"]["platformio"]["default_envs"] = env
    return prof


# ----------------------------------------------------------------------------
# ЗАПУСК PrepareProject.py
# ----------------------------------------------------------------------------

def run_prepare_project(profile_path, env):
    """
    Запускает PrepareProject.py для генерации конфигурации проекта
    на основе профиля и платформы.

    Параметры:
      profile_path: путь к JSON-файлу профиля
      env:          целевая платформа

    Возвращает: True если подготовка прошла успешно, False иначе.
    """
    cmd = [sys.executable, "PrepareProject.py", "-p", str(profile_path), "-b", env]
    r = subprocess.run(cmd, cwd=PROJECT_ROOT, capture_output=True, text=True, timeout=120)
    if r.returncode != 0:
        print(r.stderr or r.stdout, file=sys.stderr)
    return r.returncode == 0


# ----------------------------------------------------------------------------
# ИЗМЕРЕНИЕ РАЗМЕРА ПРОШИВКИ (Flash + RAM)
# ----------------------------------------------------------------------------

def get_size_from_output(env, timeout=1800):
    """
    Запускает `pio run -e ENV` (полная сборка) и парсит размеры из вывода PlatformIO.

    ВАЖНО: строки сводки памяти "RAM:" / "Flash:" появляются ТОЛЬКО при обычной
    сборке (`pio run`). При `pio run -t size` их НЕТ — там только таблица размеров
    объектных секций (raw_firmware.elf). Поэтому используется полная сборка.

    Параметры:
      env:    целевая платформа (например, "bk7231n")
      timeout: таймаут в секундах (по умолчанию 30 минут)

    Возвращает:
      dict с ключами:
        flash_used  — размер занятой Flash (байты), или None
        flash_total — общий размер Flash платформы (байты), или None
        ram_used    — размер занятой RAM (байты), или None
        ram_total   — общий размер RAM платформы (байты), или None
      или None при ошибке.

    Алгоритм парсинга:
      1. Запускаем `pio run -e ENV` (полная сборка).
      2. ОСНОВНОЙ СПОСОБ — строки сводки памяти:
           RAM:   [====      ]  36.8% (used 96364 bytes from 262144 bytes)
           Flash: [========  ]  75.9% (used 822088 bytes from 1083136 bytes)
         Из них берутся used (занято) и from (всего) для RAM и Flash.
      3. РЕЗЕРВНЫЙ СПОСОБ — строка таблицы размеров .elf:
           Формат: "  text   data    bss    dec   hex  filename"
           Пример: " 823888	   2632	 107140	 933660	  e3f1c	...raw_firmware.elf"
         Flash = text + data, RAM = bss. Этот способ используется,
         если сводка RAM:/Flash: не найдена (другие платформы).
      4. ПОСЛЕДНИЙ РЕЗЕРВ — строка "used XXXXX bytes".
      5. Возвращаем словарь {flash_used, flash_total, ram_used, ram_total}.
    """
    cmd = [PIO_BIN or "pio", "run", "-e", env]

    # Запускаем pio с потоковым выводом в консоль и одновременным захватом вывода
    # для последующего парсинга размеров Flash/RAM.
    proc = subprocess.Popen(
        cmd, cwd=PROJECT_ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, bufsize=1, encoding="utf-8", errors="replace"
    )
    chunks = []
    try:
        assert proc.stdout is not None
        for line in proc.stdout:
            print(line, end="")          # отображаем вывод pio в консоли
            chunks.append(line)
        proc.wait(timeout=timeout)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait()
        log_fail(f"Таймаут сборки pio для env={env}")
        return None

    out = "".join(chunks)
    if proc.returncode != 0:
        return None

    result = {"flash_used": None, "flash_total": None, "ram_used": None, "ram_total": None}

    # =========================================================================
    # ОСНОВНОЙ СПОСОБ: строки сводки памяти RAM: / Flash:
    # "Flash: [========  ]  75.9% (used 822088 bytes from 1083136 bytes)"
    # "RAM:   [====      ]  36.8% (used 96364 bytes from 262144 bytes)"
    # =========================================================================
    flash_match = re.search(r"Flash:.*used\s+(\d+)\s+bytes\s+from\s+(\d+)\s+bytes", out)
    if flash_match:
        result["flash_used"] = int(flash_match.group(1))
        result["flash_total"] = int(flash_match.group(2))

    ram_match = re.search(r"RAM:.*used\s+(\d+)\s+bytes\s+from\s+(\d+)\s+bytes", out)
    if ram_match:
        result["ram_used"] = int(ram_match.group(1))
        result["ram_total"] = int(ram_match.group(2))

    # =========================================================================
    # РЕЗЕРВНЫЙ СПОСОБ: таблица размеров .elf
    # "  text   data    bss    dec   hex  filename"
    # " 823888	   2632	 107140	 933660	  e3f1c	.pio\build\bk7231n\raw_firmware.elf"
    # =========================================================================
    if result["flash_used"] is None and result["ram_used"] is None:
        for line in out.split("\n"):
            if ".elf" not in line:
                continue
            parts = line.split()
            # Первый токен должен быть числом (text), отсекаем "Calculating size ..."
            if len(parts) >= 4:
                try:
                    text = int(parts[0])
                    data = int(parts[1])
                    bss = int(parts[2])
                except (ValueError, IndexError):
                    continue
                result["flash_used"] = text + data  # program (flash) size
                result["ram_used"] = bss
                break

    # =========================================================================
    # ПОСЛЕДНИЙ РЕЗЕРВ: строка "used XXXXX bytes"
    # =========================================================================
    if result["flash_used"] is None:
        match = re.search(r"used\s+(\d+)\s+bytes", out)
        if match:
            result["flash_used"] = int(match.group(1))

    # Если хотя бы одно значение не определено — заменяем на 0
    for key in result:
        if result[key] is None:
            result[key] = 0

    return result


# ----------------------------------------------------------------------------
# ФАЙЛОВАЯ СИСТЕМА (FS)
# ----------------------------------------------------------------------------

# Имена собранного образа ФС:
#   littlefs.bin / spiffs.bin — обычный таргет buildfs, образ в .pio/build/<env>/;
#   lt_littlefs.bin           — LibreTiny (bk7231n), образ кладёт кастомный таргет
#                               tools/lt_fsbuild.py в корень проекта (PROJECT_DIR).
FS_IMAGE_NAMES = ("littlefs.bin", "spiffs.bin")
LT_FS_IMAGE_NAME = "lt_littlefs.bin"
LT_FS_SCRIPT = "lt_fsbuild.py"     # признак env с LibreTiny-ФС в platformio.ini

def dir_size(path):
    """Суммарный размер всех файлов в каталоге (байты)."""
    if not path or not os.path.isdir(path):
        return 0
    total = 0
    for root, _dirs, files in os.walk(path):
        for f in files:
            try:
                total += os.path.getsize(os.path.join(root, f))
            except OSError:
                pass
    return total


def get_fs_total(env, timeout=600):
    """Ёмкость раздела ФС — размер собранного образа littlefs.bin (spiffs.bin).

    Выполняет `pio run -t buildfs -e ENV`, как при нажатии кнопки Build,
    и возвращает размер полученного образа.

    Где искать образ:
      * .pio/build/<env>/littlefs.bin — ESP (esp8266/esp32), обычный таргет buildfs;
      * .pio/build/<env>/spiffs.bin  — если FS проекта на базе spiffs;
      * lt_littlefs.bin               — LibreTiny (bk7231n): образ собирает кастомный
                                        таргет tools/lt_fsbuild.py и кладёт его в корень
                                        проекта (PROJECT_DIR), а не в .pio/build/<env>.

    Размер образа равен ёмкости раздела ФС, поэтому размер файла и есть «total_fs».
    """
    cmd = [PIO_BIN or "pio", "run", "-t", "buildfs", "-e", env]
    try:
        proc = subprocess.run(cmd, cwd=PROJECT_ROOT, capture_output=True, text=True,
                              encoding="utf-8", errors="replace", timeout=timeout)
    except subprocess.TimeoutExpired:
        log_fail(f"Таймаут buildfs для env={env}")
        return 0
    if proc.returncode != 0:
        log_fail(f"buildfs завершился с ошибкой для env={env}")
        return 0
    return fs_image_size(env)


def fs_image_size(env):
    """Размер свежесобранного образа ФС (0, если образа нет).

    Имена образов и каталоги поиска — см. FS_IMAGE_NAMES и get_fs_total().
    Образ lt_littlefs.bin (LibreTiny) ищется только для env с tools/lt_fsbuild.py
    в platformio.ini, чтобы не принять образ другой платформы за свой.
    """
    names = list(FS_IMAGE_NAMES)
    bases = [os.path.join(PROJECT_ROOT, ".pio", "build", env)]
    if is_lt_env(env):
        names.append(LT_FS_IMAGE_NAME)
        bases.append(str(PROJECT_ROOT))
    for base in bases:
        for name in names:
            p = os.path.join(base, name)
            if os.path.isfile(p):
                try:
                    size = os.path.getsize(p)
                except OSError:
                    continue
                if size:
                    return size
    return 0


def is_lt_env(env):
    """True, если env собирает ФС кастомным таргетом LibreTiny (tools/lt_fsbuild.py).

    Признак берём из platformio.ini (секция [env:<env>], поле extra_scripts).
    """
    if not env:
        return False
    try:
        cp = configparser.ConfigParser(strict=False, interpolation=None)
        cp.read(str(PROJECT_ROOT / "platformio.ini"), encoding="utf-8")
        scripts = cp.get(f"env:{env}", "extra_scripts", fallback="")
    except Exception:  # noqa: BLE001 — при любой проблеме считаем env обычной ESP
        return False
    return LT_FS_SCRIPT in scripts


def fs_used_dir(project_dir, env):
    """Занятое в ФС — размер папки data_svelte проекта.

    data_svelte всегда лежит в корне проекта (<project>/data_svelte).
    Параметр env оставлен для совместимости вызова.
    """
    if not project_dir:
        return 0
    return dir_size(os.path.join(project_dir, "data_svelte"))


# ----------------------------------------------------------------------------
# ЗАГРУЗКА / СОХРАНЕНИЕ platforms.json
# ----------------------------------------------------------------------------

def load_platforms_data():
    """
    Загружает данные о платформах из tools/measure_size/platforms.json.
    Если файл не существует — возвращает пустой словарь.

    Структура (история замеров, новый формат):
      {
        "esp8266_1mb": [
          {
            "FIRMWARE_VERSION": "463.2225",       # из include/Const.h
            "datetime": "21.09.2026 00:02:20",    # время замера
            "baseline_flash": 448947,
            "baseline_ram": 37276,
            "total_flash": 761840,
            "total_ram": 81920,
            "total_fs": 262144
          },
          ...
        ],
        "bk7231n": [ ... ]
      }
    Актуальный замер платформы — ПОСЛЕДНИЙ элемент списка.
    Старый формат (плоский словарь без истории) тоже читается.
    """
    if PLATFORMS_JSON.is_file():
        return load_json(PLATFORMS_JSON)
    return {}


def latest_platform_record(platforms_data, env):
    """
    Последняя (самая свежая) запись замера платформы из platforms.json.

    Платформа хранится как список-история замеров — берётся последний
    элемент. Старый формат (плоский словарь) поддерживается: возвращается
    он сам. Для неизвестной платформы — пустой словарь.
    """
    entry = platforms_data.get(env)
    if isinstance(entry, list) and entry:
        rec = entry[-1]
        return rec if isinstance(rec, dict) else {}
    if isinstance(entry, dict):
        return entry
    return {}


def read_firmware_version():
    """Читает версию прошивки FIRMWARE_VERSION из include/Const.h."""
    const_path = PROJECT_ROOT / "include" / "Const.h"
    try:
        text = const_path.read_text(encoding="utf-8", errors="replace")
        m = re.search(
            r'^[ \t]*#[ \t]*define[ \t]+FIRMWARE_VERSION[ \t]+"([^"]*)"',
            text, re.MULTILINE,
        )
        if m:
            return m.group(1)
    except OSError:
        pass
    return "unknown"


def save_platforms_data(data):
    """
    Сохраняет данные о платформах в tools/measure_size/platforms.json.
    Перезаписывает весь файл (для инициализации).
    """
    save_json(PLATFORMS_JSON, data)


def update_platforms_data(env, sizes, fw_version=None):
    """
    Дописывает замер платформы в историю tools/measure_size/platforms.json.

    Каждая платформа — список записей; новый замер добавляется в конец
    с полями FIRMWARE_VERSION (из include/Const.h) и datetime (время замера).
    Старый формат (плоский словарь) при обнаружении переносится в историю
    первой записью.

    Параметры:
      env:   ключ платформы (например, "bk7231n")
      sizes: словарь с полями {baseline_flash, baseline_ram, total_flash, total_ram, total_fs}
      fw_version: версия прошивки; если не задана — читается из Const.h

    Возвращает дельту baseline_flash относительно предыдущей записи (int)
    или None, если числовых предыдущих замеров не было.
    """
    platforms = load_platforms_data()

    # Если файл не существовал — создаём пустую структуру
    if not platforms:
        platforms = {}

    entry = platforms.get(env)
    # Миграция старого формата: плоский словарь -> история с одной записью
    if isinstance(entry, dict):
        entry = [entry]
        platforms[env] = entry
    elif not isinstance(entry, list):
        entry = []
        platforms[env] = entry

    # Предыдущий числовой baseline — для расчёта дельты
    prev_baseline = None
    for rec in reversed(entry):
        if isinstance(rec, dict) and isinstance(rec.get("baseline_flash"), (int, float)):
            prev_baseline = int(rec["baseline_flash"])
            break

    record = {
        "FIRMWARE_VERSION": fw_version if fw_version is not None else read_firmware_version(),
        "datetime": datetime.now().strftime("%d.%m.%Y %H:%M:%S"),
        "baseline_flash": sizes["baseline_flash"],
        "baseline_ram": sizes["baseline_ram"],
        "total_flash": sizes["total_flash"],
        "total_ram": sizes["total_ram"],
    }
    if "total_fs" in sizes:
        record["total_fs"] = sizes["total_fs"]
    entry.append(record)

    save_json(PLATFORMS_JSON, platforms)

    if prev_baseline is not None:
        return int(sizes["baseline_flash"]) - prev_baseline
    return None


def choose_baseline_source(envs):
    """
    Меню выбора источника базовых размеров (последнее меню).

    Если в platforms.json уже есть полные базовые размеры для ВСЕХ выбранных
    платформ — показывается меню:
      1 — получить из предыдущих замеров
      2 — произвести новые замеры базовых размеров
      0 — назад
    Если размеров нет — меню пропускается, сразу назначаются новые замеры.

    Параметры:
      envs: список выбранных платформ

    Возвращает:
      'prev' — использовать размеры из platforms.json
      'build'— произвести новые замеры (сборка)
      None   — назад
    """
    platforms_data = load_platforms_data()
    all_present = True
    for env in envs:
        pdata = latest_platform_record(platforms_data, env)
        vals = [
            pdata.get("baseline_flash"),
            pdata.get("baseline_ram"),
            pdata.get("total_flash"),
            pdata.get("total_ram"),
            pdata.get("total_fs"),
        ]
        if not all(isinstance(v, (int, float)) and v > 0 for v in vals):
            all_present = False
            break

    # Если размеров нет — меню не показываем, сразу новые замеры
    if not all_present:
        log_step("В platforms.json нет базовых размеров — будут выполнены новые замеры.")
        return "build"

    log_section("Базовые размеры (без модулей)")
    print()
    print(style("  Выберите источник базовых размеров:", "cyan", bold=True))
    print()
    print(style("    1", "green", bold=True) + " — получить из предыдущих замеров")
    print(style("    2", "green", bold=True) + " — произвести новые замеры базовых размеров")
    print(style("    0", "red", bold=True) + " — назад")
    print()

    while True:
        try:
            choice = input(style("  Ваш выбор (0/1/2): ", "yellow")).strip()
            if choice == "1":
                log_ok("Использовать предыдущие замеры из platforms.json")
                return "prev"
            if choice == "2":
                log_ok("Будут выполнены новые замеры базовых размеров")
                return "build"
            if choice == "0":
                log_info("Назад.")
                return None
            log_fail("Некорректный ввод. Введите 0, 1 или 2.")
        except (EOFError, KeyboardInterrupt):
            print()
            log_fail("Ввод прерван. Выход.")
            sys.exit(1)


def select_interactive_config(modules, prof_template, need_baseline=True):
    """
    Интерактивный выбор конфигурации измерения с поддержкой «назад» в меню.

    Первое меню (выбор режима) — «0» = выход из программы.
    Остальные меню — «0» = назад к предыдущему.

    Последнее меню — выбор источника базовых размеров (choose_baseline_source):
      1 — получить из предыдущих замеров
      2 — произвести новые замеры
      0 — назад
    Если в platforms.json нет базовых размеров, меню пропускается
    и сразу назначаются новые замеры.

    Параметры:
      modules:        список всех модулей (от collect_modules)
      prof_template:  словарь профиля сборки
      need_baseline:  True — запрашивать источник базовых размеров
                      (False для dry-run, где базовые размеры не нужны)

    Возвращает кортеж (envs, single_module_mode, modules, done, baseline_source):
      envs               — список выбранных платформ
      single_module_mode — True, если выбран режим 2 (один модуль для всех платформ)
      modules            — итоговый список модулей для измерения
      done               — True (интерактивный выбор завершён)
      baseline_source    — 'prev' (из platforms.json) или 'build' (новые замеры)
    """
    while True:
        # Первое меню — «0» = выход
        top_mode = show_top_mode_menu()

        if top_mode == 4:
            # Режим 4: все модули для всех платформ.
            # Последнее меню (источник базовых размеров) НЕ показывается —
            # все базовые размеры будут переизмерены заново.
            envs = get_envs_from_platformio_ini()
            log_section("Все модули для всех платформ")
            log_ok(f"Платформы: {', '.join(envs)} ({len(envs)})")
            log_info("Все базовые размеры будут переизмерены.")
            return envs, False, modules, True, "build"

        if top_mode == 2:
            # Режим 2: один модуль для всех платформ
            envs = get_envs_from_platformio_ini()
            while True:
                chosen = show_module_menu(modules, envs)           # 0 = назад
                if chosen is None:
                    break  # назад -> первое меню
                while True:
                    scope_envs = select_measure_scope(chosen[0], envs)  # 0 = назад
                    if scope_envs is None:
                        break  # назад -> перечень модулей
                    bs = choose_baseline_source(scope_envs) if need_baseline else "build"
                    if bs is None:
                        continue  # назад -> область измерения
                    return scope_envs, True, chosen, True, bs

        if top_mode == 3:
            # Режим 3: один модуль для одной платформы
            while True:
                envs = show_platform_menu(modules)                 # 0 = назад
                if envs is None:
                    break  # назад -> первое меню
                while True:
                    chosen = show_module_menu(modules, envs)       # 0 = назад
                    if chosen is None:
                        break  # назад -> выбор платформы
                    # Платформа уже выбрана во втором меню,
                    # поэтому вопрос «для каких платформ измерять?» не задаём.
                    bs = choose_baseline_source(envs) if need_baseline else "build"
                    if bs is None:
                        continue  # назад -> перечень модулей
                    return envs, True, chosen, True, bs

        # Режим 1: все модули для одной платформы
        while True:
            envs = show_platform_menu(modules)                     # 0 = назад
            if envs is None:
                break  # назад -> первое меню

            profile_modules = filter_modules_by_profile(modules, prof_template)
            without_size = filter_modules_without_size(modules, envs)
            all_count = count_modules_for_envs(modules, envs)
            profile_count = count_modules_for_envs(profile_modules, envs)
            without_size_count = len(without_size)

            while True:
                choice = show_menu(all_count, profile_count, without_size_count)  # 0 = назад
                if choice is None:
                    break  # назад -> меню платформы
                chosen = modules
                if choice == 2:
                    chosen = profile_modules
                elif choice == 3:
                    chosen = without_size
                bs = choose_baseline_source(envs) if need_baseline else "build"
                if bs is None:
                    continue  # назад -> режим обработки
                return envs, False, chosen, True, bs


# ----------------------------------------------------------------------------
# ОСНОВНАЯ ФУНКЦИЯ
# ----------------------------------------------------------------------------

def main():
    global USE_COLOR
    ap = argparse.ArgumentParser(
        description="Измерение размера Flash/RAM модулей и запись в modinfo.json + platforms.json"
    )
    ap.add_argument("--env", action="append", default=[],
                    help="Платформа (например, esp32_4mb). Можно повторять.")
    ap.add_argument("--profile", default="myProfile.json",
                    help="Файл профиля сборки (по умолчанию myProfile.json)")
    ap.add_argument("--dry-run", action="store_true",
                    help="Показать список модулей без сборки")
    ap.add_argument("--no-color", action="store_true",
                    help="Отключить цветной вывод")
    ap.add_argument("--limit", type=int, default=None, metavar="N",
                    help="Измерить только первые N модулей на платформу")
    ap.add_argument("--pio", default=None, metavar="PATH",
                    help="Полный путь к исполняемому файлу pio (иначе — из PATH)")
    ap.add_argument("--mode", type=int, choices=[1, 2, 3], default=None,
                    help="Режим обработки без интерактивного меню: 1=все, 2=модули профиля, 3=без размера")
    ap.add_argument("--baseline", choices=["build", "prev"], default=None,
                    help="Источник базовых размеров: build=новая baseline-сборка, prev=из platforms.json")
    ap.add_argument("--module", default=None, metavar="PATH",
                    help="Измерить только один модуль (путь или имя папки модуля)")
    ap.add_argument("--baseline-only", action="store_true",
                    help="Измерить только базовую прошивку (без модулей) и обновить platforms.json")
    ap.add_argument("--abort-file", default=None, metavar="PATH",
                    help="Файл-флаг мягкого прерывания: при его появлении замер останавливается")
    ap.add_argument("--project-dir", default=None, metavar="DIR",
                    help="Каталог выбранного проекта (для расчёта размера папки data_svelte в корне проекта)")
    args = ap.parse_args()

    global ABORT_FILE
    if args.abort_file:
        ABORT_FILE = args.abort_file
        # Удаляем возможный «остаток» от предыдущего запуска
        try:
            if os.path.exists(ABORT_FILE):
                os.unlink(ABORT_FILE)
        except OSError:
            pass

    if args.no_color:
        USE_COLOR = False

    # -------------------------------------------------------------------------
    # ОПРЕДЕЛЕНИЕ ПРОФИЛЯ: ищем профиль сборки в порядке приоритета:
    #   1. Файл, указанный через --profile (по умолчанию myProfile.json)
    #   2. compilerProfile.json (альтернативное имя)
    # -------------------------------------------------------------------------
    profile_path = PROJECT_ROOT / args.profile
    if not profile_path.is_file():
        profile_path = PROJECT_ROOT / "compilerProfile.json"
    if not profile_path.is_file():
        log_fail("Профиль не найден: myProfile.json или compilerProfile.json")
        sys.exit(1)

    prof_template = load_json(profile_path)
    modules = collect_modules()
    baseline_paths = set(BASELINE_MODULE_PATHS)  # пустое множество — без модулей

    # -------------------------------------------------------------------------
    # ЗАПУСК ЛОГИРОВАНИЯ В ФАЙЛ:
    #   Создаём папку measure_size/logs/[дата_время] и пишем туда весь вывод.
    #   Логирование запускается ДО интерактивных меню, чтобы весь выбор попал в лог.
    # -------------------------------------------------------------------------
    log_dir = start_logging()
    print()
    log_ok(f"Лог запуска сохранён: {log_dir}")

    # -------------------------------------------------------------------------
    # ОПРЕДЕЛЕНИЕ ПЛАТФОРМ И ИНТЕРАКТИВНЫЙ ВЫБОР:
    #   1. Если указан --env — используем его.
    #   2. Если DEFAULT_ENVS не пуст — используем его.
    #   3. Иначе — глобальное меню режима с навигацией «назад».
    # -------------------------------------------------------------------------
    single_module_mode = False
    interactive_done = False
    baseline_source = "build"
    non_interactive = not sys.stdin.isatty()
    if args.pio:
        global PIO_BIN
        PIO_BIN = args.pio

    if args.env:
        envs = args.env
    elif DEFAULT_ENVS:
        envs = DEFAULT_ENVS
    else:
        envs, single_module_mode, modules, interactive_done, baseline_source = \
            select_interactive_config(modules, prof_template, need_baseline=not args.dry_run)

    # -------------------------------------------------------------------------
    # РЕЖИМ ОДНОГО МОДУЛЯ (--module): замер только указанного модуля.
    #   Переопределяет выбор модулей, пропускает меню режима и baseline.
    # -------------------------------------------------------------------------
    if args.module:
        target = Path(args.module).as_posix().rstrip("/")
        wanted = [m for m in modules if _module_matches(m, target)]
        if not wanted:
            log_fail(f"Модуль не найден: {args.module}")
            stop_logging()
            sys.exit(1)
        modules = wanted
        single_module_mode = True
        interactive_done = True
        baseline_source = args.baseline if args.baseline else "prev"

    # -------------------------------------------------------------------------
    # РЕЖИМ «ТОЛЬКО БАЗА» (--baseline-only): замер лишь базовой прошивки (без модулей).
    #   Меню выбора модулей не нужно; всегда выполняется свежая baseline-сборка.
    # -------------------------------------------------------------------------
    if args.baseline_only:
        interactive_done = True
        baseline_source = "build"

    # -------------------------------------------------------------------------
    # МЕНЮ ВЫБОРА РЕЖИМА ОБРАБОТКИ:
    #   Показывается только при задании платформы через --env / DEFAULT_ENVS,
    #   т.к. при интерактивном выборе режим уже выбран внутри select_interactive_config.
    #   При --mode или неинтерактивном stdin меню пропускается.
    # -------------------------------------------------------------------------
    if not interactive_done and not single_module_mode:
        profile_modules = filter_modules_by_profile(modules, prof_template)
        without_size = filter_modules_without_size(modules, envs)

        all_count = count_modules_for_envs(modules, envs)
        profile_count = count_modules_for_envs(profile_modules, envs)
        without_size_count = len(without_size)

        if args.mode is not None:
            choice = args.mode
        elif non_interactive:
            choice = 1  # нет интерактивного ввода — берём все модули
        else:
            choice = show_menu(all_count, profile_count, without_size_count)
            if choice is None:
                choice = 1  # «назад» без предыдущего меню — берём все модули

        if choice == 1:
            # Все модули — ничего не фильтруем
            log_step("Режим 1: обработка ВСЕХ модулей")
        elif choice == 2:
            # Только модули из профиля
            modules = profile_modules
            log_step(f"Режим 2: обработка модулей из myProfile.json ({len(modules)})")
        elif choice == 3:
            # Только модули без размера
            modules = without_size
            log_step(f"Режим 3: обработка модулей без информации о размере ({len(modules)})")

        # ---------------------------------------------------------------------
        # Последнее меню — источник базовых размеров (для не-интерактивного пути)
        # ---------------------------------------------------------------------
        if not args.dry_run:
            if args.baseline:
                baseline_source = args.baseline
            elif non_interactive:
                baseline_source = "build"
            else:
                bs = choose_baseline_source(envs)
                if bs is None:
                    bs = "build"  # «назад» без предыдущего меню — новые замеры
                baseline_source = bs

    # -------------------------------------------------------------------------
    # Если выбран режим 2 «один модуль для всех платформ» — логируем итоговый выбор.
    # -------------------------------------------------------------------------
    if single_module_mode:
        log_step(f"Режим: 1 модуль ({modules[0]['moduleName']}) — платформы: {', '.join(envs)}")

    # -------------------------------------------------------------------------
    # ГРУППИРОВКА МОДУЛЕЙ ПО ПЛАТФОРМАМ:
    #   Для каждой платформы определяем, какие модули её поддерживают.
    # -------------------------------------------------------------------------
    modules_by_env = {e: [m for m in modules if module_supports_env(m["usedLibs"], e)] for e in envs}

    # ОГРАНИЧЕНИЕ КОЛИЧЕСТВА МОДУЛЕЙ:
    #   Если указан --limit N, берём только первые N модулей на платформу.
    if args.limit is not None:
        modules_by_env = {e: mods[: args.limit] for e, mods in modules_by_env.items()}

    # -------------------------------------------------------------------------
    # DRY RUN — только показать информацию, не собирать
    # -------------------------------------------------------------------------
    if args.dry_run:
        log_section("Dry run — модули по платформам")
        for e in envs:
            log_ok(f"{e}: {len(modules_by_env[e])} модулей")
        log_info("Примеры модулей:")
        for m in modules[:5]:
            log_info(f"{m['path']} → {m['moduleName']}")
        print()
        stop_logging()
        return

    # -------------------------------------------------------------------------
    # НАЧАЛО ИЗМЕРЕНИЙ
    # -------------------------------------------------------------------------
    log_section("Измерение размера Flash/RAM модулей")
    log_step(f"Профиль: {profile_path.name}")
    log_step(f"Платформы: {', '.join(envs)}")
    total_modules = sum(len(modules_by_env[e]) for e in envs)
    limit_note = f" (первые {args.limit} на платформу)" if args.limit is not None else ""
    log_step(f"Всего сборок: baseline × {len(envs)} + {total_modules} модулей {limit_note}")

    # -------------------------------------------------------------------------
    # СОХРАНЕНИЕ platformio.ini — будет модифицироваться PrepareProject.py
    # -------------------------------------------------------------------------
    platformio_ini = PROJECT_ROOT / "platformio.ini"
    platformio_backup = PROJECT_ROOT / "platformio.ini.measure.bak"
    original_env = prof_template["projectProp"]["platformio"].get(
        "default_envs", envs[0] if envs else "esp32_4mb"
    )
    shutil.copy(platformio_ini, platformio_backup)

    failures = 0
    aborted = False
    try:
        # -------------------------------------------------------------------------
        # ЭТАП 1: БАЗОВЫЕ СБОРКИ (baseline — БЕЗ МОДУЛЕЙ)
        # -------------------------------------------------------------------------
        _check_abort()
        # Собираем прошивку БЕЗ КАКИХ-ЛИБО модулей для каждой платформы.
        # Парсим из вывода:
        #   baseline_flash = flash_used (занятая Flash без модулей)
        #   baseline_ram   = ram_used   (занятая RAM без модулей)
        #   total_flash    = flash_total (общий Flash платформы)
        #   total_ram      = ram_total   (общая RAM платформы)
        # -------------------------------------------------------------------------
        baseline_sizes = {}
        platforms_data = load_platforms_data()

        # Если запрошены «предыдущие замеры», но платформы ещё нет в platforms.json —
        # автоматически выполняем новую baseline-сборку.
        if baseline_source == "prev":
            missing_prev = [e for e in envs
                            if not latest_platform_record(platforms_data, e).get("total_fs")]
            if missing_prev:
                log_step(
                    "Нет базовых замеров для: " + ", ".join(missing_prev)
                    + " — выполняется новая baseline-сборка"
                )
                baseline_source = "build"

        if baseline_source == "prev":
            # -----------------------------------------------------------------
            # ИСТОЧНИК «ПРЕДЫДУЩИЕ ЗАМЕРЫ»:
            #   Базовая сборка (без модулей) НЕ выполняется.
            #   Размеры базовых прошивок берутся напрямую из platforms.json.
            # -----------------------------------------------------------------
            log_section("Базовые размеры (из platforms.json)")
            for env in envs:
                pdata = latest_platform_record(platforms_data, env)
                baseline_sizes[env] = {
                    "baseline_flash": int(pdata.get("baseline_flash") or 0),
                    "baseline_ram": int(pdata.get("baseline_ram") or 0),
                    "total_flash": int(pdata.get("total_flash") or 0),
                    "total_ram": int(pdata.get("total_ram") or 0),
                    "total_fs": int(pdata.get("total_fs") or 0),
                }
                log_ok(
                    f"{env}: baseline взят из platforms.json "
                    f"(Flash={baseline_sizes[env]['baseline_flash']:,} B, "
                    f"RAM={baseline_sizes[env]['baseline_ram']:,} B)"
                )
        else:
            # -----------------------------------------------------------------
            # ИСТОЧНИК «НОВЫЕ ЗАМЕРЫ»: выполняется базовая сборка БЕЗ МОДУЛЕЙ.
            # -----------------------------------------------------------------
            log_section("Базовая сборка (без модулей)")
            for env in envs:
                prof = build_profile_with_modules(prof_template, baseline_paths, env)
                with tempfile.NamedTemporaryFile(mode="w", suffix=".json", delete=False) as f:
                    save_json(f.name, prof)
                    ok = run_prepare_project(f.name, env)
                os.unlink(f.name)
                if not ok:
                    log_fail(f"PrepareProject не удался для baseline env={env}")
                    failures += 1
                    continue
                sizes = get_size_from_output(env)
                if sizes is None:
                    log_fail(f"Не удалось получить размер для baseline env={env}")
                    failures += 1
                    continue
                # Собираем файловую систему (buildfs) и определяем ёмкость раздела ФС
                total_fs = get_fs_total(env)
                fs_used = fs_used_dir(args.project_dir, env)
                baseline_sizes[env] = {
                    "baseline_flash": sizes["flash_used"],
                    "baseline_ram": sizes["ram_used"],
                    "total_flash": sizes["flash_total"],
                    "total_ram": sizes["ram_total"],
                    "total_fs": total_fs,
                }
                log_ok(
                    f"{env}: baseline Flash={sizes['flash_used']:,} B, "
                    f"baseline RAM={sizes['ram_used']:,} B, "
                    f"total Flash={sizes['flash_total']:,} B, "
                    f"total RAM={sizes['ram_total']:,} B, "
                    f"total FS={total_fs:,} B, FS занято={fs_used:,} B"
                )

        # -------------------------------------------------------------------------
        # СОХРАНЕНИЕ platforms.json сразу после базовой сборки
        # -------------------------------------------------------------------------
        # Если использованы предыдущие замеры — перезаписывать их не нужно.
        # Обновление выполняется только после новых замеров (baseline_source == 'build').
        if baseline_source == "build":
            for env in envs:
                if env in baseline_sizes:
                    delta = update_platforms_data(env, baseline_sizes[env])
                    if delta is None:
                        log_ok(f"{env}: platforms.json — первая запись истории замеров")
                    elif delta == 0:
                        log_ok(f"{env}: размер базы не изменился (0 B)")
                    else:
                        log_ok(f"{env}: размер базы изменился на {delta:+d} B")
            log_ok(f"platforms.json обновлён: {PLATFORMS_JSON}")

        # -------------------------------------------------------------------------
        # РЕЖИМ «ТОЛЬКО БАЗА»: измеряем лишь базовую прошивку (без модулей),
        # platforms.json уже обновлён выше. Пропускаем цикл измерения модулей.
        # -------------------------------------------------------------------------
        _check_abort()
        if args.baseline_only:
            log_section("Замер базовой прошивки завершён (без модулей)")

        # -------------------------------------------------------------------------
        # ЭТАП 2: ИЗМЕРЕНИЕ МОДУЛЕЙ
        # -------------------------------------------------------------------------
        # Для каждого модуля и каждой платформы:
        #   1. Создаём профиль: базовые модули + текущий модуль.
        #   2. Строим прошивку, парсим flash_used и ram_used.
        #   3. Вычисляем дельту: (модуль) - baseline.
        #   4. Записываем about.usedFLASH и about.usedRAM в modinfo.json.
        # -------------------------------------------------------------------------
        updated = 0
        updated_paths = set()
        idx = 0

        if not args.baseline_only:
          for env in envs:
            log_section(f"Измерение модулей — {env}")
            mods = modules_by_env[env]
            for i, mod in enumerate(mods):
                _check_abort()
                idx += 1
                active_paths = baseline_paths | {mod["path"]}

                # Создаём профиль и запускаем подготовку
                prof = build_profile_with_modules(prof_template, active_paths, env)
                with tempfile.NamedTemporaryFile(mode="w", suffix=".json", delete=False) as f:
                    save_json(f.name, prof)
                    ok = run_prepare_project(f.name, env)
                    tf = f.name

                if not ok:
                    log_fail(f"[{idx}/{total_modules}] {mod['moduleName']} — PrepareProject не удался")
                    failures += 1
                    # Ставим прочерк в sizeInfo для данной платформы
                    base = env_to_base_from_module(mod["usedLibs"], env)
                    info = load_json(mod["modinfo_path"])
                    about = info.setdefault("about", {})
                    module_version = about.get("moduleVersion", "unknown")
                    update_size_info(info, module_version, base, "-", "-")
                    save_json(mod["modinfo_path"], info)
                    os.unlink(tf)
                    continue

                # Получаем размеры Flash и RAM
                sizes = get_size_from_output(env)
                os.unlink(tf)
                if sizes is None:
                    log_fail(f"[{idx}/{total_modules}] {mod['moduleName']} — не удалось распарсить размер")
                    failures += 1
                    # Ставим прочерк в sizeInfo для данной платформы
                    base = env_to_base_from_module(mod["usedLibs"], env)
                    info = load_json(mod["modinfo_path"])
                    about = info.setdefault("about", {})
                    module_version = about.get("moduleVersion", "unknown")
                    update_size_info(info, module_version, base, "-", "-")
                    save_json(mod["modinfo_path"], info)
                    continue

                # Вычисляем дельту (вклад модуля)
                delta_flash = sizes["flash_used"] - baseline_sizes[env]["baseline_flash"]
                delta_ram = sizes["ram_used"] - baseline_sizes[env]["baseline_ram"]

                # Дельта не может быть отрицательной
                used_flash = max(0, round(delta_flash))
                used_ram = max(0, round(delta_ram))

                # Определяем ключ платформы
                base = env_to_base_from_module(mod["usedLibs"], env)

                # -------------------------------------------------------------------------
                # ОБНОВЛЕНИЕ modinfo.json
                # -------------------------------------------------------------------------
                # Записываем результаты в sizeInfo, привязанный к версии модуля:
                #   sizeInfo: [
                #     { "moduleVersion": "1.0",
                #       "usedFLASH": { "platform": bytes },
                #       "usedRAM":   { "platform": bytes } }
                #   ]
                # -------------------------------------------------------------------------
                info = load_json(mod["modinfo_path"])
                about = info.setdefault("about", {})
                module_version = about.get("moduleVersion", "unknown")

                update_size_info(info, module_version, base, used_flash, used_ram)

                save_json(mod["modinfo_path"], info)

                if mod["modinfo_path"] not in updated_paths:
                    updated_paths.add(mod["modinfo_path"])
                    updated += 1

                log_ok(
                    f"[{idx}/{total_modules}] {mod['moduleName']} ({base}): "
                    f"+{delta_flash} B Flash, +{delta_ram} B RAM"
                )

        log_section("Итог")
        log_ok(f"Файлов modinfo.json обновлено: {updated}")
        log_ok("platformio.ini восстановлен.")

    except _AbortError:
        # Мягкое прерывание пользователем — состояние восстановит блок finally.
        aborted = True
        log_fail("Прервано пользователем")

    # -------------------------------------------------------------------------
    # ВОССТАНОВЛЕНИЕ СОСТОЯНИЯ (finally)
    # -------------------------------------------------------------------------
    # Этот блок выполняется ВСЕГДА, даже при ошибке:
    #   1. Восстанавливаем platformio.ini из резервной копии.
    #   2. Удаляем резервную копию.
    #   3. Запускаем PrepareProject.py с оригинальным профилем,
    #      чтобы вернуть проект в исходное состояние.
    # -------------------------------------------------------------------------
    finally:
        shutil.copy(platformio_backup, platformio_ini)
        platformio_backup.unlink(missing_ok=True)

        log_section("Восстановление состояния проекта")
        if run_prepare_project(profile_path, original_env):
            log_ok(f"PrepareProject применён: {profile_path.name} (env={original_env})")
        else:
            log_fail(
                f"PrepareProject не удался — восстановите вручную: "
                f"python PrepareProject.py -p {profile_path.name} -b {original_env}"
            )

        # Завершаем логирование
        stop_logging()

    print()

    # Код выхода для внешних раннеров (measure_run.py):
    #   0 — успешно; 2 — завершено, но часть модулей не измерилась;
    #   3 — прервано пользователем (мягкий abort).
    if aborted:
        sys.exit(3)
    if failures:
        sys.exit(2)


if __name__ == "__main__":
    main()

