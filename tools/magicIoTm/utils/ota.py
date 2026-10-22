# -*- coding: utf-8 -*-
"""
ota — прошивка выбранного проекта на IoTManager-устройство по воздуху (Wi-Fi).

Два механизма (fs_method):
  * 'flash' — запись целого образа через НАТИВНЫЙ pull-механизм прошивки:
      инструмент поднимает локальный HTTP-сервер .bin, а устройство САМО
      скачивает файлы по команде GET /localota_handler?server=<url>,
      где <url> = http://<ip_пк>:<port>. В UpgradeFirm.cpp:
          type=1 -> только firmware.bin
          type=2 -> только littlefs.bin
          type=3 -> littlefs.bin, затем firmware.bin
      Это надёжнее, чем ручной multipart-POST на /update (ESP8266WebServer
      часто рвёт передачу на ESP8266 -> «[OTA] Ошибка записи данных»).
  * 'copy'  — пофайловая загрузка из data_svelte через POST /edit
      (handleFileUpload); каталоги создаются через PUT /edit?path=/dir/.

Режимы (mode), выбираемые в модалке, с учётом fs_method:
    'firmware'             — только прошивка               (pull, type=1)
    'fs'   + flash         — только ФС, записать образ     (pull, type=2)
    'fs'   + copy          — только ФС, скопировать файлы  (data_svelte -> /edit)
    'full' + flash         — ФС(образ)+прошивка            (pull, type=3)
    'full' + copy          — ФС(копия) затем прошивка      (/edit, затем pull type=1)

Во время OTA следует отключать фоновый пинг устройства (одинаковые IP могут
быть у разных устройств) — для этого есть busy_ip().

Прогресс отдаётся по SSE через event_stream() (паттерн как в flash.py).
"""

import configparser
import json
import os
import time
import threading
import uuid

import http.client
import http.server
import urllib.request


HTTP_PORT = 80
UPLOAD_EDIT_PATH = "/edit"       # POST multipart: произвольный файл FS
FIELD_NAME = "data"
_CHUNK_SIZE = 32 * 1024
_BETWEEN_STEPS_DELAY = 1.5       # пауза между шагами
# Отдача .bin мелким чанком с паузой: ESP8266 (маленький TCP/read-буфер, запись
# во flash) не поспевает за быстрым потоком и рвёт соединение на середине.
_PULL_SERVE_CHUNK = 4096
_PULL_SERVE_DELAY = 0.006        # секунд между чанками
_UPDATE_TYPES = {"firmware": 1, "fs": 2, "full": 3}   # типы UpgradeFirm
# Признак LibreTiny-окружения в platformio.ini (как в core/config.py: LT_FS_SCRIPT).
LT_FS_SCRIPT = "lt_fsbuild.py"
# Отказ OTA-образа ФС для LibreTiny: Update.begin() принимает только U_FLASH,
# поэтому образ littlefs.bin загрузить нельзя — только copy либо USB (ltchiptool).
_LT_FS_FLASH_ERROR = (
    "Для LibreTiny (bk7231n) запись образа ФС по воздуху невозможна "
    "(Update принимает только U_FLASH): выберите «ФС: скопировать файлы» "
    "или прошейте ФС по USB (таргет flashfs)."
)


class OtaError(Exception):
    """Ошибка OTA-прошивки (пользовательское сообщение)."""


# ==================== Шаги ====================

def _is_libretiny_env(ini_path, env):
    """True, если env собирает ФС кастомным таргетом LibreTiny (lt_fsbuild.py).

    Признак берётся из platformio.ini (секция [env:<env>], поле extra_scripts) —
    как в core/config.py:_is_lt_env, чтобы не решать по имени env (там могут
    быть и esp-окружения с похожими именами).
    """
    if not env or not ini_path or not os.path.isfile(ini_path):
        return False
    try:
        cp = configparser.ConfigParser(strict=False, interpolation=None)
        cp.read(ini_path, encoding="utf-8")
        scripts = cp.get(f"env:{env}", "extra_scripts", fallback="")
    except Exception:  # noqa: BLE001 — при любой проблеме считаем обычной ESP
        return False
    return LT_FS_SCRIPT in scripts


