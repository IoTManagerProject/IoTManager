"""
Управление проектами MagicIoTm
Каждый проект = папка с data.json, about.txt, myProfile.json
Категории = папки верхнего уровня внутри projects/
"""

import json
import os
import re
import shutil
import logging
from datetime import datetime
from threading import Lock

logger = logging.getLogger(__name__)

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PROJECTS_DIR = os.path.join(PROJECT_ROOT, 'projects')
ROOT_CONFIG_FILE = os.path.join(PROJECT_ROOT, '..', '..', 'myProfile.json')
HISTORY_FILE = os.path.join(PROJECTS_DIR, '.history.json')
BACKUP_DIR = os.path.join(PROJECTS_DIR, '.backups')

CONFIG_FILENAME = 'myProfile.json'
PLATFORMIO_INI_FILENAME = 'platformio.ini'
DATA_DIR_NAME = 'data_svelte'
# Корень репозитория IoTManager — рабочий каталог процессов pio/PrepareProject.
# От него считаются относительные пути в platformio.ini (в т.ч. data_dir).
REPO_ROOT = os.path.abspath(os.path.join(PROJECT_ROOT, '..', '..'))

# Защищённый проект PlatformIO (нельзя переименовать, перенести, удалить).
# Его данные берутся напрямую из корневого myProfile.json, а список платформ — из platformio.ini.
PLATFORMIO_PROJECT = 'PlatformIO'


def is_platformio(name):
    """Проверка, является ли имя проектом PlatformIO"""
    return name == PLATFORMIO_PROJECT


def _same_entry(path_a, path_b):
    """True, если оба пути указывают на одну запись файловой системы.

    На Windows ФС нечувствительна к регистру: 'MyProject' и 'myproject' —
    один и тот же каталог, поэтому os.path.exists(new) даёт ложное
    «уже существует» при переименовании сменой регистра.
    """
    try:
        return (os.path.exists(path_a) and os.path.exists(path_b)
                and os.path.samefile(path_a, path_b))
    except OSError:
        return False


# ==================== platformio.ini: ключ data_dir ====================
# data_dir — каталог, из которого PlatformIO собирает образ файловой системы
# (mklittlefs -c $PROJECT_DATA_DIR). Путь относительный — от корня репозитория
# (cwd процесса pio, см. REPO_ROOT), см. PrepareProject.py.

_SECTION_RE = re.compile(r'^\s*\[([^\]]+)\]\s*$')


def _ini_key(line):
    """Имя ключа из строки ini (без значения) в нижнем регистре."""
    return line.split('=', 1)[0].strip().lower()


def _read_ini_lines(ini_path):
    """Читает platformio.ini, сохраняя переводы строк.

    Returns:
        (lines, encoding) или (None, None), если файл не читается.
    """
    for enc in ('utf-8', 'cp1251'):
        try:
            with open(ini_path, 'r', encoding=enc, newline='') as f:
                return f.readlines(), enc
        except UnicodeDecodeError:
            continue
    return None, None


def read_ini_data_dir(ini_path):
    """Возвращает значение data_dir из секции [platformio] (или None)."""
    if not ini_path or not os.path.isfile(ini_path):
        return None
    lines, _enc = _read_ini_lines(ini_path)
    if lines is None:
        return None
    in_section = False
    for line in lines:
        m = _SECTION_RE.match(line)
        if m:
            in_section = m.group(1).strip().lower() == 'platformio'
            continue
        if in_section and _ini_key(line) == 'data_dir':
            parts = line.split('=', 1)
            if len(parts) == 2:
                return parts[1].strip()
    return None


