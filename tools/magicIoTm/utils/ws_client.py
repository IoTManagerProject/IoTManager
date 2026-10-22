# -*- coding: utf-8 -*-
"""
WebSocket-клиент (RFC6455) для IoTManager на ESP8266/ESP32.

Используется бэкендом magicIoTm для взаимодействия с устройством по порту 81:

    standWebSocket = WebSocketsServer(81);   // Global.cpp

Чтение (как в tools/ws_cmds.py):
    - /config|  -> itemsj (items.json), widget (widgets.json), config (config.json),
                   scenar (scenario.txt), settin (settings.json)
    - /profile| -> otaupd (ota.json), prfile (profile.json)

Запись файлов обратно на устройство (обратные команды):
    payload = '/' + инвертированный заголовок + '|' + содержимое файла
    прошивка пишет данные после заголовка в файл через writeFileUint8tByFrames().
"""

import base64
import json
import os
import socket
import struct
import time
import urllib.parse
import urllib.request

PORT = 81
DEFAULT_TIMEOUT = 10
HTTP_PORT = 80

# ----------------------------------------------------------------------
# Заголовки ответных блоков (первая часть сообщения от устройства).
# key - заголовок из прошивки, value - имя файла для сохранения.
# ----------------------------------------------------------------------
FILE_NAMES = {
    "layout": "layout.json", "itemsj": "items.json", "widget": "widgets.json",
    "config": "config.json", "scenar": "scenario.txt", "settin": "settings.json",
    "ssidli": "ssidlist.json", "errors": "errors.json", "devlis": "devlist.json",
    "otaupd": "ota.json", "prfile": "profile.json", "params": "params.json",
    "charta": "charts", "status": "status.json",
}

# Файлы, которые мы реально сохраняем при скачивании раздела RAM
# (по командам /config| и /profile|).
RAM_FILES = {
    "itemsj": "items.json", "widget": "widgets.json", "config": "config.json",
    "scenar": "scenario.txt", "settin": "settings.json",
    "otaupd": "ota.json", "prfile": "profile.json",
}

# Команды чтения RAM: имя команды -> заголовок, отправляемый в WebSocket.
RAM_READ_COMMANDS = [
    "/config|",
    "/profile|",
]

# Широковещательный шум (приходит периодически всем клиентам, не является ответом
# на команду). Игнорируется при сохранении в папку устройства.
BROADCAST = {"status", "errors", "ssidli", "devlis"}

# ----------------------------------------------------------------------
# Обратные команды записи файлов обратно на устройство.
# key - имя файла, value - заголовок (инверсия имени команды).
# Только эти файлы поддерживает прошивка (WsServer.cpp /gifnoc|, /oiranecs|, /sgnittes|).
# ----------------------------------------------------------------------
WRITE_HEADERS = {
    "config.json": "/gifnoc|",
    "scenario.txt": "/oiranecs|",
    "settings.json": "/sgnittes|",
    "layout.json": "/tuoyal|",
    "devlist.json": "/tsil|",
}


# ======================================================================
# Низкоуровневый протокол WebSocket
# ======================================================================

def build_text_frame(message):
    """Собирает замаскированный текстовый фрейм (опкод 0x1)."""
    payload = message.encode("utf-8")
    mask = os.urandom(4)
    masked = bytes(b ^ mask[i % 4] for i, b in enumerate(payload))
    length = len(payload)
    header = b"\x81"
    if length < 126:
        header += bytes([0x80 | length])
    elif length < 65536:
        header += bytes([0x80 | 126]) + struct.pack(">H", length)
    else:
        header += bytes([0x80 | 127]) + struct.pack(">Q", length)
    return header + mask + masked


def build_pong_frame(payload=b""):
    """Собирает замаскированный pong-фрейм (опкод 0xA) — ответ на ping сервера.

    Прошивка с включённым heartbeat (enableHeartbeat в WsServer.cpp) пингует
    клиентов; без ответа pong соединение разрывается после серии таймаутов.
    По RFC 6455 pong возвращает payload полученного пинга.
    """
    mask = os.urandom(4)
    masked = bytes(b ^ mask[i % 4] for i, b in enumerate(payload))
    length = len(payload)
    if length < 126:
        header = b"\x8a" + bytes([0x80 | length])
    else:
        header = b"\x8a" + bytes([0x80 | 126]) + struct.pack(">H", length)
    return header + mask + masked


