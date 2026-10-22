"""
Сборка прошивки выбранного проекта в фоновом потоке.

Модуль управляет состоянием сборки (шаги, лог, результат), потоковым запуском
subprocess и отдачей событий по SSE. Модуль самодостаточен: все пути и параметры
передаются функцией start(cfg) из app.py, поэтому циклических импортов нет.

Последовательность шагов (зафиксирована ТЗ):
  1. PrepareProject.py -p <myProfile.json>
  2. pio run -c <platformio.ini> -t buildfs -e <default_envs>
  3. pio run -c <platformio.ini> -e <default_envs>
  4. (успех) расчёт размеров Flash / RAM / FS
"""

import configparser
import os
import re
import sys
import json
import shutil
import threading
import subprocess

# Фраза, по которой определяем успешное завершение PrepareProject.py
PREPARE_SUCCESS = "you can run compilation and firmware."

# Шаги сборки (id должны быть стабильными)
STEPS = [
    {"id": 1, "label": "Подготовка профиля (PrepareProject)"},
    {"id": 2, "label": "Сборка файловой системы (buildfs)"},
    {"id": 3, "label": "Сборка прошивки (build)"},
]

_ANSI_RE = re.compile(r"\x1b\[[0-9;]*[A-Za-z]")

# Регулярки для парсинга размера из вывода pio
_FLASH_RE = re.compile(r"Flash:\s*\[[^\]]*\]\s*([\d.]+)%\s*\(used\s+(\d+)\s+bytes from\s+(\d+)\s+bytes\)")
_RAM_RE = re.compile(r"RAM:\s*\[[^\]]*\]\s*([\d.]+)%\s*\(used\s+(\d+)\s+bytes from\s+(\d+)\s+bytes\)")


# ==================== Состояние сборки ====================

_state = {
    "running": False,
    "success": None,          # None — не завершено, True/False — результат
    "steps": [],              # копии STEPS со статусом status
    "lines": [],              # накопленный лог (без ANSI)
    "sizes": None,
    "error_step": None,
    "error": None,
    "project_label": "",
    "cond": threading.Condition(),
}

_lock = threading.Lock()


def _reset_state(project_label=""):
    steps = []
    for s in STEPS:
        steps.append({"id": s["id"], "label": s["label"], "status": "pending"})
    _state.update({
        "running": False,
        "success": None,
        "steps": steps,
        "lines": [],
        "sizes": None,
        "error_step": None,
        "error": None,
        "project_label": project_label,
    })


def _notify():
    with _state["cond"]:
        _state["cond"].notify_all()


def _append_line(text):
    text = _ANSI_RE.sub("", text).rstrip("\r\n")
    with _state["cond"]:
        _state["lines"].append(text)
        _state["cond"].notify_all()


def _set_step(step_id, status):
    with _state["cond"]:
        for s in _state["steps"]:
            if s["id"] == step_id:
                s["status"] = status
        _state["cond"].notify_all()


def _set_running(flag):
    with _state["cond"]:
        _state["running"] = flag
        _state["cond"].notify_all()


# ==================== Публичный API ====================

def is_running():
    with _state["cond"]:
        return _state["running"]


def get_status():
    """Снимок состояния для отображения (может использоваться при опросе)."""
    with _state["cond"]:
        return {
            "running": _state["running"],
            "success": _state["success"],
            "steps": list(_state["steps"]),
            "sizes": _state["sizes"],
            "error_step": _state["error_step"],
            "error": _state["error"],
            "project_label": _state["project_label"],
        }


def start(cfg):
    """Запуск сборки в фоне. cfg — dict с параметрами (см. app.py)."""
    with _lock:
        if _state["running"]:
            return False
        _reset_state(cfg.get("project_label", ""))
        _set_running(True)
    t = threading.Thread(target=_worker, args=(cfg,), daemon=True)
    t.start()
    return True


def event_stream():
    """Генератор SSE-событий сборки: log, step, done/error, finish."""
    idx = 0
    emitted_step = {}   # step_id -> уже отправленный status
    done_sent = False
    while True:
        with _state["cond"]:
            # Новые строки лога
            while idx < len(_state["lines"]):
                yield _sse("log", {"text": _state["lines"][idx]})
                idx += 1
            # Изменения статусов шагов
            for s in _state["steps"]:
                status = s["status"]
                if status != "pending" and emitted_step.get(s["id"]) != status:
                    emitted_step[s["id"]] = status
                    yield _sse("step", {"id": s["id"], "label": s["label"], "status": status})
            finished = not _state["running"]
            if finished and not done_sent:
                done_sent = True
                if _state["success"]:
                    yield _sse("done", {"sizes": _state["sizes"]})
                else:
                    yield _sse("error", {
                        "step": _state["error_step"],
                        "error": _state["error"],
                        "label": _step_label(_state["error_step"]),
                    })
                yield _sse("finish", {})
            if finished:
                break
            _state["cond"].wait(timeout=1.0)