def set_ini_data_dir(ini_path, value):
    """Заменяет (или добавляет) только строку data_dir в секции [platformio].

    В отличие от ConfigParser.write() в PrepareProject.py остальные строки,
    комментарии и форматирование platformio.ini сохраняются без изменений.

    Returns:
        (True, 'OK') или (False, текст ошибки).
    """
    if not ini_path or not os.path.isfile(ini_path):
        return False, f"platformio.ini не найден: {ini_path}"
    lines, enc = _read_ini_lines(ini_path)
    if lines is None:
        return False, f"Не удалось прочитать platformio.ini: {ini_path}"

    header_idx = None
    data_idx = None
    in_section = False
    for idx, line in enumerate(lines):
        m = _SECTION_RE.match(line)
        if m:
            in_section = m.group(1).strip().lower() == 'platformio'
            if in_section and header_idx is None:
                header_idx = idx
            continue
        if in_section and _ini_key(line) == 'data_dir':
            data_idx = idx
            break

    if header_idx is None:
        return False, f"В {ini_path} нет секции [platformio]"

    eol = '\r\n' if any(line.endswith('\r\n') for line in lines) else '\n'
    new_line = f"data_dir = {value}{eol}"
    if data_idx is not None:
        lines[data_idx] = new_line
    else:
        lines.insert(header_idx + 1, new_line)

    try:
        with open(ini_path, 'w', encoding=enc, newline='') as f:
            f.writelines(lines)
    except OSError as e:
        return False, f"Не удалось записать platformio.ini: {e}"
    logger.info(f"Обновлён data_dir в {ini_path}: {value}")
    return True, "OK"


def fix_data_dir(proj_dir, cwd=None):
    """Приводит [platformio] data_dir проекта к <proj_dir>/data_svelte.

    Путь записывается относительным (от корня репозитория — cwd, где запускается
    pio), как это делает PrepareProject.py: без абсолютных путей и кириллических
    полных адресов диска.

    Returns:
        (True, новое значение data_dir) или (False, текст ошибки).
    """
    if not proj_dir or not os.path.isdir(proj_dir):
        return False, f"Каталог проекта не найден: {proj_dir}"
    data_dir = os.path.join(proj_dir, DATA_DIR_NAME)
    if not os.path.isdir(data_dir):
        return False, f"Каталог данных не найден: {data_dir}"
    base = os.path.abspath(cwd) if cwd else REPO_ROOT
    abs_data_dir = os.path.abspath(data_dir)
    try:
        # relpath бросает ValueError, если проект и корень репозитория на разных дисках
        value = os.path.relpath(abs_data_dir, base)
    except ValueError:
        value = abs_data_dir
    value = value.replace(os.sep, '/')
    ok, msg = set_ini_data_dir(os.path.join(proj_dir, PLATFORMIO_INI_FILENAME), value)
    return ok, (value if ok else msg)


def sync_data_dir(proj_dir, cwd=None):
    """Молча синхронизирует data_dir после переноса/переименования проекта.

    Ничего не делает, если в проекте нет platformio.ini или data_svelte.
    """
    ini_path = os.path.join(proj_dir, PLATFORMIO_INI_FILENAME)
    if not os.path.isfile(ini_path) or not os.path.isdir(os.path.join(proj_dir, DATA_DIR_NAME)):
        return
    try:
        ok, value = fix_data_dir(proj_dir, cwd)
        if not ok:
            logger.warning(f"data_dir не обновлён в {ini_path}: {value}")
    except Exception as e:  # noqa: BLE001
        logger.error(f"Ошибка обновления data_dir в {ini_path}: {e}")


def data_dir_report(ini_path, expected_dir, cwd=None):
    """Сверяет каталог данных ФС из platformio.ini с ожидаемым каталогом проекта.

    PlatformIO при `-t buildfs`/`uploadfs` передаёт в mklittlefs значение
    `$PROJECT_DATA_DIR` = `[platformio] data_dir` из platformio.ini
    (espressif32@6.6.0, builder/main.py: `DataToBin(..., "$PROJECT_DATA_DIR")`).
    Относительный путь отсчитывается от корня PIO-проекта — cwd процесса pio
    (magicIoTm запускает `pio run -c <ini>` из корня репозитория, см. REPO_ROOT).
    Если каталог не читается, mklittlefs печатает «can't read source directory»
    и падает с кодом 1, а PIO рапортует «*** [...] littlefs.bin] Error 1».

    Returns:
        dict: ok, reason, ini, ini_value, resolved, expected
              reason: '' | 'no_ini' | 'no_option' | 'missing' | 'mismatch'
    """
    base = os.path.abspath(cwd) if cwd else REPO_ROOT
    ini_path = ini_path or ''
    ini_value = read_ini_data_dir(ini_path)
    resolved = os.path.abspath(os.path.join(base, ini_value)) if ini_value else ''
    expected = os.path.abspath(expected_dir) if expected_dir else ''

    if not ini_path or not os.path.isfile(ini_path):
        reason = 'no_ini'
    elif not ini_value:
        reason = 'no_option'
    elif not os.path.isdir(resolved):
        reason = 'missing'
    elif expected and (os.path.normcase(os.path.normpath(resolved))
                       != os.path.normcase(os.path.normpath(expected))):
        reason = 'mismatch'
    else:
        reason = ''

    return {
        'ok': reason == '',
        'reason': reason,
        'ini': ini_path,
        'ini_value': ini_value or '',
        'resolved': resolved or (ini_value or ''),
        'expected': expected,
    }