def parse_frame(buf):
    """Извлекает один полный фрейм из буфера.
    Возвращает (fin, opcode, payload, rest) либо (None,None,None,buf)."""
    if len(buf) < 2:
        return None, None, None, buf
    b1, b2 = buf[0], buf[1]
    fin = bool(b1 & 0x80)
    opcode = b1 & 0x0F
    masked = bool(b2 & 0x80)
    length = b2 & 0x7F
    idx = 2
    if length == 126:
        if len(buf) < idx + 2:
            return None, None, None, buf
        length = struct.unpack(">H", buf[idx:idx + 2])[0]
        idx += 2
    elif length == 127:
        if len(buf) < idx + 8:
            return None, None, None, buf
        length = struct.unpack(">Q", buf[idx:idx + 8])[0]
        idx += 8
    mask = None
    if masked:
        if len(buf) < idx + 4:
            return None, None, None, buf
        mask = buf[idx:idx + 4]
        idx += 4
    if len(buf) < idx + length:
        return None, None, None, buf
    payload = bytearray(buf[idx:idx + length])
    if mask:
        for i in range(length):
            payload[i] ^= mask[i % 4]
    return fin, opcode, bytes(payload), buf[idx + length:]


def _handshake(sock, host, port):
    """HTTP-апгрейд до WebSocket. Возвращает байты после блока заголовков."""
    key = base64.b64encode(os.urandom(16)).decode()
    request = (
        f"GET / HTTP/1.1\r\n"
        f"Host: {host}:{port}\r\n"
        f"Upgrade: websocket\r\n"
        f"Connection: Upgrade\r\n"
        f"Sec-WebSocket-Key: {key}\r\n"
        f"Sec-WebSocket-Version: 13\r\n"
        f"Origin: http://{host}\r\n"
        f"\r\n"
    )
    sock.sendall(request.encode())
    resp = b""
    while b"\r\n\r\n" not in resp:
        resp += sock.recv(4096)
    head, _, _rest = resp.partition(b"\r\n\r\n")
    if b"101" not in head.split(b"\r\n", 1)[0]:
        raise ConnectionError(
            f"Не удалось установить WebSocket-соединение:\n{head.decode(errors='replace')}"
        )
    return _rest


def _split_header(payload):
    """Делит полезную нагрузку сообщения на (тип, размер, данные)."""
    part1, _, tail = payload.partition(b"|")
    size, _, data = tail.partition(b"|")
    return part1.decode(errors="replace"), size.decode(errors="replace"), data


# ======================================================================
# Чтение
# ======================================================================