def _step_label(step_id):
    for s in STEPS:
        if s["id"] == step_id:
            return s["label"]
    return "неизвестный шаг"


def _sse(event, data):
    return "event: {}\ndata: {}\n\n".format(event, json.dumps(data, ensure_ascii=False))


# ==================== Воркер ====================

def _worker(cfg):
    try:
        _append_line("Сборка проекта: " + cfg.get("project_label", ""))

        # ---- Шаг 1: PrepareProject ----
        _set_step(1, "running")
        _append_line("")
        _append_line("=== Шаг 1. " + _step_label(1) + " ===")
        cmd1 = [sys.executable, cfg["prepare"], "-p", cfg["profile"]]
        _append_line("> " + " ".join(cmd1))
        rc1, out1 = _run_streaming(cmd1, cfg["cwd"])
        if rc1 != 0 or PREPARE_SUCCESS not in out1:
            msg = "PrepareProject завершился с ошибкой (код {})".format(rc1)
            if PREPARE_SUCCESS not in out1:
                msg = "Подготовка профиля не дала подтверждения успеха ('{}')".format(PREPARE_SUCCESS)
            _set_step(1, "error")
            _fail(1, msg)
            return
        _set_step(1, "done")

        # ---- Шаг 2: buildfs ----
        _set_step(2, "running")
        _append_line("")
        _append_line("=== Шаг 2. " + _step_label(2) + " ===")
        cmd2 = [cfg["pio"], "run", "-c", cfg["ini"], "-t", "buildfs", "-e", cfg["env"]]
        _append_line("> " + " ".join(cmd2))
        rc2, _ = _run_streaming(cmd2, cfg["cwd"])
        if rc2 != 0:
            _set_step(2, "error")
            _fail(2, "Ошибка сборки файловой системы (код {})".format(rc2))
            return
        _set_step(2, "done")

        # ---- Шаг 3: build (прошивка) ----
        _set_step(3, "running")
        _append_line("")
        _append_line("=== Шаг 3. " + _step_label(3) + " ===")
        cmd3 = [cfg["pio"], "run", "-c", cfg["ini"], "-e", cfg["env"]]
        _append_line("> " + " ".join(cmd3))
        rc3, out3 = _run_streaming(cmd3, cfg["cwd"])
        if rc3 != 0:
            _set_step(3, "error")
            _fail(3, "Ошибка сборки прошивки (код {})".format(rc3))
            return
        _set_step(3, "done")

        # ---- Копирование файлов прошивки в iotm/<платформа>/400/ ----
        _copy_firmware(cfg)

        # ---- Очистка iotm/ — оставляем только одну папку с прошивками ----
        _prune_iotm(cfg)

        # ---- Шаг 4: успех + размеры ----
        sizes = _compute_sizes(out3, cfg)
        _append_line("")
        _append_line("=== Успех! Прошивка собрана ===")
        _finish_success(sizes)
    except Exception as e:  # noqa: BLE001
        _append_line("[build] Внутренняя ошибка: {}".format(e))
        _fail(None, str(e))


def _run_streaming(cmd, cwd):
    """Запуск команды с построчным выводом. Возвращает (returncode, полный текст)."""
    full = []
    # Принудительно UTF-8 у дочерних процессов (PrepareProject/pio), чтобы кириллица
    # не искажалась при декодировании pipe (по аналогии с measure_run).
    env = os.environ.copy()
    env["PYTHONIOENCODING"] = "utf-8"
    env["PYTHONUTF8"] = "1"
    try:
        p = subprocess.Popen(
            cmd,
            cwd=cwd,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",
            errors="replace",
            bufsize=1,
        )
        for raw in iter(p.stdout.readline, ""):
            line = _ANSI_RE.sub("", raw).rstrip("\r\n")
            full.append(line)
            _append_line(line)
        p.stdout.close()
        rc = p.wait()
    except FileNotFoundError as e:
        _append_line("[build] Не удалось запустить команду: {}".format(e))
        return 127, "\n".join(full)
    except Exception as e:  # noqa: BLE001
        _append_line("[build] Ошибка выполнения: {}".format(e))
        return 1, "\n".join(full)
    return rc, "\n".join(full)