def data_dir_error_text(report):
    """Человекочитаемое описание проблемы с data_dir (для UI и логов)."""
    reason = report.get('reason', '')
    if reason == 'no_ini':
        return f"Не найден файл конфигурации проекта: {report.get('ini', '')}"
    if reason == 'no_option':
        return ("В platformio.ini не задан каталог данных ([platformio] data_dir). "
                f"Ожидается: {report.get('expected', '')}")
    if reason == 'missing':
        return ("Каталог данных файловой системы не найден: "
                f"{report.get('resolved', '')}. В platformio.ini указан путь "
                f"'{report.get('ini_value', '')}', ожидается '{report.get('expected', '')}'.")
    if reason == 'mismatch':
        return ("Каталог данных в platformio.ini не совпадает с каталогом проекта: "
                f"'{report.get('ini_value', '')}' (разворачивается в {report.get('resolved', '')}), "
                f"ожидается {report.get('expected', '')}.")
    return ""


_lock = Lock()


def ensure_dirs():
    """Создание служебных директорий"""
    os.makedirs(PROJECTS_DIR, exist_ok=True)
    os.makedirs(BACKUP_DIR, exist_ok=True)


def list_projects():
    """Получение дерева проектов: {category: [project_names]}"""
    tree = {}
    try:
        for entry in sorted(os.listdir(PROJECTS_DIR)):
            full = os.path.join(PROJECTS_DIR, entry)
            if not os.path.isdir(full) or entry.startswith('.'):
                continue
            # Категория (включая пустые)
            projects = []
            for proj in sorted(os.listdir(full)):
                proj_path = os.path.join(full, proj)
                if os.path.isdir(proj_path) and os.path.exists(os.path.join(proj_path, CONFIG_FILENAME)):
                    projects.append(proj)
            tree[entry] = projects
    except Exception as e:
        logger.error(f"Ошибка чтения дерева проектов: {e}")
    return tree


def create_category(name):
    """Создание папки-категории"""
    path = os.path.join(PROJECTS_DIR, name)
    if os.path.exists(path):
        return False, "Категория уже существует"
    os.makedirs(path, exist_ok=True)
    logger.info(f"Создана категория: {name}")
    return True, "OK"


def delete_category(name):
    """Удаление папки-категории с резервной копией"""
    path = os.path.join(PROJECTS_DIR, name)
    if not os.path.exists(path):
        return False, "Категория не найдена"
    # Резервная копия
    _backup(path, name)
    shutil.rmtree(path)
    logger.info(f"Удалена категория: {name}")
    return True, "OK"