def fetch_ram(host, out_dir, port=PORT, timeout=DEFAULT_TIMEOUT, keep_only_ram=True, progress=None):
    """Скачивает файлы раздела RAM по командам /config| и /profile|.

    Все команды отправляются по одному WebSocket-соединению (так же, как это
    делает веб-интерфейс устройства). Ответные файлы сохраняются в out_dir
    (с перезаписью). Широковещательный шум и посторонние файлы игнорируются.

    progress: опциональный callback progress(fname, done, total), вызывается
    после сохранения каждого файла.

    Возвращает список имён сохранённых файлов.
    """
    os.makedirs(out_dir, exist_ok=True)
    sock = socket.create_connection((host, port), timeout=timeout)
    sock.settimeout(timeout)
    saved = []
    try:
        buf = _handshake(sock, host, port)
        # отправляем все команды чтения подряд (прошивка обрабатывает их по очереди)
        for message in RAM_READ_COMMANDS:
            sock.sendall(build_text_frame(message))

        current_type, current_data = None, b""
        start = time.time()
        last_response_time = start
        QUIET = 1.5
        done = False

        while not done and time.time() - start < timeout:
            # защита от зависшего (но не закрытого) соединения:
            # если долго нет реальных данных — завершаемся
            if time.time() - last_response_time > QUIET:
                break
            while True:
                fin, opcode, pl, rest = parse_frame(buf)
                if pl is not None:
                    break
                try:
                    chunk = sock.recv(4096)
                except socket.timeout:
                    done = True
                    break
                if not chunk:
                    done = True
                    break
                buf = buf + chunk
            if done:
                break
            buf = rest

            if opcode == 0x8:   # Close
                break
            if opcode == 0x9:   # Ping от сервера (heartbeat) — отвечаем Pong
                try:
                    sock.sendall(build_pong_frame(pl))
                except OSError:
                    break
                last_response_time = time.time()
                continue
            if opcode == 0xA:   # Pong — признак активности соединения
                last_response_time = time.time()
                continue

            if opcode == 0x0:   # continuation
                current_data += pl
            elif opcode in (0x1, 0x2):  # новое текстовое/бинарное сообщение
                current_type, _size_str, current_data = _split_header(pl)

            if not (fin and current_type):
                continue

            hdr, data = current_type, current_data
            current_type, current_data = None, b""

            if hdr.startswith("/"):
                # служебный текстовый ответ (/po|, /tstr|)
                last_response_time = time.time()
                continue

            is_broadcast = hdr in BROADCAST
            if keep_only_ram and (is_broadcast or hdr not in RAM_FILES):
                # не сохраняем шум и посторонние файлы
                continue

            fname = RAM_FILES.get(hdr) if keep_only_ram else FILE_NAMES.get(hdr, hdr + ".bin")
            if not fname:
                continue
            path = os.path.join(out_dir, fname)
            with open(path, "wb") as f:
                f.write(data)
            if fname not in saved:
                saved.append(fname)
                if progress:
                    progress(fname, len(saved), len(RAM_FILES))
            last_response_time = time.time()

        return saved
    finally:
        try:
            sock.sendall(bytes([0x88, 0x00]))
        except Exception:
            pass
        sock.close()


def fetch_profile(host, port=PORT, timeout=DEFAULT_TIMEOUT):
    """Скачивает только profile.json (команда /profile|) с устройства.

    Используется для определения платформы (env) устройства при проверке
    совместимости OTA. Возвращает содержимое файла как dict (json) либо None,
    если устройство недоступно или не вернуло profile.
    """
    try:
        sock = socket.create_connection((host, port), timeout=timeout)
        sock.settimeout(timeout)
    except OSError:
        return None
    try:
        buf = _handshake(sock, host, port)
        sock.sendall(build_text_frame("/profile|"))

        current_type, current_data = None, b""
        start = time.time()
        last_response_time = start
        QUIET = 1.5
        profile_bytes = None

        while time.time() - start < timeout:
            # защита от зависшего (но не закрытого) соединения
            if time.time() - last_response_time > QUIET:
                break
            fin, opcode, pl, rest = parse_frame(buf)
            if pl is None:
                try:
                    chunk = sock.recv(4096)
                except socket.timeout:
                    break
                if not chunk:
                    break
                buf = buf + chunk
                continue
            buf = rest

            if opcode == 0x8:   # Close
                break
            if opcode == 0x9:   # Ping от сервера (heartbeat) — отвечаем Pong
                try:
                    sock.sendall(build_pong_frame(pl))
                except OSError:
                    break
                last_response_time = time.time()
                continue
            if opcode == 0xA:   # Pong
                last_response_time = time.time()
                continue
            if opcode == 0x0:   # continuation
                current_data += pl
            elif opcode in (0x1, 0x2):
                current_type, _size_str, current_data = _split_header(pl)

            if not (fin and current_type):
                continue

            hdr, data = current_type, current_data
            current_type, current_data = None, b""

            if hdr.startswith("/"):
                last_response_time = time.time()
                continue

            if FILE_NAMES.get(hdr) == "profile.json":
                profile_bytes = data
                break
            last_response_time = time.time()

        if profile_bytes is not None:
            try:
                return json.loads(profile_bytes.decode("utf-8", errors="replace"))
            except Exception:
                return None
        return None
    finally:
        try:
            sock.sendall(bytes([0x88, 0x00]))
        except Exception:
            pass
        sock.close()


