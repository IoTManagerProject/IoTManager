"""
flash — прошивка выбранного проекта на подключённое устройство по USB.

Модуль самодостаточен: все пути и параметры передаются функцией start(cfg)
из app.py (аналогично build.py), циклических импортов нет.

Режимы прошивки (mode):
  'firmware' — записать только прошивку:        pio run -t upload
  'fs'       — записать только файловую систему: pio run -t uploadfs
  'full'     — полная прошивка:                  uploadfs, затем upload

Прошивка выполняется средствами PlatformIO (`pio upload`/`uploadfs`),
который знает правильные смещения памяти под конкретную плату и внутри
использует esptool. Порт задаётся через `--upload-port <port>`.

Прогресс отдаётся по SSE через event_stream() (паттерн как в build.py).
"""

import json
import os
import re
import subprocess
import sys
import threading

from utils import build, projects

# Режимы прошивки: id -> (label, порядок pio-шагов)
# Каждый шаг: {"target": pio-target для запуска, "label": человекочитаемая метка}
MODES = {
    "fs": {"label": "Только файловая система", "steps": [
        {"target": "uploadfs", "label": "Загрузка файловой системы FS"},
    ]},
    "firmware": {"label": "Только прошивка", "steps": [
        {"target": "upload", "label": "Загрузка прошивки"},
    ]},
    "full": {"label": "Полная прошивка", "steps": [
        {"target": "uploadfs", "label": "Загрузка файловой системы FS"},
        {"target": "upload", "label": "Загрузка прошивки"},
    ]},
}

_ANSI_RE = re.compile(r"\x1b\[[0-9;]*[A-Za-z]")


# ==================== Состояние ====================

_state = {
    "running": False,
    "success": None,
    "mode": None,
    "steps": [],
    "lines": [],
    "error_step": None,
    "error": None,
    "project_label": "",
    "port": None,
    "cond": threading.Condition(),
}

_lock = threading.Lock()


def _notify():
    with _state["cond"]:
        _state["cond"].notify_all()


def _append_line(text):
    text = _ANSI_RE.sub("", text).rstrip("\r\n")
    with _state["cond"]:
        _state["lines"].append(text)
        _state["cond"].notify_all()


def _set_running(flag):
    with _state["cond"]:
        _state["running"] = flag
        _state["cond"].notify_all()


def _reset_state(project_label="", mode="", port=""):
    steps = []
    for i, step in enumerate(MODES[mode]["steps"], start=1):
        steps.append({"id": i, "label": step["label"], "status": "pending"})
    with _state["cond"]:
        _state.update({
            "running": False,
            "success": None,
            "mode": mode,
            "steps": steps,
            "lines": [],
            "error_step": None,
            "error": None,
            "project_label": project_label,
            "port": port,
        })
        _state["cond"].notify_all()


def _fail(error_step, message):
    with _state["cond"]:
        _state["success"] = False
        _state["error_step"] = error_step
        _state["error"] = message
        _state["running"] = False
        _state["cond"].notify_all()


def _finish_success():
    with _state["cond"]:
        _state["success"] = True
        _state["running"] = False
        _state["cond"].notify_all()


# ==================== Публичный API ====================

def is_running():
    with _state["cond"]:
        return _state["running"]


def get_status():
    """Снимок состояния прошивки для UI."""
    with _state["cond"]:
        return {
            "running": _state["running"],
            "success": _state["success"],
            "mode": _state["mode"],
            "steps": list(_state["steps"]),
            "error_step": _state["error_step"],
            "error": _state["error"],
            "project_label": _state["project_label"],
            "port": _state["port"],
        }


def start(cfg):
    """Запуск прошивки в фоне. cfg — dict с параметрами (см. app.py).

    Returns:
        bool: True если запущено, False если уже выполняется.
    """
    with _lock:
        if _state["running"]:
            return False
        mode = cfg.get("mode", "firmware")
        _reset_state(cfg.get("project_label", ""), mode, cfg.get("upload_port", ""))
        _set_running(True)
    t = threading.Thread(target=_worker, args=(cfg,), daemon=True)
    t.start()
    return True


