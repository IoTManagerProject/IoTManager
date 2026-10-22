"""
Конфигурация: платформы, модули, размеры, FS-использование.

Все функции, связанные с загрузкой/сохранением платформ, сканированием модулей,
расчётом размеров FLASH/RAM и FS-использования.
"""

import configparser
import json
import os
import glob
import re
import shutil
import time
import logging

from utils import projects
import state.globals as globals_

# ==================== Логирование ====================
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
logger = logging.getLogger(__name__)

# ==================== Пути ====================
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(BASE_DIR)))
ROOT_CONFIG_FILE = os.path.join(PROJECT_ROOT, 'myProfile.json')
MODULES_SRC_DIR = os.path.join(PROJECT_ROOT, 'src', 'modules')
PLATFORMS_FILE = os.path.join(BASE_DIR, '..', '..', 'measure_size', 'platforms.json')
PLATFORMIO_INI_FILE = os.path.join(PROJECT_ROOT, 'platformio.ini')
MEASURE_SCRIPT = os.path.abspath(os.path.join(BASE_DIR, '..', '..', 'measure_size', 'measure.py'))


def _latest_platform_record(entry):
    """Последняя (самая свежая) запись платформы из platforms.json.

    Платформа хранится как список-история замеров — берётся последний
    элемент. Старый формат (плоский словарь) поддерживается: возвращается
    он сам. Для отсутствующей/битой записи — пустой словарь.
    """
    if isinstance(entry, list) and entry:
        rec = entry[-1]
        return rec if isinstance(rec, dict) else {}
    if isinstance(entry, dict):
        return entry
    return {}