def fetch_settings(host, port=PORT, timeout=DEFAULT_TIMEOUT):
    """Скачивает актуальный settings.json с устройства (команда '/list|').

    Прошивка (WsServer.cpp) на '/list|' отвечает широковещательным блоком 'settin'
    (settingsFlashJson). Лёгкая альтернатива полному fetch_ram: возвращается один файл.
    Используется для подтверждения идентичности устройства по пингу (name+id).
    Возвращает содержимое как dict либо None, если устройство недоступно.
    """
    try:
        sock = socket.create_connection((host, port), timeout=timeout)
        sock.settimeout(timeout)
    except OSError:
        return None
    try:
        buf = _handshake(sock, host, port)
        sock.sendall(build_text_frame("/list|"))

        current_type, current_data = None, b""
        start = time.time()
        last_response_time = start
        QUIET = 1.5
        settings_bytes = None

        while time.time() - start < timeout:
            # защита от зависшего (но не закрытого) соединения
            if time.time() - last_response_time > QUIET:
                break
            fin, opcode, pl, rest = parse_frame(buf)
            if pl is None:
                try:
                    chunk = sock.recv(4096)
                except socket.timeout:
                    break
                if not chunk:
                    break
                buf = buf + chunk
                continue
            buf = rest

            if opcode == 0x8:   # Close
                break
            if opcode == 0x9:   # Ping от сервера (heartbeat) — отвечаем Pong
                try:
                    sock.sendall(build_pong_frame(pl))
                except OSError:
                    break
                last_response_time = time.time()
                continue
            if opcode == 0xA:   # Pong
                last_response_time = time.time()
                continue
            if opcode == 0x0:   # continuation
                current_data += pl
            elif opcode in (0x1, 0x2):
                current_type, _size_str, current_data = _split_header(pl)

            if not (fin and current_type):
                continue

            hdr, data = current_type, current_data
            current_type, current_data = None, b""

            if hdr.startswith("/"):
                last_response_time = time.time()
                continue

            if FILE_NAMES.get(hdr) == "settings.json":
                settings_bytes = data
                break
            last_response_time = time.time()

        if settings_bytes is not None:
            try:
                return json.loads(settings_bytes.decode("utf-8", errors="replace"))
            except Exception:
                return None
        return None
    finally:
        try:
            sock.sendall(bytes([0x88, 0x00]))
        except Exception:
            pass
        sock.close()


def send_command(host, command, payload="", port=PORT, timeout=DEFAULT_TIMEOUT):
    """Отправляет устройству произвольную WS-команду (command с завершающим '|').

    Фрейм: command + payload, например:
        send_command(ip, '/update1|', base_url) -> '/update1|<url>'
    Возвращает True, если фрейм отправлен успешно.
    """
    try:
        sock = socket.create_connection((host, port), timeout=timeout)
        sock.settimeout(timeout)
    except OSError:
        return False
    try:
        _handshake(sock, host, port)
        sock.sendall(build_text_frame(command + payload))
        return True
    except Exception:  # noqa: BLE001
        return False
    finally:
        try:
            sock.sendall(bytes([0x88, 0x00]))
        except Exception:  # noqa: BLE001
            pass
        sock.close()


def reboot(host, port=PORT, timeout=DEFAULT_TIMEOUT):
    """Отправляет устройству команду перезагрузки по WebSocket (/reboot|).

    Прошивка (WsServer.cpp) при получении /reboot| выполняет ESP.restart().
    Возвращает True, если команда отправлена успешно.
    """
    try:
        sock = socket.create_connection((host, port), timeout=timeout)
        sock.settimeout(timeout)
    except OSError:
        return False
    try:
        _handshake(sock, host, port)
        sock.sendall(build_text_frame("/reboot|"))
        return True
    except Exception:  # noqa: BLE001
        return False
    finally:
        try:
            sock.sendall(bytes([0x88, 0x00]))
        except Exception:  # noqa: BLE001
            pass
        sock.close()


# ----------------------------------------------------------------------
# Файловая система (HTTP, порт 80)
# ----------------------------------------------------------------------

def _http_get(host, path, retries=3, delay=0.8):
    """GET по HTTP (порт 80); возвращает тело ответа байтами.

    При нестабильном Wi-Fi соединении с устройством делает несколько
    попыток (retries) с паузой delay перед тем как сдаться.
    """
    url = "http://{}:{}{}".format(host, HTTP_PORT, path)
    last_err = None
    for attempt in range(retries):
        try:
            with urllib.request.urlopen(url, timeout=DEFAULT_TIMEOUT) as resp:
                return resp.read()
        except Exception as e:  # noqa: BLE001 — сеть/сервер, повторяем
            last_err = e
            if attempt + 1 < retries:
                time.sleep(delay)
    raise last_err