def _fail(error_step, message):
    with _state["cond"]:
        _state["success"] = False
        _state["error_step"] = error_step
        _state["error"] = message
        _state["running"] = False
        _state["cond"].notify_all()


def _finish_success(sizes):
    with _state["cond"]:
        _state["success"] = True
        _state["sizes"] = sizes
        _state["running"] = False
        _state["cond"].notify_all()


# ==================== Размеры ====================

def _compute_sizes(build_out, cfg):
    sizes = {"flash_used": 0, "flash_total": 0, "flash_pct": 0,
             "ram_used": 0, "ram_total": 0, "ram_pct": 0,
             "fs_used": 0, "fs_total": 0, "fs_pct": 0}

    flash_matches = _FLASH_RE.findall(build_out)
    if flash_matches:
        _, used, total = flash_matches[-1]
        used, total = int(used), int(total)
        sizes["flash_used"], sizes["flash_total"] = used, total
        sizes["flash_pct"] = round(used / total * 100, 1) if total > 0 else 0

    ram_matches = _RAM_RE.findall(build_out)
    if ram_matches:
        _, used, total = ram_matches[-1]
        used, total = int(used), int(total)
        sizes["ram_used"], sizes["ram_total"] = used, total
        sizes["ram_pct"] = round(used / total * 100, 1) if total > 0 else 0

    fs_used = _dir_size(cfg.get("data_dir", ""))
    fs_total = _fs_total(cfg)
    sizes["fs_used"], sizes["fs_total"] = fs_used, fs_total
    sizes["fs_pct"] = round(fs_used / fs_total * 100, 1) if fs_total > 0 else 0
    return sizes


def _dir_size(path):
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


# Имена собранного образа ФС.
# littlefs.bin/spiffs.bin — обычный таргет buildfs, образ в .pio/build/<env>/;
# lt_littlefs.bin — LibreTiny (bk7231n), образ кладёт кастомный таргет
# tools/lt_fsbuild.py в корень проекта (PROJECT_DIR).
FS_IMAGE_NAMES = ("littlefs.bin", "spiffs.bin")
LT_FS_IMAGE_NAME = "lt_littlefs.bin"
LT_FS_SCRIPT = "lt_fsbuild.py"     # признак env с LibreTiny-ФС в platformio.ini

# Файлы прошивки, копируемые после сборки в iotm/<платформа>/<подкаталог>/
FIRMWARE_FILES = ["firmware.bin", "littlefs.bin", "partitions.bin"]
FIRMWARE_DEST_SUBDIR = "400"


def copy_firmware(cfg):
    """Публичный вызов копирования артефактов сборки в iotm/<платформа>/400/.

    Используется как сборкой (после шага 3), так и прошивкой по USB
    (utils/flash.py), чтобы готовые firmware.bin/littlefs.bin лежали в папке
    проекта и после прошивки.
    """
    _copy_firmware(cfg)


def _copy_firmware(cfg):
    """Копирует собранные файлы прошивки в iotm/<платформа>/400/ проекта.

    Источник — каталог сборки .pio/build/<платформа> в корне PlatformIO-проекта
    (e:/GitHub/IoTManager/.pio/build/<платформа>).

    Целевая папка iotm создаётся в корне самого собираемого проекта (каталог,
    где лежит myProfile.json/platformio.ini): например, для проекта PlatformIO —
    это e:/GitHub/IoTManager, а для Test/ESP8266 — e:/GitHub/IoTManager/tools/
    magicIoTm/projects/Test/ESP8266.

    Каждый файл копируется только если он реально существует в каталоге сборки
    (например, у ESP8266 нет partitions.bin, а littlefs.bin создаётся шагом buildfs).
    """
    env = cfg.get("env", "")
    # Источник — .pio\build\<платформа> в корне PlatformIO-проекта
    build_base = cfg.get("cwd", "")
    # Цель — iotm\<платформа>\400\ в корне собираемого проекта
    proj_base = os.path.dirname(cfg.get("profile", "")) if cfg.get("profile") else build_base

    src_dir = os.path.join(build_base, ".pio", "build", env)
    dst_dir = os.path.join(proj_base, "iotm", env, FIRMWARE_DEST_SUBDIR)

    if not os.path.isdir(src_dir):
        _append_line(f"[copy] Каталог сборки не найден, пропускаем: {src_dir}")
        return

    os.makedirs(dst_dir, exist_ok=True)
    copied = False
    for fname in FIRMWARE_FILES:
        src_path = os.path.join(src_dir, fname)
        dst_path = os.path.join(dst_dir, fname)
        if os.path.isfile(src_path):
            try:
                shutil.copy2(src_path, dst_path)
                copied = True
                _append_line(f"[copy] Скопировано: {src_path} -> {dst_path}")
            except OSError as e:
                _append_line(f"[copy] Ошибка копирования {fname}: {e}")
        else:
            _append_line(f"[copy] Файл не найден, пропускаем: {src_path}")

    if not copied:
        _append_line(f"[copy] Не найдено ни одного файла прошивки в {src_dir}")