def load_platforms():
    """Загрузка platforms.json с baseline/total значениями.

    Каждая платформа в файле — список-история замеров; в кэш попадает
    последняя (самая свежая) запись. Старый плоский формат тоже поддерживается.

    Кэш подменяется новым словарём только после успешного чтения файла:
    параллельные запросы (/api/size и т.п.) видят либо старые, либо новые
    данные, но не промежуточное пустое состояние — иначе верхние gauge
    FLASH/RAM/FS после замера сбрасываются в 0 (total_flash = 0 → pct = 0).
    """
    try:
        with open(PLATFORMS_FILE, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except Exception as e:
        logger.error(f"platforms.json: {e}")
        return  # при ошибке чтения оставляем предыдущий кэш без изменений
    globals_.platforms_cache = {
        env: _latest_platform_record(entry) for env, entry in data.items()
    }
    logger.info(f"Загружено платформ: {len(data)}")


def save_platform_fs_total(env, fs_total):
    """Сохраняет ёмкость ФС (total_fs) платформы в platforms.json.

    Обновляется последняя запись истории замеров платформы (старый плоский
    формат обновляется как есть). Запись происходит, если поля ещё нет ИЛИ
    текущее значение отличается от переданного размера (например, изменился
    раздел littlefs).
    """
    if not env or not fs_total:
        return
    try:
        with open(PLATFORMS_FILE, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except Exception:
        data = {}
    entry = data.setdefault(env, [])
    if isinstance(entry, dict):
        record = entry  # старый плоский формат (без истории)
    elif isinstance(entry, list) and entry and isinstance(entry[-1], dict):
        record = entry[-1]
    else:
        record = {}
        data[env] = [record]
    current = record.get("total_fs")
    if not current or current != fs_total:
        record["total_fs"] = fs_total
        try:
            with open(PLATFORMS_FILE, 'w', encoding='utf-8') as f:
                json.dump(data, f, ensure_ascii=False, indent=4)
        except Exception as e:
            logger.error(f"Не удалось сохранить total_fs в platforms.json: {e}")
    # Обновляем кэш для немедленного отображения
    if env in globals_.platforms_cache:
        globals_.platforms_cache[env]["total_fs"] = record.get("total_fs") or fs_total


def read_firmware_version():
    """Читает текущую версию прошивки FIRMWARE_VERSION из include/Const.h.

    Похожая функция есть в tools/measure_size/measure.py — там она нужна при
    записи замеров, здесь — для отображения версии в панели. При ошибке
    чтения/парсинга возвращается 'unknown' (панель скрывает такую версию).
    """
    const_path = os.path.join(PROJECT_ROOT, 'include', 'Const.h')
    try:
        with open(const_path, 'r', encoding='utf-8', errors='replace') as f:
            text = f.read()
        m = re.search(
            r'^[ \t]*#[ \t]*define[ \t]+FIRMWARE_VERSION[ \t]+"([^"]*)"',
            text, re.MULTILINE,
        )
        if m:
            return m.group(1)
    except OSError:
        pass
    return "unknown"


def load_platformio_envs(ini_path=None):
    """Получение списка платформ из platformio.ini (секции [env:*]).

    Исключаются вспомогательные секции вида *_fromitems.
    Возвращает список имён платформ в порядке появления в файле.
    """
    ini_path = ini_path or PLATFORMIO_INI_FILE
    envs = []
    try:
        with open(ini_path, 'r', encoding='utf-8') as f:
            for line in f:
                m = re.match(r'^\s*\[env:([^\]]+)\]\s*$', line)
                if m:
                    name = m.group(1).strip()
                    # Пропускаем вспомогательные секции-шаблоны от PreBuild
                    if name.endswith('_fromitems'):
                        continue
                    if name not in envs:
                        envs.append(name)
        logger.info(f"platformio.ini: загружено платформ: {len(envs)}")
    except Exception as e:
        logger.error(f"platformio.ini: {e}")
    return envs


def get_project_platformio_path(proj=None):
    """Путь к platformio.ini текущего проекта (из папки проекта).

    Если проект не открыт, является проектом PlatformIO или у проекта нет
    собственного platformio.ini — используется корневой platformio.ini.
    """
    if not proj:
        return PLATFORMIO_INI_FILE
    if projects.is_platformio(proj.get("name", "")):
        return PLATFORMIO_INI_FILE
    path = os.path.join(projects.PROJECTS_DIR, proj.get("category", ""), proj.get("name", ""), "platformio.ini")
    if os.path.exists(path):
        return path
    return PLATFORMIO_INI_FILE


def get_platformio_platforms():
    """Список платформ из platformio.ini текущего проекта (или корневого)"""
    ini = get_project_platformio_path(globals_.current_project)
    return load_platformio_envs(ini)


def scan_modinfo():
    """Сканирование всех modinfo.json.

    Заполняется локальный словарь, затем кэш подменяется целиком: запросы,
    пришедшие во время сканирования (оно может идти сотни миллисекунд),
    видят либо старый, либо новый кэш, но не пустой.
    """
    new_cache = {}
    pattern = os.path.join(MODULES_SRC_DIR, '**', 'modinfo.json')
    files = glob.glob(pattern, recursive=True)
    for fp in files:
        try:
            with open(fp, 'r', encoding='utf-8') as f:
                info = json.load(f)
            name = os.path.basename(os.path.dirname(fp))
            about = info.get("about", {})
            # sizeInfo — массив, берём первый элемент
            size_info_list = info.get("sizeInfo", [])
            used_flash = {}
            used_ram = {}
            if size_info_list and isinstance(size_info_list, list):
                si = size_info_list[0]
                used_flash = si.get("usedFLASH", {})
                used_ram = si.get("usedRAM", {})
            new_cache[name] = {
                "usedFLASH": used_flash,
                "usedRAM": used_ram,
                "usedLibs": info.get("usedLibs", {}),
                "about": about,
            }
        except Exception as e:
            logger.error(f"modinfo {fp}: {e}")
    globals_.modinfo_cache = new_cache
    logger.info(f"Кэш modinfo: {len(new_cache)} модулей")


def is_compatible(platform, used_libs):
    if not used_libs:
        return True
    # список платформ — неперечисленные считаем совместимыми
    if isinstance(used_libs, list):
        return True
    if not isinstance(used_libs, dict):
        return True
    # структура вида {"exclude": [...], "platforms": [...]} и т.п.
    if "exclude" in used_libs or "platforms" in used_libs:
        excluded = set(used_libs.get("exclude") or [])
        included = set(used_libs.get("platforms") or [])
        if platform in excluded:
            return False
        if included and platform not in included:
            return False
        return True
    # обычный словарь: платформа → список библиотек (["exclude"] = запрет)
    if platform in used_libs:
        return used_libs[platform] != ["exclude"]
    for pattern, libs in used_libs.items():
        if pattern.endswith("*") and platform.startswith(pattern[:-1]):
            return libs != ["exclude"]
    # платформа не упомянута явно — считается НЕСОВМЕСТИМОЙ
    return False


def _lookup_platform_value(data, platform):
    """Поиск значения только по точной платформе, без фолбэков; '-' → 0"""
    if not data or not isinstance(data, dict):
        return 0
    v = data.get(platform)
    if v is None or v == "-":
        return 0
    return int(v) if isinstance(v, (int, float, str)) else 0


def _is_numeric_size(v):
    """True, если значение размера — число (0 — тоже измеренный размер)."""
    if isinstance(v, (int, float)):
        return True
    if isinstance(v, str):
        try:
            float(v)
            return True
        except (TypeError, ValueError):
            return False
    return False


def _module_size_state(used_flash, platform):
    """Статус размера модуля для платформы (по sizeInfo.usedFLASH).

    Возвращает:
      'ok'             — числовой размер для платформы есть;
      'error_platform' — значение "-" для текущей платформы, но для других
                         платформ есть размеры (ошибка компиляции именно
                         для этой платформы);
      'error_all'      — значение "-" для всех платформ (модуль не компилируется
                         ни для одной из замерявшихся платформ);
      'missing'        — нет записи для текущей платформы (размер не замерялся).
    """
    if not used_flash or not isinstance(used_flash, dict):
        return 'missing'
    val = used_flash.get(platform)
    if val is not None and _is_numeric_size(val):
        return 'ok'
    if val == "-":
        # Есть ли числовые размеры для других платформ
        for p, v in used_flash.items():
            if p != platform and _is_numeric_size(v):
                return 'error_platform'
        return 'error_all'
    return 'missing'


def get_module_flash(name, platform):
    """Размер FLASH модуля для платформы"""
    info = globals_.modinfo_cache.get(name, {})
    return _lookup_platform_value(info.get("usedFLASH", {}), platform)


def get_module_ram(name, platform):
    """Размер RAM модуля для платформы"""
    info = globals_.modinfo_cache.get(name, {})
    return _lookup_platform_value(info.get("usedRAM", {}), platform)


def get_platform_limits(platform):
    """Возвращает (baseline_flash, total_flash, baseline_ram, total_ram) из platforms.json"""
    p = globals_.platforms_cache.get(platform, {})
    return (
        p.get("baseline_flash", 0),
        p.get("total_flash", 0),
        p.get("baseline_ram", 0),
        p.get("total_ram", 0),
    )


def calc_size():
    """Расчёт заполнения FLASH и RAM: baseline + сумма активных модулей"""
    if not globals_.current_config:
        return 0, 0, 0, 0, 0, 0
    flash_total = 0
    ram_total = 0
    for mods in globals_.current_config.get("modules", {}).values():
        if not isinstance(mods, list):
            continue
        for m in mods:
            if m.get("active"):
                name = m.get("path", "").split("/")[-1]
                flash_total += get_module_flash(name, globals_.current_platform)
                ram_total += get_module_ram(name, globals_.current_platform)
    bf, tf, br, tr = get_platform_limits(globals_.current_platform)
    flash_used = bf + flash_total
    ram_used = br + ram_total
    flash_pct = round(flash_used / tf * 100, 1) if tf > 0 else 0
    ram_pct = round(ram_used / tr * 100, 1) if tr > 0 else 0
    return flash_pct, flash_used, tf, ram_pct, ram_used, tr


def _project_dir(proj):
    """Каталог проекта (там, где лежат myProfile.json/platformio.ini)."""
    if projects.is_platformio(proj.get("name", "")):
        return PROJECT_ROOT
    return os.path.join(projects.PROJECTS_DIR, proj.get("category", ""), proj.get("name", ""))


def _find_data_svelte(proj_dir):
    """Путь к каталогу data_svelte внутри проекта (корень или iotm/<платформа>).

    Возвращает путь к папке data_svelte или None, если её нет.
    """
    root = os.path.join(proj_dir, "data_svelte")
    if os.path.isdir(root):
        return root
    iotm = os.path.join(proj_dir, "iotm")
    if os.path.isdir(iotm):
        for p in sorted(os.listdir(iotm)):
            cand = os.path.join(iotm, p, "data_svelte")
            if os.path.isdir(cand):
                return cand
    return None


def _dir_size(path):
    """Суммарный размер файлов в каталоге (байты)."""
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


# Имена собранного образа файловой системы.
# littlefs.bin/spiffs.bin — обычный таргет buildfs, кладёт образ в .pio/build/<env>/.
# lt_littlefs.bin — LibreTiny (bk7231n): образ собирает кастомный таргет
# tools/lt_fsbuild.py и кладёт его в корень проекта (PROJECT_DIR).
FS_IMAGE_NAMES = ("littlefs.bin", "spiffs.bin")
LT_FS_IMAGE_NAME = "lt_littlefs.bin"
LT_FS_SCRIPT = "lt_fsbuild.py"     # признак env с LibreTiny-ФС в platformio.ini


def _is_lt_env(env):
    """True, если env собирает ФС кастомным таргетом LibreTiny (tools/lt_fsbuild.py).

    У таких платформ образ ФС — lt_littlefs.bin в корне проекта, а не
    littlefs.bin/spiffs.bin в .pio/build/<env>/. Признак берём из platformio.ini
    (секция [env:<env>], поле extra_scripts), чтобы не искать образ чужой платформы.
    """
    if not env:
        return False
    try:
        cp = configparser.ConfigParser(strict=False, interpolation=None)
        cp.read(PLATFORMIO_INI_FILE, encoding="utf-8")
        scripts = cp.get(f"env:{env}", "extra_scripts", fallback="")
    except Exception:  # noqa: BLE001 — при любой проблеме считаем env обычной ESP
        return False
    return LT_FS_SCRIPT in scripts


def _fs_image_size():
    """Ёмкость раздела ФС по свежесобранному образу (0, если образа нет).

    Для обычных ESP ищем littlefs.bin/spiffs.bin в .pio/build/<current_platform>/;
    для LibreTiny (bk7231n) — lt_littlefs.bin в корне проекта. Размер образа равен
    ёмкости раздела ФС.
    """
    env = globals_.current_platform
    names = list(FS_IMAGE_NAMES)
    bases = [os.path.join(PROJECT_ROOT, ".pio", "build", env)]
    if _is_lt_env(env):
        names.append(LT_FS_IMAGE_NAME)
        bases.append(PROJECT_ROOT)
    if globals_.current_project:
        proj_dir = _project_dir(globals_.current_project)
        bases.append(os.path.join(proj_dir, ".pio", "build", env))
        if LT_FS_IMAGE_NAME in names and os.path.abspath(proj_dir) != os.path.abspath(PROJECT_ROOT):
            bases.append(proj_dir)
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


def get_fs_usage():
    """Возвращает (fs_pct, fs_used, fs_total) для текущей платформы.

    fs_total — ёмкость раздела ФС (total_fs из platforms.json; если его ещё нет —
               размер свежесобранного образа ФС: .pio/build/<платформа>/littlefs.bin
               либо lt_littlefs.bin в корне проекта — так собирает ФС для bk7231n).
    fs_used  — занятое: размер папки data_svelte проекта (в корне проекта).
    """
    fs_total = globals_.platforms_cache.get(globals_.current_platform, {}).get("total_fs", 0)
    if not fs_total:
        fs_total = _fs_image_size()
    fs_used = 0
    if globals_.current_project:
        # data_svelte всегда лежит в корне проекта
        data_dir = os.path.join(_project_dir(globals_.current_project), "data_svelte")
        fs_used = _dir_size(data_dir)
    fs_pct = round(fs_used / fs_total * 100, 1) if fs_total > 0 else 0
    return fs_pct, fs_used, fs_total


def get_compat_map():
    if not globals_.current_config:
        return {}
    result = {}
    for section, mods in globals_.current_config.get("modules", {}).items():
        if not isinstance(mods, list):
            continue
        for m in mods:
            name = m.get("path", "").split("/")[-1]
            info = globals_.modinfo_cache.get(name, {})
            result[m.get("path", "")] = {
                "compatible": is_compatible(globals_.current_platform, info.get("usedLibs", {})),
                "size": get_module_flash(name, globals_.current_platform),
                "ram": get_module_ram(name, globals_.current_platform),
                # Статус размера: ok / error_platform / error_all / missing
                "sizeState": _module_size_state(info.get("usedFLASH", {}), globals_.current_platform),
            }
    return result