def fetch_fs(host, out_dir, progress=None):
    """Скачивает файлы раздела FS по HTTP (порт 80).

    Рекурсивно перечисляет каталоги через /list?dir=<path> (эндпоинт прошивки)
    и скачивает каждый файл через /<path>?download=1, сохраняя в out_dir.

    progress: опциональный callback progress(fname, done, total).

    Возвращает список относительных имён сохранённых файлов.
    """
    os.makedirs(out_dir, exist_ok=True)

    files = []  # (url_path, rel_path)

    def walk(path):
        raw = _http_get(host, "/list?dir=" + urllib.parse.quote(path))
        try:
            items = json.loads(raw.decode("utf-8"))
        except Exception:
            items = []
        # base — текущий каталог без ведущего '/'; для корня пустая строка
        base = "" if path in ("", "/") else path.rstrip("/")
        for it in items:
            raw_name = (it.get("name") or "").strip().lstrip("/")
            if not raw_name:
                continue
            # Прошивка может отдавать как просто имя, так и полный путь
            # (ядро ESP32 2.x — basename, ядро 3.x — полный путь с '/').
            # Берём последний компонент, чтобы пути не дублировались.
            name = raw_name.rsplit("/", 1)[-1]
            if not name:
                continue
            joined = "/" + name if not base else base + "/" + name
            if it.get("type") == "dir":
                walk(joined)
            else:
                files.append((joined, joined.lstrip("/")))

    walk("/")

    total = len(files)
    saved = []
    for i, (url_path, rel) in enumerate(files, 1):
        data = _http_get(host, url_path + "?download=1")
        abs_path = os.path.join(out_dir, rel)
        os.makedirs(os.path.dirname(abs_path), exist_ok=True)
        with open(abs_path, "wb") as f:
            f.write(data)
        saved.append(rel)
        if progress:
            progress(rel, i, total)
    return saved
    saved = []
    for i, (url_path, rel) in enumerate(files, 1):
        data = _http_get(host, url_path + "?download=1")
        abs_path = os.path.join(out_dir, rel)
        os.makedirs(os.path.dirname(abs_path), exist_ok=True)
        with open(abs_path, "wb") as f:
            f.write(data)
        saved.append(rel)
        if progress:
            progress(rel, i, total)
    return saved


def fetch_fs_file(host, rel_path, out_dir, retries=3):
    """Скачивает один файл раздела FS по HTTP (порт 80).

    Аналог fetch_fs, но без обхода дерева: файл /<rel_path>?download=1
    сохраняется в out_dir/<rel_path>. Используется кнопкой
    «⤵ Получить этот файл» панели «Файлы из файловой системы».

    Возвращает относительное имя сохранённого файла.
    Бросает исключение при ошибке сети/недоступности файла или некорректном пути.
    """
    rel = rel_path.strip().lstrip("/")
    if not rel or rel.endswith("/"):
        raise ValueError("Некорректный путь файла FS")
    data = _http_get(host, "/" + urllib.parse.quote(rel) + "?download=1", retries=retries)
    abs_path = os.path.realpath(os.path.join(out_dir, *rel.split("/")))
    root = os.path.realpath(out_dir)
    # защита от выхода за пределы каталога устройства (path traversal)
    if not (abs_path == root or abs_path.startswith(root + os.sep)):
        raise ValueError("Путь вне каталога устройства")
    os.makedirs(os.path.dirname(abs_path), exist_ok=True)
    with open(abs_path, "wb") as f:
        f.write(data)
    return rel


# Заголовок RAM-файла по имени (обратная карта RAM_FILES).
RAM_FILE_HEADERS = {v: k for k, v in RAM_FILES.items()}