def event_stream():
    """Генератор SSE-событий прошивки: log, step, done/error, finish."""
    idx = 0
    emitted_step = {}
    done_sent = False
    while True:
        with _state["cond"]:
            while idx < len(_state["lines"]):
                yield _sse("log", {"text": _state["lines"][idx]})
                idx += 1
            for s in _state["steps"]:
                status = s["status"]
                if status != "pending" and emitted_step.get(s["id"]) != status:
                    emitted_step[s["id"]] = status
                    yield _sse("step", {"id": s["id"], "label": s["label"], "status": status})
            finished = not _state["running"]
            if finished and not done_sent:
                done_sent = True
                if _state["success"]:
                    yield _sse("done", {})
                else:
                    yield _sse("error", {"step": _state["error_step"],
                                         "error": _state["error"],
                                         "label": _state["mode"]})
                yield _sse("finish", {})
            if finished:
                break
            _state["cond"].wait(timeout=1.0)


def _sse(event, data):
    return "event: {}\ndata: {}\n\n".format(event, json.dumps(data, ensure_ascii=False))


# ==================== Воркер ====================

def _worker(cfg):
    try:
        mode = cfg.get("mode", "firmware")
        _append_line(f"Прошивка проекта: {cfg.get('project_label', '')}")
        _append_line(f"Режим: {MODES[mode]['label']} | Порт: {cfg.get('upload_port', '')}")
        _append_line("")

        # ---- Предпроверка каталога данных ФС ----
        # pio -t uploadfs собирает образ littlefs из [platformio] data_dir
        # (mklittlefs -c $PROJECT_DATA_DIR). Если путь устарел (проект перенесли,
        # категорию переименовали, ini правили руками) — PIO падает с
        # неинформативным «can't read source directory … littlefs.bin Error 1».
        if mode in ("fs", "full"):
            report = projects.data_dir_report(
                cfg.get("ini", ""), cfg.get("data_dir", ""), cfg.get("cwd", ""))
            if not report["ok"]:
                _append_line("[flash] " + projects.data_dir_error_text(report))
                _set_step_running(1)
                _set_step_error(1)
                _fail(1, projects.data_dir_error_text(report))
                return

        for idx, step in enumerate(MODES[mode]["steps"], start=1):
            step_id = idx
            target = step["target"]
            label = step["label"]
            _set_step_running(step_id)
            _append_line("")
            _append_line(f"=== Шаг {step_id}. {label} ===")
            cmd = [cfg["pio"], "run", "-c", cfg["ini"], "-e", cfg["env"],
                   "-t", target, "--upload-port", cfg["upload_port"]]
            _append_line("> " + " ".join(cmd))
            rc, _out = _run_streaming(cmd, cfg["cwd"])
            if rc != 0:
                _set_step_error(step_id)
                _fail(step_id, f"Ошибка шага '{label}' (код {rc})")
                return
            _set_step_done(step_id)

        # Обновляем готовые артефакты в iotm/<платформа>/400/ проекта: pio при
        # uploadfs/upload пересобирает .pio/build/<платформа>/firmware.bin и
        # littlefs.bin, поэтому копия в папке проекта должна быть свежей.
        try:
            build.copy_firmware(cfg)
        except Exception as e:  # noqa: BLE001
            _append_line(f"[copy] Не удалось обновить iotm/: {e}")

        _append_line("")
        _append_line("=== Успех! Прошивка записана ===")
        _finish_success()
    except Exception as e:  # noqa: BLE001
        _append_line(f"[flash] Внутренняя ошибка: {e}")
        _fail(None, str(e))


def _set_step_running(step_id):
    with _state["cond"]:
        for s in _state["steps"]:
            if s["id"] == step_id:
                s["status"] = "running"
        _state["cond"].notify_all()


def _set_step_done(step_id):
    with _state["cond"]:
        for s in _state["steps"]:
            if s["id"] == step_id:
                s["status"] = "done"
        _state["cond"].notify_all()


def _set_step_error(step_id):
    with _state["cond"]:
        for s in _state["steps"]:
            if s["id"] == step_id:
                s["status"] = "error"
        _state["cond"].notify_all()


def _run_streaming(cmd, cwd):
    """Запуск команды с построчным выводом. Возвращает (returncode, текст)."""
    full = []
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
        _append_line(f"[flash] Не удалось запустить команду: {e}")
        return 127, "\n".join(full)
    except Exception as e:  # noqa: BLE001
        _append_line(f"[flash] Ошибка выполнения: {e}")
        return 1, "\n".join(full)
    return rc, "\n".join(full)