def rename_category(old_name, new_name):
    """Переименование категории (папки) с обновлением data.json всех проектов"""
    old_path = os.path.join(PROJECTS_DIR, old_name)
    new_path = os.path.join(PROJECTS_DIR, new_name)
    if not os.path.exists(old_path):
        return False, "Категория не найдена"
    if os.path.exists(new_path) and not _same_entry(old_path, new_path):
        return False, "Категория с таким именем уже существует"
    os.rename(old_path, new_path)
    # Обновляем поле category в data.json каждого проекта
    for proj in sorted(os.listdir(new_path)):
        proj_path = os.path.join(new_path, proj)
        data_path = os.path.join(proj_path, 'data.json')
        if os.path.isdir(proj_path) and os.path.exists(data_path):
            try:
                with open(data_path, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                data['category'] = new_name
                with open(data_path, 'w', encoding='utf-8') as f:
                    json.dump(data, f, ensure_ascii=False, indent=2)
            except Exception as e:
                logger.error(f"Ошибка обновления data.json при переименовании категории: {e}")
        # platformio.ini: data_dir зависит от пути проекта — категория изменилась
        if os.path.isdir(proj_path):
            sync_data_dir(proj_path)
    logger.info(f"Переименована категория: {old_name} -> {new_name}")
    return True, "OK"


def create_project(category, name, description=""):
    """Создание проекта внутри категории"""
    cat_path = os.path.join(PROJECTS_DIR, category)
    if not os.path.exists(cat_path):
        return False, "Категория не найдена"

    proj_path = os.path.join(cat_path, name)
    if os.path.exists(proj_path):
        return False, "Проект уже существует"

    os.makedirs(proj_path, exist_ok=True)

    # Копируем базовый myProfile.json из корня проекта как шаблон
    config = None
    if os.path.exists(ROOT_CONFIG_FILE):
        try:
            with open(ROOT_CONFIG_FILE, 'r', encoding='utf-8') as f:
                config = json.load(f)
            logger.info(f"Создан конфиг из шаблона {ROOT_CONFIG_FILE}")
        except Exception as e:
            logger.error(f"Ошибка чтения шаблона {ROOT_CONFIG_FILE}: {e}")
            config = None

    if config is None:
        # Структура по умолчанию
        config = {
            "iotmSettings": {
                "name": name,
                "apssid": "",
                "appass": "12341234",
                "timezone": 3,
                "ntp": "pool.ntp.org",
                "weblogin": "admin",
                "webpass": "admin",
                "mqttServer": "",
                "mqttPort": 8021,
                "mqttPrefix": "/" + name,
                "mqttUser": "",
                "mqttPass": "",
                "serverip": "http://iotmanager.org",
                "serverlocal": "",
                "log": 0,
                "mqttin": 0,
                "pinSCL": 0,
                "pinSDA": 0,
                "i2cFreq": 100000,
                "wg": "group1"
            },
            "projectProp": {"platformio": {"default_envs": "esp8266_4mb"}},
            "modules": {}
        }

    # Обновляем имя устройства в новом конфиге
    config.setdefault("iotmSettings", {})["name"] = name

    data = {"name": name, "category": category, "created": datetime.now().isoformat()}

    with open(os.path.join(proj_path, CONFIG_FILENAME), 'w', encoding='utf-8') as f:
        json.dump(config, f, ensure_ascii=False, indent=2, sort_keys=False)
    with open(os.path.join(proj_path, 'data.json'), 'w', encoding='utf-8') as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    with open(os.path.join(proj_path, 'about.txt'), 'w', encoding='utf-8') as f:
        f.write(description)

    logger.info(f"Создан проект: {category}/{name}")
    return True, "OK"


def delete_project(category, name):
    """Удаление проекта с резервной копией"""
    if is_platformio(name):
        return False, "Проект PlatformIO нельзя удалить"
    proj_path = os.path.join(PROJECTS_DIR, category, name)
    if not os.path.exists(proj_path):
        return False, "Проект не найден"
    _backup(proj_path, f"{category}_{name}")
    shutil.rmtree(proj_path)
    logger.info(f"Удалён проект: {category}/{name}")
    return True, "OK"


def rename_project(category, old_name, new_name):
    """Переименование проекта"""
    if is_platformio(old_name):
        return False, "Проект PlatformIO нельзя переименовать"
    cat_path = os.path.join(PROJECTS_DIR, category)
    old_path = os.path.join(cat_path, old_name)
    new_path = os.path.join(cat_path, new_name)
    if not os.path.exists(old_path):
        return False, "Проект не найден"
    if os.path.exists(new_path) and not _same_entry(old_path, new_path):
        return False, "Проект с таким именем уже существует"

    os.rename(old_path, new_path)

    # Обновляем data.json
    data_path = os.path.join(new_path, 'data.json')
    if os.path.exists(data_path):
        with open(data_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        data['name'] = new_name
        with open(data_path, 'w', encoding='utf-8') as f:
            json.dump(data, f, ensure_ascii=False, indent=2)

    # platformio.ini: data_dir зависит от пути проекта — имя изменилось
    sync_data_dir(new_path)
    logger.info(f"Переименован проект: {old_name} -> {new_name}")
    return True, "OK"


def copy_project(src_cat, src_name, dst_cat, dst_name):
    """Копирование проекта"""
    src_path = os.path.join(PROJECTS_DIR, src_cat, src_name)
    dst_cat_path = os.path.join(PROJECTS_DIR, dst_cat)
    dst_path = os.path.join(dst_cat_path, dst_name)
    if not os.path.exists(src_path):
        return False, "Исходный проект не найден"
    if os.path.exists(dst_path):
        return False, "Проект назначения уже существует"
    if not os.path.exists(dst_cat_path):
        return False, "Категория назначения не найдена"

    shutil.copytree(src_path, dst_path)

    # Обновляем data.json
    data_path = os.path.join(dst_path, 'data.json')
    if os.path.exists(data_path):
        with open(data_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        data['name'] = dst_name
        data['category'] = dst_cat
        data['created'] = datetime.now().isoformat()
        with open(data_path, 'w', encoding='utf-8') as f:
            json.dump(data, f, ensure_ascii=False, indent=2)

    # platformio.ini: data_dir зависит от пути проекта — путь копии другой
    sync_data_dir(dst_path)
    logger.info(f"Скопирован проект: {src_cat}/{src_name} -> {dst_cat}/{dst_name}")
    return True, "OK"


def move_project(src_cat, src_name, dst_cat, dst_name=None):
    """Перенос проекта в другую категорию (с возможностью переименования)"""
    if is_platformio(src_name):
        return False, "Проект PlatformIO нельзя перенести"
    src_path = os.path.join(PROJECTS_DIR, src_cat, src_name)
    dst_cat_path = os.path.join(PROJECTS_DIR, dst_cat)
    if not os.path.exists(src_path):
        return False, "Проект не найден"
    if not os.path.exists(dst_cat_path):
        return False, "Категория назначения не найдена"

    final_name = dst_name or src_name
    dst_path = os.path.join(dst_cat_path, final_name)

    if os.path.abspath(src_path) == os.path.abspath(dst_path):
        return False, "Проект уже находится в этой категории"
    if os.path.exists(dst_path) and not _same_entry(src_path, dst_path):
        return False, "Проект с таким именем уже существует в категории назначения"

    shutil.move(src_path, dst_path)

    # Обновляем data.json
    data_path = os.path.join(dst_path, 'data.json')
    if os.path.exists(data_path):
        try:
            with open(data_path, 'r', encoding='utf-8') as f:
                data = json.load(f)
            data['name'] = final_name
            data['category'] = dst_cat
            with open(data_path, 'w', encoding='utf-8') as f:
                json.dump(data, f, ensure_ascii=False, indent=2)
        except Exception as e:
            logger.error(f"Ошибка обновления data.json при переносе: {e}")

    # platformio.ini: data_dir зависит от пути проекта — категория/имя изменились
    sync_data_dir(dst_path)
    logger.info(f"Перенесён проект: {src_cat}/{src_name} -> {dst_cat}/{final_name}")
    return True, "OK"


def load_project_config(category, name):
    """Загрузка myProfile.json проекта"""
    path = os.path.join(PROJECTS_DIR, category, name, CONFIG_FILENAME)
    if not os.path.exists(path):
        return None
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def _detect_indent(path, default=2):
    """Определяет ширину отступа в существующем JSON-файле."""
    try:
        with open(path, "r", encoding='utf-8') as f:
            for line in f:
                if line.startswith(' ') and not line.strip().startswith('{'):
                    return len(line) - len(line.lstrip())
    except Exception:  # noqa: BLE001
        pass
    return default


def save_project_config(category, name, config):
    """Сохранение myProfile.json проекта (без изменения порядка полей и отступов).

    Существующий отступ файла сохраняется, порядок полей не изменяется.
    """
    if is_platformio(name):
        # Проект PlatformIO пишет напрямую в корневой myProfile.json
        indent = _detect_indent(ROOT_CONFIG_FILE, 4)
        with open(ROOT_CONFIG_FILE, 'w', encoding='utf-8') as f:
            json.dump(config, f, ensure_ascii=False, indent=indent, sort_keys=False)
        return
    path = os.path.join(PROJECTS_DIR, category, name, CONFIG_FILENAME)
    indent = _detect_indent(path, 2)
    with open(path, 'w', encoding='utf-8') as f:
        json.dump(config, f, ensure_ascii=False, indent=indent, sort_keys=False)


def load_project_about(category, name):
    """Загрузка about.txt"""
    path = os.path.join(PROJECTS_DIR, category, name, 'about.txt')
    if not os.path.exists(path):
        return ""
    with open(path, 'r', encoding='utf-8') as f:
        return f.read()


def get_all_abouts():
    """Сбор описаний (about.txt) всех проектов: {cat/name: text}"""
    result = {}
    tree = list_projects()
    for cat, projs in tree.items():
        for proj in projs:
            text = load_project_about(cat, proj).strip()
            if text:
                result[f"{cat}/{proj}"] = text
    return result


def save_project_about(category, name, text):
    """Сохранение about.txt"""
    if is_platformio(name):
        # У проекта PlatformIO нет about.txt — игнорируем сохранение
        return
    path = os.path.join(PROJECTS_DIR, category, name, 'about.txt')
    with open(path, 'w', encoding='utf-8') as f:
        f.write(text)


# ==================== История ====================

def save_history(category, name):
    """Сохранение последнего открытого проекта"""
    with _lock:
        hist = {"category": category, "name": name, "time": datetime.now().isoformat()}
        with open(HISTORY_FILE, 'w', encoding='utf-8') as f:
            json.dump(hist, f, ensure_ascii=False, indent=2)


def load_history():
    """Загрузка последнего открытого проекта"""
    if not os.path.exists(HISTORY_FILE):
        return None
    try:
        with open(HISTORY_FILE, 'r', encoding='utf-8') as f:
            return json.load(f)
    except Exception:
        return None


# ==================== Валидация ====================

def get_all_field_values(field, exclude_project=None):
    """Сбор значений поля из всех проектов (кроме текущего).

    Возвращает словарь {значение: [список проектов, использующих значение]}.
    """
    values = {}
    # Нормализация исключаемого проекта: PlatformIO приходит с фронтенда как
    # "PlatformIO/PlatformIO" (category/name), а внутри кода ключ — "PlatformIO".
    if exclude_project and (exclude_project == PLATFORMIO_PROJECT
                            or exclude_project.split('/')[-1] == PLATFORMIO_PROJECT):
        exclude_project = PLATFORMIO_PROJECT
    tree = list_projects()
    for cat, projects in tree.items():
        for proj in projects:
            if exclude_project and f"{cat}/{proj}" == exclude_project:
                continue
            config = load_project_config(cat, proj)
            if config:
                val = config.get("iotmSettings", {}).get(field, "")
                if val:
                    values.setdefault(val, []).append(f"{cat}/{proj}")
    # Проект PlatformIO (данные берутся из корневого myProfile.json)
    if exclude_project != PLATFORMIO_PROJECT and os.path.exists(ROOT_CONFIG_FILE):
        try:
            with open(ROOT_CONFIG_FILE, 'r', encoding='utf-8') as f:
                pio_config = json.load(f)
            val = pio_config.get("iotmSettings", {}).get(field, "")
            if val:
                values.setdefault(val, []).append(PLATFORMIO_PROJECT)
        except Exception as e:
            logger.error(f"Ошибка чтения корневого конфига PlatformIO: {e}")
    return values


def validate_name(name, current_project=None):
    """Валидация имени устройства"""
    if not name:
        return False, "Имя не может быть пустым"
    if len(name) > 64:
        return False, "Имя не должно превышать 64 символа"
    if ' ' in name:
        return False, "Имя не должно содержать пробелы"
    # Проверка уникальности (текущий проект не участвует)
    all_names = get_all_field_values("name", exclude_project=current_project)
    owners = all_names.get(name)
    if owners:
        return False, f"Имя уже используется в проектах: {', '.join(owners)}"
    return True, "OK"


def validate_apssid(apssid, current_project=None):
    """Валидация apssid"""
    if not apssid:
        return True, "OK"  # Может быть пустым
    all_ssids = get_all_field_values("apssid", exclude_project=current_project)
    owners = all_ssids.get(apssid)
    if owners:
        return False, f"AP SSID уже используется в проектах: {', '.join(owners)}"
    return True, "OK"


# ==================== Резервное копирование ====================

def _backup(src_path, label):
    """Создание резервной копии перед удалением"""
    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    backup_name = f"{label}_{ts}"
    backup_path = os.path.join(BACKUP_DIR, backup_name)
    try:
        shutil.copytree(src_path, backup_path)
        logger.info(f"Резервная копия: {backup_path}")
    except Exception as e:
        logger.error(f"Ошибка резервного копирования: {e}")