def fetch_ram_file(host, filename, out_dir, port=PORT, timeout=DEFAULT_TIMEOUT):
    """Скачивает один файл раздела RAM по WebSocket (порт 81).

    Отправляет ту же команду чтения (/config| или /profile|), что и полная
    выгрузка RAM (fetch_ram), но завершается, как только получен нужный файл.
    Используется для одиночного получения файла («⤵ Получить этот файл»)
    без скачивания всего раздела.

    filename — имя RAM-файла (config.json, items.json, scenario.txt, settings.json,
    widgets.json, ota.json, profile.json); файл сохраняется в out_dir.
    Возвращает имя сохранённого файла. Бросает ValueError, если файл не относится
    к разделу RAM, ConnectionError при ошибке соединения и RuntimeError,
    если устройство не вернуло файл за отведённое время.
    """
    header = RAM_FILE_HEADERS.get(filename)
    if not header:
        raise ValueError(f"Файл {filename} не относится к разделу RAM устройства")
    # /config| отдаёт itemsj/widget/config/scenar/settin, /profile| — otaupd/prfile
    command = "/profile|" if header in ("otaupd", "prfile") else "/config|"
    os.makedirs(out_dir, exist_ok=True)
    sock = socket.create_connection((host, port), timeout=timeout)
    sock.settimeout(timeout)
    try:
        buf = _handshake(sock, host, port)
        sock.sendall(build_text_frame(command))

        current_type, current_data = None, b""
        start = time.time()
        last_response_time = start
        QUIET = 1.5
        while time.time() - start < timeout:
            # защита от зависшего (но не закрытого) соединения
            if time.time() - last_response_time > QUIET:
                break
            fin, opcode, pl, rest = parse_frame(buf)
            if pl is None:
                try:
                    chunk = sock.recv(4096)
                except socket.timeout:
                    break
                if not chunk:
                    break
                buf = buf + chunk
                continue
            buf = rest

            if opcode == 0x8:   # Close
                break
            if opcode == 0x9:   # Ping от сервера (heartbeat) — отвечаем Pong
                try:
                    sock.sendall(build_pong_frame(pl))
                except OSError:
                    break
                last_response_time = time.time()
                continue
            if opcode == 0xA:   # Pong
                last_response_time = time.time()
                continue
            if opcode == 0x0:   # continuation
                current_data += pl
            elif opcode in (0x1, 0x2):
                current_type, _size_str, current_data = _split_header(pl)

            if not (fin and current_type):
                continue

            hdr, data = current_type, current_data
            current_type, current_data = None, b""

            if hdr.startswith("/"):
                # служебный текстовый ответ (/po|, /tstr|)
                last_response_time = time.time()
                continue
            last_response_time = time.time()

            if hdr == header:
                path = os.path.join(out_dir, filename)
                with open(path, "wb") as f:
                    f.write(data)
                return filename

        raise RuntimeError(f"Устройство не вернуло файл {filename}")
    finally:
        try:
            sock.sendall(bytes([0x88, 0x00]))
        except Exception:
            pass
        sock.close()


# ======================================================================
# Запись
# ======================================================================

def write_file(host, filename, content, port=PORT, timeout=DEFAULT_TIMEOUT):
    """Отправляет содержимое файла обратно на устройство.

    Используется обратная WS-команда (например /gifnoc| для config.json).
    Содержимое отправляется одним текстовым фреймом: '/<заголовок>|<данные>'.

    Возвращает True при успешной отправке. Бросает RuntimeError, если файл
    не поддерживается прошивкой для записи.
    """
    header = WRITE_HEADERS.get(filename)
    if not header:
        raise RuntimeError(
            f"Файл '{filename}' не поддерживается прошивкой для записи обратно "
            f"на устройство. Доступны: {', '.join(sorted(WRITE_HEADERS))}."
        )

    if isinstance(content, str):
        content = content.encode("utf-8")

    # большие файлы могут не поместиться в один WS-фрейм на устройстве
    if len(content) > 50000:
        raise RuntimeError(
            f"Файл '{filename}' слишком большой ({len(content)} Б) для записи "
            f"одним WebSocket-фреймом на ESP. Уменьшите размер файла."
        )

    sock = socket.create_connection((host, port), timeout=timeout)
    sock.settimeout(timeout)
    try:
        buf = _handshake(sock, host, port)
        payload = header.encode("utf-8") + content
        sock.sendall(build_text_frame(payload.decode("utf-8", errors="replace")))
        # небольшая пауза, чтобы прошивка успела обработать запись
        time.sleep(0.5)
        return True
    finally:
        try:
            sock.sendall(bytes([0x88, 0x00]))
        except Exception:
            pass
        sock.close()