def _prune_iotm(cfg):
    """После успешной сборки оставляет в папке iotm/ проекта только одну папку
    с прошивками — текущей платформы. Остальные папки удаляются.

    Выполняется для всех проектов, кроме PlatformIO (у него iotm/ не используется).

    data_svelte теперь всегда лежит в корне проекта (<проект>/data_svelte),
    поэтому из папки прошивки также удаляется устаревшая внутренняя data_svelte.
    """
    build_base = cfg.get("cwd", "")
    proj_base = os.path.dirname(cfg.get("profile", "")) if cfg.get("profile") else build_base
    env = cfg.get("env", "")

    # Проект PlatformIO — очистка iotm/ не требуется.
    if os.path.abspath(proj_base) == os.path.abspath(build_base):
        return

    iotm_dir = os.path.join(proj_base, "iotm")
    if not os.path.isdir(iotm_dir):
        return

    for entry in sorted(os.listdir(iotm_dir)):
        full = os.path.join(iotm_dir, entry)
        if not os.path.isdir(full):
            continue
        if entry != env:
            try:
                shutil.rmtree(full)
                _append_line(f"[iotm] Удалена лишняя папка с прошивками: {full}")
            except OSError as e:
                _append_line(f"[iotm] Не удалось удалить {full}: {e}")
        else:
            # В папке текущей платформы оставляем только прошивку.
            old_data = os.path.join(full, "data_svelte")
            if os.path.isdir(old_data):
                try:
                    shutil.rmtree(old_data)
                    _append_line(f"[iotm] Удалена устаревшая data_svelte из папки прошивки: {old_data}")
                except OSError as e:
                    _append_line(f"[iotm] Не удалось удалить {old_data}: {e}")


def _is_lt_env(cfg, env):
    """True, если env собирает ФС кастомным таргетом LibreTiny (tools/lt_fsbuild.py).

    У таких платформ образ ФС — lt_littlefs.bin в корне проекта, а не
    littlefs.bin/spiffs.bin в .pio/build/<env>/. Признак берём из platformio.ini
    (секция [env:<env>], поле extra_scripts), чтобы не искать образ чужой платформы.
    """
    if not env:
        return False
    try:
        cp = configparser.ConfigParser(strict=False, interpolation=None)
        cp.read(cfg.get("ini", ""), encoding="utf-8")
        scripts = cp.get(f"env:{env}", "extra_scripts", fallback="")
    except Exception:  # noqa: BLE001 — при любой проблеме считаем env обычной ESP
        return False
    return LT_FS_SCRIPT in scripts


def _fs_total(cfg):
    """Доступный объём FS — размер собранного образа ФС.

    Где искать образ:
      * .pio/build/<env>/littlefs.bin — ESP (esp8266/esp32), обычный таргет buildfs;
      * .pio/build/<env>/spiffs.bin  — если FS на ESP-проекте на базе spiffs;
      * lt_littlefs.bin               — LibreTiny (bk7231n): образ собирает кастомный
                                        таргет tools/lt_fsbuild.py, причём кладёт его
                                        в корень проекта (PROJECT_DIR), а не в .pio/build.

    Размер образа равен ёмкости раздела ФС (mklittlefs/mklittlefs.exe дополняет образ
    до размера раздела), поэтому размер файла и есть «total_fs».
    """
    env = cfg.get("env", "")
    names = list(FS_IMAGE_NAMES)
    proj_dir = os.path.dirname(cfg.get("profile", "")) if cfg.get("profile") else cfg.get("cwd", "")
    bases = [os.path.join(cfg.get("cwd", ""), ".pio", "build", env)]
    if _is_lt_env(cfg, env):
        names.append(LT_FS_IMAGE_NAME)
        bases.append(proj_dir)
        if os.path.abspath(proj_dir) != os.path.abspath(cfg.get("cwd", "")):
            bases.append(cfg.get("cwd", ""))
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