def build_steps(mode, fs_method, env="", ini_path=""):
    """Список шагов для (mode, fs_method).

    kind:
      'pull' — устройство само скачивает .bin с локального сервера
               (fields: type, files[label], label)
      'copy' — пофайловая загрузка data_svelte через POST /edit
      'error' — неподдерживаемая комбинация (env + fs_method); воркер превращает
               такой шаг в OtaError, API возвращает 400 до запуска воркера

    Для LibreTiny (bk7231n): запись образа ФС невозможна (только U_FLASH),
    поэтому fs_method='flash' с mode='fs'/'full' отдаёт шаг 'error';
    mode='full' там = copy (ФС) + pull type=1 (прошивка).
    """
    lt = _is_libretiny_env(ini_path, env)
    if mode == "firmware":
        return [{"kind": "pull", "type": 1, "files": ["firmware.bin"],
                 "label": "Загрузка прошивки"}]
    if mode == "fs":
        if fs_method == "copy":
            return [{"kind": "copy", "label": "Копирование файлов FS на устройство"}]
        if lt:
            return [{"kind": "error", "label": _LT_FS_FLASH_ERROR}]
        return [{"kind": "pull", "type": 2, "files": ["littlefs.bin"],
                 "label": "Загрузка файловой системы FS"}]
    # mode == full
    if fs_method == "copy":
        return [
            {"kind": "copy", "label": "Копирование файлов FS на устройство"},
            {"kind": "pull", "type": 1, "files": ["firmware.bin"],
             "label": "Загрузка прошивки"},
        ]
    if lt:
        return [{"kind": "error", "label": _LT_FS_FLASH_ERROR}]
    return [{"kind": "pull", "type": 3,
             "files": ["littlefs.bin", "firmware.bin"],
             "label": "Полная прошивка (FS + прошивка)"}]


# ==================== Состояние ====================

_state = {
    "running": False,
    "success": None,
    "mode": None,
    "fs_method": None,
    "steps": [],
    "lines": [],
    "error_step": None,
    "error": None,
    "project_label": "",
    "ip": None,
    "current_step": None,
    "progress": {"file": None, "done": 0, "total": 0},
    "cond": threading.Condition(),
}

_lock = threading.Lock()


def _append_line(text):
    with _state["cond"]:
        _state["lines"].append(text)
        _state["cond"].notify_all()


def _set_running(flag):
    with _state["cond"]:
        _state["running"] = flag
        _state["cond"].notify_all()


def _reset_state(project_label="", mode="", fs_method="flash", ip="", env="", ini_path=""):
    steps = []
    for i, st in enumerate(build_steps(mode, fs_method, env, ini_path), start=1):
        steps.append({"id": i, "label": st["label"], "status": "pending"})
    with _state["cond"]:
        _state.update({
            "running": False,
            "success": None,
            "mode": mode,
            "fs_method": fs_method,
            "steps": steps,
            "lines": [],
            "error_step": None,
            "error": None,
            "project_label": project_label,
            "ip": ip,
            "current_step": None,
            "progress": {"file": None, "done": 0, "total": 0},
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


def busy_ip():
    """IP устройства, которое сейчас прошивается, либо None."""
    with _state["cond"]:
        return _state["ip"] if _state["running"] else None


def get_status():
    with _state["cond"]:
        return {
            "running": _state["running"],
            "success": _state["success"],
            "mode": _state["mode"],
            "fs_method": _state["fs_method"],
            "steps": list(_state["steps"]),
            "error_step": _state["error_step"],
            "error": _state["error"],
            "project_label": _state["project_label"],
            "ip": _state["ip"],
            "progress": dict(_state["progress"]),
        }


def start(cfg):
    """Запуск OTA в фоне. cfg — dict (см. app.py): mode, fs_method, ip, files,
    data_dir, pc_ip, project_label, timeout."""
    with _lock:
        if _state["running"]:
            return False
        mode = cfg.get("mode", "firmware")
        fs_method = cfg.get("fs_method", "flash")
        _reset_state(cfg.get("project_label", ""), mode, fs_method, cfg.get("ip", ""),
                     cfg.get("env", ""), cfg.get("ini", ""))
        _set_running(True)
    t = threading.Thread(target=_worker, args=(cfg,), daemon=True)
    t.start()
    return True


def event_stream():
    """Генератор SSE-событий OTA: log, step, progress, done/error, finish.

    События собираются под блокировкой, а отдаются (yield) уже вне её:
    yield при удержанной блокировке подвешивает воркер OTA, если SSE-клиент
    медленно читает поток.
    """
    idx = 0
    emitted_step = {}
    done_sent = False
    last_progress = None
    while True:
        events = []
        finished = False
        with _state["cond"]:
            while idx < len(_state["lines"]):
                events.append(_sse("log", {"text": _state["lines"][idx]}))
                idx += 1
            for s in _state["steps"]:
                status = s["status"]
                if status != "pending" and emitted_step.get(s["id"]) != status:
                    emitted_step[s["id"]] = status
                    events.append(_sse("step", {"id": s["id"], "label": s["label"], "status": status}))
            pr = _state["progress"]
            pkey = (pr["file"], pr["done"], pr["total"])
            if pkey != last_progress:
                last_progress = pkey
                events.append(_sse("progress", {"file": pr["file"], "done": pr["done"], "total": pr["total"]}))
            finished = not _state["running"]
            if finished and not done_sent:
                done_sent = True
                if _state["success"]:
                    events.append(_sse("done", {}))
                else:
                    # success=None значит, что OTA не запускалась (например,
                    # авто-reconnect старой вкладки) — не отдаём "неизвестную ошибку"
                    err = _state["error"] or "OTA не запущена (поток открыт без запуска прошивки)"
                    events.append(_sse("error", {"step": _state["error_step"],
                                                 "error": err,
                                                 "label": _state["mode"]}))
                events.append(_sse("finish", {}))
        for ev in events:
            yield ev
        if finished:
            break
        with _state["cond"]:
            _state["cond"].wait(timeout=1.0)


def _sse(event, data):
    return "event: {}\ndata: {}\n\n".format(event, json.dumps(data, ensure_ascii=False))


# ==================== Воркер ====================

def _worker(cfg):
    try:
        mode = cfg.get("mode", "firmware")
        fs_method = cfg.get("fs_method", "flash")
        ip = cfg.get("ip", "")
        timeout = int(cfg.get("timeout", 240))
        steps = build_steps(mode, fs_method, cfg.get("env", ""), cfg.get("ini", ""))

        _append_line(f"OTA прошивка проекта: {cfg.get('project_label', '')}")
        _append_line(f"Режим: {mode} ({fs_method}) | Устройство: {ip}")
        _append_line("")

        for idx, st in enumerate(steps, start=1):
            step_id = idx
            _set_current_step(step_id, st["label"])
            _set_step_running(step_id)
            _append_line("")
            _append_line(f"=== Шаг {step_id}. {st['label']} ===")
            if st["kind"] == "error":
                # неподдерживаемая комбинация (например, LibreTiny + fs flash)
                _set_step_error(step_id)
                raise OtaError(st["label"])
            if st["kind"] == "pull":
                _run_pull(ip, cfg, st, timeout)
            elif st["kind"] == "copy":
                _copy_fs(ip, cfg.get("data_dir", ""), timeout)
            _set_step_done(step_id)
            if idx < len(steps):
                time.sleep(_BETWEEN_STEPS_DELAY)

        _append_line("")
        _append_line("=== Успех! OTA завершена ===")
        _append_line("=== Перезагрузка устройства произойдет автоматически ===")
        _finish_success()
    except OtaError as e:
        _set_current_error_step()
        _fail(_state["current_step"], str(e))
    except Exception as e:  # noqa: BLE001
        _append_line(f"[ota] Внутренняя ошибка: {e}")
        _fail(_state.get("current_step"), str(e))


def _set_current_step(step_id, label):
    with _state["cond"]:
        _state["current_step"] = step_id
        _state["progress"] = {"file": label, "done": 0, "total": 0}
        _state["cond"].notify_all()


def _set_current_error_step():
    sid = _state.get("current_step")
    if sid is None:
        return
    with _state["cond"]:
        for s in _state["steps"]:
            if s["id"] == sid and s["status"] == "running":
                s["status"] = "error"
        _state["cond"].notify_all()


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


def _set_progress(file, done, total):
    with _state["cond"]:
        _state["progress"] = {"file": file, "done": done, "total": total}
        _state["cond"].notify_all()


# ==================== Pull-прошивка (native OTA) ====================

def _run_pull(device_ip, cfg, step, timeout):
    """Запускает локальный HTTP-сервер .bin и велит устройству скачать файлы.

    Устройство само выполняет HTTPUpdate (надёжно), инструмент лишь мониторит
    объём выданных байт через SSE-прогресс.
    """
    files = cfg.get("files", {})
    pc_ip = cfg.get("pc_ip", "")
    if not pc_ip:
        raise OtaError("Не определён IP компьютера для локального OTA-сервера")

    need = {}
    for fname in step["files"]:
        path = files.get(fname)
        if not path or not os.path.isfile(path):
            raise OtaError(f"Не найден файл для отправки: {fname} ({path})")
        need[fname] = path

    srv = _start_bin_server(need)
    port = srv.server_address[1]
    base = f"http://{pc_ip}:{port}"
    otype = step.get("type", 3)   # 1=только FW, 2=только FS, 3=полная
    _append_line(f"Локальный OTA-сервер: {base}")
    try:
        # Надёжный HTTP-триггер: устройство само качает .bin с локального сервера.
        # GET блокируется до конца обновления/перезагрузки — гоняем в потоке.
        path = f"/localota_handler?server={base}&type={otype}"
        _append_line(f"> GET http://{device_ip}{path}")
        threading.Thread(target=_http_get,
                         args=(device_ip, path, timeout),
                         daemon=True).start()

        deadline = time.time() + timeout
        while time.time() < deadline:
            if _server_all_done(srv, need):
                _append_line("Файлы полностью скачаны устройством")
                # устройство завершает запись во flash и перезагружается
                time.sleep(2.5)
                return
            time.sleep(0.5)
        raise OtaError("Устройство не завершило скачивание за отведённое время")
    finally:
        _stop_bin_server(srv)


def _start_bin_server(need_files):
    """ThreadingHTTPServer, отдающий need_files {имя: абс_путь}, с прогрессом."""
    state = {"completed": {}, "lock": threading.Lock()}

    class _Handler(http.server.BaseHTTPRequestHandler):
        protocol_version = "HTTP/1.1"

        def do_GET(self):  # noqa: N802
            name = self.path.lstrip("/").split("?")[0]
            _append_line(f"[ota-server] GET {self.path} от {self.client_address[0]}")
            path = need_files.get(name)
            if not path or not os.path.isfile(path):
                self.send_response(404)
                self.send_header("Connection", "close")
                self.end_headers()
                return
            size = os.path.getsize(path)
            self.send_response(200)
            self.send_header("Content-Type", "application/octet-stream")
            self.send_header("Content-Length", str(size))
            self.send_header("Connection", "close")
            self.end_headers()
            sent = 0
            try:
                with open(path, "rb") as f:
                    while True:
                        chunk = f.read(_PULL_SERVE_CHUNK)
                        if not chunk:
                            break
                        self.wfile.write(chunk)
                        sent += len(chunk)
                        with state["lock"]:
                            state["completed"][name] = max(state["completed"].get(name, 0), sent)
                        _set_progress(name, sent, size)
                        time.sleep(_PULL_SERVE_DELAY)
            except (BrokenPipeError, ConnectionResetError, OSError):
                _append_line(f"[ota-server] прервана отдача {name} ({sent}/{size})")
            try:
                self.wfile.flush()
            except Exception:  # noqa: BLE001
                pass

        def log_message(self, *args):  # noqa: A003
            pass

    srv = http.server.ThreadingHTTPServer(("0.0.0.0", 0), _Handler)
    srv.state = state
    srv.need_files = need_files
    srv.thread = threading.Thread(target=srv.serve_forever, daemon=True)
    srv.thread.start()
    return srv


def _server_all_done(srv, need_files):
    with srv.state["lock"]:
        return all(srv.state["completed"].get(n, 0) >= os.path.getsize(p)
                   for n, p in need_files.items())


def _stop_bin_server(srv):
    try:
        srv.shutdown()
        srv.server_close()
    except Exception:  # noqa: BLE001
        pass


def _http_get(host, path, timeout=60):
    """Простой GET; при ошибке возвращает (None, '')."""
    try:
        with urllib.request.urlopen(f"http://{host}{path}", timeout=timeout) as r:
            return r.status, r.read().decode("utf-8", "replace")
    except Exception:  # noqa: BLE001 — устройство перезагружается, ответ может пропасть
        return None, ""


# ==================== Пофайловая загрузка ФС (POST /edit) ====================

def _copy_fs(ip, data_dir, timeout):
    """Копирует все файлы из data_dir (data_svelte) на устройство через /edit."""
    if not os.path.isdir(data_dir):
        raise OtaError(f"Не найден каталог data_svelte: {data_dir}")

    items = []
    for base, _dirs, fnames in os.walk(data_dir):
        for fn in fnames:
            abs_path = os.path.join(base, fn)
            rel = os.path.relpath(abs_path, data_dir).replace("\\", "/")
            items.append((abs_path, rel))
    if not items:
        raise OtaError(f"Каталог data_svelte пуст: {data_dir}")
    items.sort(key=lambda x: x[1])

    dirs = sorted({os.path.dirname(r) for _a, r in items if os.path.dirname(r)})
    for d in dirs:
        _ensure_edit_dir(ip, "/" + d, timeout)

    total = sum(os.path.getsize(a) for a, _r in items)
    sent = 0
    for i, (abs_path, rel) in enumerate(items, 1):
        _append_line(f"[{i}/{len(items)}] {rel}")
        _post_edit_file(ip, abs_path, "/" + rel, timeout)
        sent += os.path.getsize(abs_path)
        _set_progress(rel, sent, total)
    _append_line(f"Скопировано файлов FS: {len(items)}")


def _ensure_edit_dir(ip, dir_path, timeout):
    """Создаёт каталог dir_path (с '/') на устройстве через PUT /edit."""
    conn = http.client.HTTPConnection(ip, HTTP_PORT, timeout=timeout)
    try:
        conn.request("PUT", UPLOAD_EDIT_PATH + "?path=" + dir_path)
        resp = conn.getresponse()
        resp.read()
        # 400 "PATH FILE EXISTS" — каталог уже есть, это не ошибка
        if resp.status not in (200, 400):
            raise OtaError(f"Не удалось создать каталог {dir_path} (HTTP {resp.status})")
    finally:
        try:
            conn.close()
        except Exception:  # noqa: BLE001
            pass


def _post_edit_file(ip, path, filename, timeout):
    """Загружает один файл на /edit устройства (multipart). filename — '/...'."""
    boundary = "----MagicIoTmEdit" + uuid.uuid4().hex
    part_head = (
        "--" + boundary + "\r\n"
        'Content-Disposition: form-data; name="' + FIELD_NAME +
        '"; filename="' + filename + '"\r\n'
        "Content-Type: application/octet-stream\r\n\r\n"
    ).encode("utf-8")
    closing = ("\r\n--" + boundary + "--\r\n").encode("utf-8")
    total = os.path.getsize(path)
    body_len = len(part_head) + total + len(closing)

    conn = http.client.HTTPConnection(ip, HTTP_PORT, timeout=timeout)
    try:
        conn.putrequest("POST", UPLOAD_EDIT_PATH)
        conn.putheader("Content-Type", "multipart/form-data; boundary=" + boundary)
        conn.putheader("Content-Length", str(body_len))
        conn.endheaders()
        conn.send(part_head)
        with open(path, "rb") as f:
            while True:
                chunk = f.read(_CHUNK_SIZE)
                if not chunk:
                    break
                conn.send(chunk)
                time.sleep(0.004)
        conn.send(closing)
        resp = conn.getresponse()
        body = resp.read()
        if resp.status != 200:
            raise OtaError("Не удалось записать {} (HTTP {}{})".format(
                filename, resp.status,
                ": " + body[:200].decode("utf-8", "replace").strip() if body else ""))
    finally:
        try:
            conn.close()
        except Exception:  # noqa: BLE001
            pass