"""
Business logic for IoTManager device management.

All functions previously in app.py have been extracted here for modularity.
Global state is stored in state/globals.py.
"""

import json
import os
import re
import shutil
import socket
import struct
import subprocess
import time
import threading
import ipaddress
import urllib.request
from concurrent.futures import ThreadPoolExecutor, as_completed

from utils import ws_client, ota
import state.globals as globals_
from state.globals import (
    _device_folders_lock, _device_folders,
    _devices_lock, _devices,
    _device_states_lock, _device_states,
    _scan_lock, _scan_state,
    _fetch_progress,
)

# ==================== Constants ====================
MULTICAST_GROUP = "239.255.255.255"
MULTICAST_PORT = 4210
DEVICES_TIMEOUT = 90
PING_SKIP_AFTER_MULTICAST = 70.0
HTTP_TIMEOUT = 2.0
PING_TIMEOUT_MS = 700
PING_CONCURRENCY = 64
PING_INTERVAL = 60.0
FAILS_YELLOW = 3
FAILS_RED = 3

# Кэш доступности IP (пинг), чтобы не пинговать каждый раз в списке источников
_host_reachable_cache = {}        # ip -> (ok: bool, ts: float)
_HOST_REACHABLE_TTL = 3.0


# [DEPRECATED] Реальный ICMP-пинг для проверки доступности не используется
# (решение: доступность определяется FSM-статусом _device_states). Оставлен
# по правилу «код не удалять»; ни один маршрут его не вызывает.
def _host_reachable(ip, timeout=2):
    """Пингует IP для реальной проверки доступности.

    Надёлен кэшем на _HOST_REACHABLE_TTL секунд. Надёжнее кэшированного
    статуса конечного автомата (_device_states), который удерживает «зелёный»
    до DEVICES_TIMEOUT (90 сек) и может показывать офлайн-устройство как онлайн.
    """
    if not ip:
        return False
    now = time.time()
    entry = _host_reachable_cache.get(ip)
    if entry and now - entry[1] < _HOST_REACHABLE_TTL:
        return entry[0]
    ok = False
    try:
        param = "-n" if os.name == "nt" else "-c"
        res = subprocess.run(
            ["ping", param, "1", "-w", str(timeout * 1000)],
            capture_output=True, timeout=timeout + 1,
        )
        ok = res.returncode == 0
    except Exception:
        ok = False
    _host_reachable_cache[ip] = (ok, now)
    return ok

# ==================== Device States ====================
STATE_GREEN = "green"
STATE_YELLOW = "yellow"
STATE_RED = "red"
STATE_GREY = "grey"

# ==================== Paths ====================
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DEVICE_DIR_ROOT = os.path.normpath(os.path.join(BASE_DIR, "..", "devices"))

_ssid_cache = {"value": None, "ts": 0.0}
_SSID_TTL = 300


# ==================== Logging ====================
import logging
logger = logging.getLogger(__name__)


# ==================== Multicast Device Detection ====================
# Внимание: _devices и _device_thread живут в state/globals.py (импортированы
# выше). Дублировать их здесь нельзя: локальная копия _devices = {} перекрывала бы
# импорт, и routes/devices.py (удаление устройства → _devices.pop) чистил бы не тот
# словарь, что наполняет multicast-слушатель.


def _mark_device_seen(ip, name=""):
    """Marks multicast packet arrival from IP (updates device attributes)."""
    now = time.time()
    with _devices_lock:
        old = _devices.get(ip)
        _devices[ip] = {
            "ip": ip,
            "name": name or (old or {}).get("name", ""),
            "wg": (old or {}).get("wg", ""),
            "id": (old or {}).get("id", ""),
            "status": bool((old or {}).get("status", False)),
            "fv": (old or {}).get("fv", ""),
            "last_seen": now,
        }


def _get_state(key):
    """Returns state-machine record for folder key (creates on first access)."""
    with _device_states_lock:
        st = _device_states.get(key)
        if st is None:
            st = {"state": STATE_GREY, "fails": 0, "last_confirm": 0.0}
            _device_states[key] = st
        return st


def _confirm_device(key, now=None):
    """Confirms device (multicast or ping+settings): sets green, resets counter."""
    with _device_states_lock:
        st = _device_states.setdefault(
            key, {"state": STATE_GREY, "fails": 0, "last_confirm": 0.0})
        st["state"] = STATE_GREEN
        st["fails"] = 0
        st["last_confirm"] = now or time.time()


def _next_state(cur, fails, confirmed, ping_ok):
    """Computes next state of the device status state machine.

    Args:
      cur       - current state ("grey"/"green"/"yellow"/"red")
      fails     - current ping failure counter
      confirmed - identity confirmed (multicast or ping+settings)
      ping_ok   - ping succeeded

    Returns: (new_state, new_fails).
    """
    if confirmed:
        return STATE_GREEN, 0
    if ping_ok:
        return STATE_GREY, 0
    f = fails + 1
    if cur == STATE_GREEN:
        return (STATE_YELLOW if f <= FAILS_YELLOW else STATE_RED), f
    if cur == STATE_YELLOW:
        return (STATE_RED if f > FAILS_RED else STATE_YELLOW), f
    return cur, f


def _enforce_single_green(ip, keys, confirmed_here):
    """Ensures only one device can be green per IP address."""
    with _device_states_lock:
        green_here = [k for k in keys
                      if _device_states.get(k, {}).get("state") == STATE_GREEN]
        if len(green_here) > 1:
            keep = confirmed_here[0] if confirmed_here else green_here[0]
            for k in green_here:
                if k != keep:
                    st = _device_states[k]
                    st["state"] = STATE_GREY
                    st["fails"] = 0
                    logger.warning(f"One green on IP {ip}: {keep} stays, {k} -> grey")
            green_here = [keep]
    return list(green_here)


def _fetch_settings_identity(ip):
    """Downloads settings.json from device and returns (name, id) or None.

    Used as identity confirmation via ping when multicast is unavailable.
    """
    if _device_busy_fetching(ip) or ota.busy_ip() == ip:
        return None
    try:
        data = ws_client.fetch_settings(ip)
    except Exception:
        logger.warning(f"Could not get settings.json from {ip}")
        return None
    if not data:
        return None
    return str(data.get("name", "")), str(data.get("id", ""))


# Чтение identity из settings.json с повторами. Сразу после ассоциации с точкой
# доступа модуля (192.168.4.x) его WebSocket/HTTP могут ещё не отвечать, поэтому
# однократный запрос вернул бы пусто — и папка получилась бы «по IP» вместо
# <name> <id>. Прошивка всегда пишет в settings.json `id` (= getChipId) и `name`
# (src/EspFileSystem.cpp), так что имя/id появятся, как только устройство откликнется.
AP_IDENTITY_ATTEMPTS = 6         # попытки чтения settings.json в AP-потоке
AP_IDENTITY_DELAY = 2.0          # пауза между попытками, сек


def fetch_identity_with_retry(ip, attempts=AP_IDENTITY_ATTEMPTS, delay=AP_IDENTITY_DELAY):
    """(name, id) из settings.json, с повторами. ('', '') — не удалось за все попытки."""
    attempts = max(1, int(attempts))
    name = dev_id = ""
    for i in range(attempts):
        sj = _fetch_settings_identity(ip)
        if sj:
            name, dev_id = str(sj[0] or "").strip(), str(sj[1] or "").strip()
            if name or dev_id:
                return name, dev_id
        if i + 1 < attempts:
            time.sleep(delay)
    return name, dev_id


def _device_busy_fetching(ip):
    """True if a section fetch is currently active for this IP."""
    return any(
        st.get("stage") == "running" and st.get("ip") == ip
        for st in _fetch_progress.values()
    )


# Кэш локальных подсетей панели: панель могут перевести в другую сеть на ходу,
# поэтому кэш короткий; psutil-опрос интерфейсов на каждый цикл пинга не нужен
_subnets_cache = {"ts": 0.0, "nets": None}
_SUBNETS_TTL = 30.0

# Узел вне локальных подсетей (панель переехала в другую сеть) проверяется
# пингом редко: устройство может быть доступно маршрутом (VPN, несколько
# интерфейсов), поэтому полностью пинг не отключаем
CROSS_SUBNET_RETRY = 600.0
_cross_subnet_retry = {}  # ip -> time.time() следующей полной проверки


def _local_subnets():
    """Список подсетей (ipaddress.ip_network) интерфейсов панели или None.

    None — определить не удалось (psutil недоступен/нет IPv4-адресов):
    оптимизация отключается, пинг ведётся как раньше.
    """
    now = time.time()
    if now - _subnets_cache["ts"] < _SUBNETS_TTL:
        return _subnets_cache["nets"]
    nets = None
    try:
        import psutil
        nets = []
        for addr_list in psutil.net_if_addrs().values():
            for addr in addr_list:
                if addr.family == socket.AF_INET and addr.netmask:
                    try:
                        nets.append(ipaddress.ip_network(
                            f"{addr.address}/{addr.netmask}", strict=False))
                    except ValueError:
                        continue
    except Exception:  # noqa: BLE001 — нет psutil или доступа к интерфейсам
        nets = None
    _subnets_cache["ts"] = now
    _subnets_cache["nets"] = nets
    return nets


def _ip_in_local_subnet(ip):
    """True, если IP принадлежит одной из локальных подсетей панели.

    Подсети неизвестны (None) — возвращает True: пингуем как раньше.
    """
    nets = _local_subnets()
    if nets is None:
        return True
    try:
        addr = ipaddress.ip_address(ip)
    except ValueError:
        return True
    return any(addr in net for net in nets)


def _ping_cycle():
    """One cycle of background device checks (ping + identity confirmation)."""
    now = time.time()
    with _device_folders_lock:
        entries = [(e["key"], e.get("ip")) for e in _device_folders.values() if e.get("ip")]

    by_ip = {}
    for key, ip in entries:
        if _device_busy_fetching(ip) or ota.busy_ip() == ip:
            continue
        by_ip.setdefault(ip, []).append(key)

    for ip, keys in by_ip.items():
        with _devices_lock:
            live = _devices.get(ip)
        confirmed_here = []
        live_name = str(live.get("name", "")) if live else ""
        live_id = str(live.get("id", "")) if live else ""
        mc_fresh = bool(live) and (now - live["last_seen"]) <= DEVICES_TIMEOUT
        mc_no_ping = bool(live) and (now - live["last_seen"]) <= PING_SKIP_AFTER_MULTICAST
        pending = []
        for key in keys:
            if mc_fresh and key in _folder_key_candidates(
                    str(live.get("name", "")), str(live.get("id", "")), ip):
                _confirm_device(key, now)
                confirmed_here.append(key)
            else:
                pending.append(key)
        if pending and not mc_no_ping:
            in_subnet = _ip_in_local_subnet(ip)
            if not in_subnet and now < _cross_subnet_retry.get(ip, 0.0):
                # Узел из чужой подсети (панель в другой сети): не пингуем
                # каждую минуту — считаем недоступным для FSM, полная
                # проверка пингом по расписанию (CROSS_SUBNET_RETRY)
                logger.info(f"Ping skipped for {ip}: foreign subnet, "
                            f"full retry in {_cross_subnet_retry[ip] - now:.0f}s")
                for key in pending:
                    with _device_states_lock:
                        st = _device_states.setdefault(
                            key, {"state": STATE_GREY, "fails": 0, "last_confirm": 0.0})
                        st["state"], st["fails"] = _next_state(
                            st["state"], st["fails"], False, False)
                    logger.info(f"Foreign-subnet fail: {ip} ({key}), fails: {st['fails']}")
            elif _host_pingable(ip):
                logger.info(f"Ping OK: {ip}")
                _cross_subnet_retry.pop(ip, None)
                sj = _fetch_settings_identity(ip)
                sj_name, sj_id = (sj[0], sj[1]) if sj else ("", "")
                sj_keys = set(_folder_key_candidates(sj[0], sj[1], ip)) if sj else set()
                # Сопоставление с папками: точное имя+id, папка «по IP» (усыновляет
                # имя/id) либо папка с тем же именем при «переехавшем» id устройства
                matched = resolve_identity_folder(ip, sj_name, sj_id) if sj else None
                matched_key = matched["key"] if matched else None
                for key in pending:
                    if key in sj_keys or (matched_key and key == matched_key):
                        _confirm_device(key, now)
                        confirmed_here.append(key)
                        logger.info(f"Ping+settings OK: {ip} ({key})")
                    else:
                        with _device_states_lock:
                            st = _device_states.setdefault(
                                key, {"state": STATE_GREY, "fails": 0, "last_confirm": 0.0})
                            st["state"], st["fails"] = _next_state(
                                st["state"], st["fails"], False, True)
                        logger.info(f"Ping OK, no identity confirmation: {ip} ({key}), "
                                    f"reported name/id: '{sj_name}'/'{sj_id}'")
            else:
                logger.info(f"Ping fail (no response): {ip}")
                # Чужая подсеть — следующий полный пинг только через паузу;
                # своя — обычный порядок (пинг на каждом цикле)
                if not in_subnet:
                    _cross_subnet_retry[ip] = now + CROSS_SUBNET_RETRY
                else:
                    _cross_subnet_retry.pop(ip, None)
                for key in pending:
                    with _device_states_lock:
                        st = _device_states.setdefault(
                            key, {"state": STATE_GREY, "fails": 0, "last_confirm": 0.0})
                        st["state"], st["fails"] = _next_state(
                            st["state"], st["fails"], False, False)
                    logger.info(f"Ping fail: {ip} ({key}), fails: {st['fails']}")
        elif pending and mc_no_ping:
            logger.info(f"Ping skipped for {ip}: fresh multicast "
                        f"({now - live['last_seen']:.0f} sec ago)")
        _enforce_single_green(ip, keys, confirmed_here)
        logger.info(f"Status cycle IP={ip} keys={keys} "
                    f"live='{live_name}/{live_id}' confirmed={confirmed_here}")


def _ping_worker():
    """Background thread: ping devices every PING_INTERVAL seconds."""
    time.sleep(1)
    while True:
        try:
            _ping_cycle()
        except Exception as e:
            logger.error(f"Ping cycle error: {e}")
        time.sleep(PING_INTERVAL)


def start_ping_worker():
    """Start background device ping (idempotent). First cycle starts immediately."""
    if globals_._ping_thread and globals_._ping_thread.is_alive():
        return
    globals_._ping_thread = threading.Thread(target=_ping_worker, daemon=True,
                                     name="device-ping-worker")
    globals_._ping_thread.start()


def _grey_peers_on_ip(src_ip, mc_candidates, confirmed_keys):
    """Moves to grey folders sharing the same IP but not matching by name+id."""
    with _device_folders_lock:
        peers = [k for k, e in _device_folders.items() if e.get("ip") == src_ip]
    with _device_states_lock:
        for key in peers:
            if key in confirmed_keys or key in mc_candidates:
                continue
            st = _device_states.setdefault(
                key, {"state": STATE_GREY, "fails": 0, "last_confirm": 0.0})
            if st["state"] != STATE_GREY:
                st["state"] = STATE_GREY
                st["fails"] = 0
                logger.info(f"Multicast {src_ip}: {key} -> grey (no name+id match)")


def _ingest_payload(src_ip, text):
    """Parse payload: JSON array or single object. IP taken from packet header."""
    try:
        obj = json.loads(text)
    except (json.JSONDecodeError, ValueError):
        logger.warning(f"Multicast from {src_ip}: not JSON ({text[:120]!r})")
        return
    items = obj if isinstance(obj, list) else [obj]
    logger.info(f"Multicast from {src_ip}, objects: {len(items)}, data: {text}")
    mc_candidates = set()
    confirmed_keys = []
    for it in items:
        if not isinstance(it, dict):
            continue
        name = str(it.get("name", ""))
        dev_id = str(it.get("id", ""))
        mc_candidates.update(_folder_key_candidates(name, dev_id, src_ip))
        _mark_device_seen(src_ip, name=name)
        with _devices_lock:
            live = _devices.get(src_ip)
            if live:
                live["wg"] = str(it.get("wg", ""))
                live["id"] = dev_id
                live["status"] = bool(it.get("status", False))
                live["fv"] = it.get("fv", "")
        entry = ensure_device_folder(src_ip, name, dev_id)
        if entry:
            # Папка именно этого устройства (созданная по имени/id либо папка «по IP»,
            # усыновившая имя/id) — подтверждаем её
            mc_candidates.add(entry["key"])
            _confirm_device(entry["key"])
            confirmed_keys.append(entry["key"])
    _grey_peers_on_ip(src_ip, mc_candidates, confirmed_keys)


def _build_devices_payload():
    """Current device list with network status, sorted.

    Source is device folders: key = base folder name (unique on disk),
    so EVERY folder is displayed as a separate device.
    """
    now = time.time()
    with _device_folders_lock:
        folder_entries = list(_device_folders.values())
    devices = []
    for e in folder_entries:
        ip = e.get("ip")
        dev = {
            "key": e["key"],
            "ip": ip or "",
            "name": e["name"],
            "wg": "",
            "id": "",
            "status": False,
            "fv": "",
            "online": False,
            "missed": 0,
            "net": "grey",
        }
        live = None
        if ip:
            with _devices_lock:
                live = _devices.get(ip)
        if live:
            dev["wg"] = live["wg"]
            dev["id"] = live["id"]
            dev["status"] = bool(live["status"])
            dev["fv"] = live["fv"]
        st = _get_state(e["key"])
        dev["net"] = st["state"]
        dev["missed"] = st["fails"]
        dev["online"] = st["state"] == STATE_GREEN
        devices.append(dev)
    with _devices_lock:
        known_ips = {d["ip"] for d in devices}
        for src_ip, d in _devices.items():
            if src_ip in known_ips:
                continue
            online = (now - d["last_seen"]) <= DEVICES_TIMEOUT
            devices.append({
                "key": src_ip,
                "ip": src_ip,
                "name": d["name"],
                "wg": d["wg"],
                "id": d["id"],
                "status": bool(d["status"]),
                "fv": d["fv"],
                "online": online,
                "missed": 0,
                "net": "green" if online else "grey",
            })
    devices.sort(key=lambda x: (not x["online"], x["ip"], x["key"]))
    return {"success": True, "devices": devices}


def _devices_signature():
    """Signature of device set for change detection in SSE stream."""
    now = time.time()
    with _device_folders_lock:
        folder_keys = tuple(sorted(_device_folders.keys()))
    with _devices_lock:
        online = tuple(sorted(
            (d["ip"], round(d["last_seen"], 1), (now - d["last_seen"]) <= DEVICES_TIMEOUT)
            for d in _devices.values()))
    with _device_states_lock:
        states = tuple(sorted(
            (k, v["state"], v["fails"])
            for k, v in _device_states.items()
            if k in folder_keys))
    return (folder_keys, online, states)


def _device_listener():
    """Background thread: receive multicast packets from IoTManager devices."""
    logger.info(f"Multicast listener: {MULTICAST_GROUP}:{MULTICAST_PORT}")
    sock = None
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        if hasattr(socket, "SO_REUSEPORT"):
            try:
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
            except OSError:
                pass
        sock.bind(("", MULTICAST_PORT))
        mreq = struct.pack("=4s4s", socket.inet_aton(MULTICAST_GROUP),
                           socket.inet_aton("0.0.0.0"))
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
        sock.settimeout(0.5)
    except OSError as e:
        logger.error(f"Could not setup multicast socket: {e}")
        return

    while True:
        try:
            data, addr = sock.recvfrom(65535)
        except socket.timeout:
            continue
        except OSError:
            break
        try:
            text = data.decode("utf-8")
        except UnicodeDecodeError:
            text = data.decode("latin-1", errors="replace")
        _ingest_payload(addr[0], text)

    if sock:
        sock.close()
    logger.info("Multicast listener stopped")


def start_device_listener():
    """Start multicast listener thread (idempotent)."""
    if globals_._device_thread and globals_._device_thread.is_alive():
        return
    globals_._device_thread = threading.Thread(target=_device_listener, daemon=True,
                                      name="device-multicast-listener")
    globals_._device_thread.start()


# ==================== Device Folders on Disk ====================


def _sanitize_name(name):
    """Sanitizes name for use as folder name (no special characters)."""
    if not name:
        return "unnamed"
    s = re.sub(r"[\\\\/:*?\"<>|]+", "_", str(name))
    s = re.sub(r"\\s+", "_", s).strip("._ ")
    return s or "unnamed"


def _get_wifi_ssid():
    """[DEPRECATED] Gets laptop WiFi SSID (netsh), cached. Fallback: 'NET'."""
    now = time.time()
    if _ssid_cache["value"] and (now - _ssid_cache["ts"]) < _SSID_TTL:
        return _ssid_cache["value"]
    ssid = "NET"
    try:
        out = subprocess.run(
            ["netsh", "wlan", "show", "interfaces"],
            capture_output=True, text=True,
            encoding="cp1251", errors="replace", timeout=5,
        ).stdout or ""
        for line in out.splitlines():
            if "SSID" in line and "BSSID" not in line:
                val = line.split(":", 1)[-1].strip()
                if val:
                    ssid = val
                    break
    except Exception as e:
        logger.warning(f"Could not determine laptop SSID: {e}")
    _ssid_cache["value"] = ssid
    _ssid_cache["ts"] = now
    return ssid


def _folder_base_name(name, dev_id, ip):
    """Canonical base folder name: <name> <id>, or <name> <ip>."""
    return _folder_key_candidates(name, dev_id, ip)[0]


def _ip_based_folder_keys(ip):
    """Ключи папок, созданных по IP (discovery не получил имя/id устройства).

    Это `<ip> <ip>` и легаси-вариант `unnamed <ip>` — единственные ключи, которые
    могут относиться к устройству, ещё не сообщившему своё имя и id.
    """
    return set(_folder_key_candidates("", "", ip))


def _folder_has_identity(entry, ip):
    """True, если в папке уже прописано настоящее имя/id (а не сам IP)."""
    return bool(str(entry.get("id") or "").strip()) or \
        str(entry.get("name") or "").strip() not in ("", ip)


def _key_name_part(key):
    """Имя устройства из ключа папки: ключ — «<имя> <id>» либо легаси «<имя>_<id>»."""
    key = str(key or "")
    for sep in (" ", "_"):
        if sep in key:
            return key.rpartition(sep)[0]
    return key


def _entry_holds_device(entry, key, name):
    """True, если папка относится к устройству с таким именем.

    Имя в записи бывает чистым («Модуль»), «как ключ» («Модуль 14810077-1458392»)
    или IP (папки, созданные по адресу), поэтому сверяем и запись, и часть ключа.
    """
    bare = str(name or "").strip()
    if not bare:
        return False
    for form in (str(entry.get("name") or "").strip(), _key_name_part(key)):
        if not form:
            continue
        if form == bare or _sanitize_name(form) == _sanitize_name(bare):
            return True
        if form.startswith(bare + " "):          # метка вида «<имя> <id>» / «<имя> <ip>»
            return True
    return False


def _apply_identity(entry, name, dev_id):
    """Прописывает имя/id в запись папки (если изменились) и сохраняет meta."""
    name = str(name or "").strip()
    dev_id = str(dev_id or "").strip()
    changed = False
    if name and entry.get("name") != name:
        entry["name"] = name
        changed = True
    if dev_id and str(entry.get("id") or "") != dev_id:
        entry["id"] = dev_id
        changed = True
    if changed:
        _save_folder_meta(entry)
        logger.info(f"Identity adopted for folder '{entry['key']}': "
                    f"'{entry.get('name')}'/'{entry.get('id')}'")
    return entry


def resolve_identity_folder(ip, name, dev_id):
    """Находит папку, которую подтверждает отклик устройства (имя/id с его IP).

    Порядок поиска:
      1) точное совпадение ключа `<имя> <id>` (или легаси `<имя>_<id>`) — обычный случай;
      2) папка «по IP» (`<ip> <ip>`, легаси `unnamed <ip>`, создана discovery, когда
         устройство ещё не отдавало имя/id) — в неё прописываются настоящие имя/id,
         иначе она навсегда серая, а multicast заводит папку-дубль;
      3) папка с тем же именем на том же IP, но другим id — у части прошивок id
         (`getChipId()`) меняется при перезагрузке, поэтому id обновляется в той же
         папке (ключ и каталог на диске не меняются — иначе на каждый ребут
         появлялась бы новая папка).

    Папку, в которой уже прописано ДРУГОЕ имя, не перезаписываем (подмена модуля
    на том же адресе). Возвращает запись папки или None.
    """
    name = str(name or "").strip()
    dev_id = str(dev_id or "").strip()
    if not (name or dev_id):
        return None
    cands = set(_folder_key_candidates(name, dev_id, ip))
    ip_keys = _ip_based_folder_keys(ip)
    target = need_update = None
    with _device_folders_lock:
        same_ip = [(k, e) for k, e in _device_folders.items() if e.get("ip") == ip]
        for key, entry in same_ip:                       # 1) точное совпадение
            if key in cands:
                return entry
        for key, entry in same_ip:                       # 2) папка «по IP»
            if key not in ip_keys:
                continue
            if _folder_has_identity(entry, ip) and not _entry_holds_device(entry, key, name):
                continue                                 # там уже другое устройство
            target, need_update = entry, True
            break
        if target is None and name:                      # 3) «переехавший» id
            for key, entry in same_ip:
                if _entry_holds_device(entry, key, name):
                    target, need_update = entry, True
                    break
    if target is not None and need_update:
        _apply_identity(target, name, dev_id)
    return target


def _folder_key_candidates(name, dev_id, ip):
    """Possible folder keys for (name, id/ip): new separator ' ' and legacy '_'.

    Backward compatibility: folders created with separator '_' must continue
    to match, so no duplicates after update.

    Имя может быть пустым (свежий модуль в режиме точки доступа ещё не назван):
    папку в этом случае создают по IP (`name or ip` в `add_device_by_ip`),
    поэтому и кандидаты строим от IP — иначе устройство никогда не проходит
    проверку идентичности (multicast/settings приносят пустые имя и id) и
    навсегда остаётся в сером статусе. Вариант `unnamed` сохранён для
    совместимости с папками, созданными прежней версией.
    """
    raw = str(name or "")
    if raw.strip():
        bases = [_sanitize_name(raw)]
    else:
        bases = [_sanitize_name(ip), "unnamed"]
    keys = []
    for base in bases:
        if dev_id:
            keys.append(f"{base} {dev_id}")
            keys.append(f"{base}_{dev_id}")
        else:
            keys.append(f"{base} {ip}")
            keys.append(f"{base}_{ip}")
    return keys


def _device_meta_path(folder):
    """Path to device meta file inside device folder."""
    return os.path.join(folder, ".device.json")


def _load_folder_meta(folder):
    """Reads .device.json from device folder (ip/name/id)."""
    try:
        with open(_device_meta_path(folder), "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception:
        return None


def _save_folder_meta(entry):
    """Saves ip/name/id device tuple to folder meta file."""
    try:
        with open(_device_meta_path(entry["folder"]), "w", encoding="utf-8") as f:
            json.dump({
                "key": entry["key"],
                "ip": entry.get("ip"),
                "name": entry.get("name"),
                "id": entry.get("id", ""),
                "wg": entry.get("wg", ""),
            }, f, ensure_ascii=False, indent=2)
    except Exception as e:
        logger.warning(f"Could not save device meta {entry['folder']}: {e}")


def _register_folder_entry(key, ip=None, name=None, create=True):
    """Registers device folder entry in _device_folders.

    Key = base folder name (unique on disk). IP is just an attribute.
    """
    folder = os.path.join(DEVICE_DIR_ROOT, key)
    ram_dir = os.path.join(folder, "RAM")
    fs_dir = os.path.join(folder, "FS")
    if create:
        try:
            os.makedirs(ram_dir, exist_ok=True)
            os.makedirs(fs_dir, exist_ok=True)
        except Exception as e:
            logger.error(f"Could not prepare device folder {folder}: {e}")
    entry = {
        "key": key,
        "folder": folder,
        "ram_dir": ram_dir,
        "fs_dir": fs_dir,
        "name": name if name is not None else key,
        "ip": ip,
    }
    with _device_folders_lock:
        _device_folders[key] = entry
    _save_folder_meta(entry)
    logger.info(f"Device folder: {folder}")
    return entry


def ensure_device_folder(ip, name, dev_id=""):
    """Creates/returns device folder with key <name> <id> (separator is space).

    Idempotent: if folder with this name already exists in memory or on disk,
    not recreated. On repeated discovery by broadcast, UPDATES
    the current IP record (otherwise device forever stays offline).

    Если папки по имени ещё нет, но есть папка «по IP» (создана, когда устройство
    не отдавало имя/id), она и будет использована — с прописыванием в неё имени/id
    (adopt_identity): без этого получилось бы две папки на одно устройство.
    """
    base = _folder_base_name(name, dev_id, ip)
    with _device_folders_lock:
        for cand in _folder_key_candidates(name, dev_id, ip):
            entry = _device_folders.get(cand)
            if entry is None:
                continue
            if entry.get("ip") != ip:
                entry["ip"] = ip
                logger.info(f"Updated IP for device '{cand}': {ip}")
                _save_folder_meta(entry)
            return entry
    adopted = resolve_identity_folder(ip, name, dev_id)
    if adopted:
        return adopted
    # Имени у устройства ещё нет (свежий модуль в режиме точки доступа): ключ и
    # папка остаются «<ip> <ip>», но в дереве показываем сам IP, а не «ip ip»
    bare_name = str(name or "").strip()
    label = ip if (not str(dev_id or "").strip()
                   and (not bare_name or bare_name == ip)) else base
    return _register_folder_entry(base, ip=ip, name=label)


def get_device_folder(key):
    """Returns device folder entry by key (base name) or None."""
    with _device_folders_lock:
        e = _device_folders.get(key)
        if e:
            return e
        for ek, e in _device_folders.items():
            if e.get("ip") == key:
                return e
        key_normalized = key.replace(" ", "_")
        for ek, e in _device_folders.items():
            if ek == key_normalized or ek.replace(" ", "_") == key_normalized:
                return e
    return None


def _scan_device_folders():
    """Registers entries for ALL folders from DEVICE_DIR_ROOT."""
    if not os.path.isdir(DEVICE_DIR_ROOT):
        return
    with _device_folders_lock:
        known = set(_device_folders.keys())
    for folder_name in sorted(os.listdir(DEVICE_DIR_ROOT)):
        if folder_name in known:
            continue
        folder = os.path.join(DEVICE_DIR_ROOT, folder_name)
        if not os.path.isdir(folder):
            continue
        meta = _load_folder_meta(folder)
        if meta:
            ip = meta.get("ip") or None
            name = meta.get("name") or folder_name
            dev_id = meta.get("id", "")
        else:
            m = re.search(r"\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}", folder_name)
            ip = m.group(0) if m else None
            name = folder_name
            dev_id = ""
        if ip and not str(dev_id or "").strip() and folder_name in (
                f"{ip} {ip}", f"{ip}_{ip}", f"unnamed {ip}", f"unnamed_{ip}"):
            name = ip          # папка безымянного модуля: в дереве показываем IP
        _register_folder_entry(folder_name, ip=ip, name=name)
        if dev_id and meta is None:
            with _device_folders_lock:
                _device_folders[folder_name]["id"] = dev_id


# ==================== Network Scanning ====================


def _is_useful_interface(ip):
    """Checks that interface may contain IoT devices."""
    try:
        addr = ipaddress.ip_address(ip)
    except ValueError:
        return False
    if addr.is_loopback:
        return False
    if addr.is_link_local:
        return False
    return True


def _get_wifi_ssids():
    """Map of WiFi interface name -> network SSID (netsh, cached 30 sec)."""
    cache = getattr(_get_wifi_ssids, "_cache", None)
    now = time.time()
    if cache and (now - cache[0]) < 30:
        return cache[1]

    result = {}
    if os.name == "nt":
        try:
            raw = subprocess.run(
                ["netsh", "wlan", "show", "interfaces"],
                capture_output=True, timeout=5,
            ).stdout or b""
            out = None
            for enc in ("utf-8", "cp1251"):
                try:
                    out = raw.decode(enc)
                    break
                except UnicodeDecodeError:
                    continue
            if out is None:
                out = raw.decode("utf-8", errors="replace")
            iface_name = None
            for line in out.splitlines():
                m = re.match(r"\\s*(?:Name)\\s*:\\s*(.+)$", line)
                if m:
                    iface_name = m.group(1).strip()
                    continue
                m = re.match(r"\\s*SSID\\s*:\\s*(.+)$", line)
                if m and iface_name:
                    ssid = m.group(1).strip()
                    if ssid:
                        result[iface_name] = ssid
        except Exception as e:
            logger.warning(f"Could not get WiFi interface SSIDs: {e}")

    _get_wifi_ssids._cache = (now, result)
    return result


def get_network_interfaces():
    """Returns list of network interfaces where ESP devices may be."""
    try:
        import psutil
    except ImportError:
        ip = _get_local_ip()
        if _is_useful_interface(ip):
            return [{"ip": ip, "label": f"{ip} (auto-detect)"}]
        return []

    wifi_ssids = _get_wifi_ssids()

    addrs = []
    for iface, addr_list in psutil.net_if_addrs().items():
        for addr in addr_list:
            if addr.family == socket.AF_INET and _is_useful_interface(addr.address):
                iface_label = wifi_ssids.get(iface, iface)
                addrs.append({"ip": addr.address, "label": f"{addr.address} - {iface_label}"})

    seen = set()
    unique = []
    for a in addrs:
        if a["ip"] not in seen:
            seen.add(a["ip"])
            unique.append(a)

    def sort_key(item):
        ip_str = item["ip"]
        try:
            addr = ipaddress.ip_address(ip_str)
            if addr.is_private:
                return 0
        except ValueError:
            pass
        return 1

    unique.sort(key=sort_key)
    return unique


def scan_network_worker(subnet_label=None):
    """Background subnet scanning: ping -> identify -> add.

    If subnet_label is specified, only that /24 subnet is scanned.
    If subnet_label == '__all__', all detected subnets are scanned.
    Otherwise, the active subnet is automatically determined.
    """
    try:
        if subnet_label == "__all__":
            hosts = _get_all_subnet_hosts()
            local_ip = "all interfaces"
        elif subnet_label:
            hosts = _get_subnet_hosts(subnet_label)
            local_ip = subnet_label
        else:
            hosts = _get_local_subnet()
            local_ip = _get_local_ip()

        with _scan_lock:
            _scan_state.update({
                "total": len(hosts),
                "done": 0,
                "subnet": (subnet_label if subnet_label == "__all__" else f"{local_ip}/24") if hosts else "",
                "alive": [],
                "found": [],
                "failed": [],
                "error": None if hosts else "Could not determine local subnet",
            })
        if not hosts:
            return

        with ThreadPoolExecutor(max_workers=PING_CONCURRENCY) as ex:
            futures = {ex.submit(_host_pingable, ip): ip for ip in hosts}
            for fut in as_completed(futures):
                ip = futures[fut]
                try:
                    ok = bool(fut.result())
                except Exception:
                    ok = False
                with _scan_lock:
                    _scan_state["done"] += 1
                    if ok:
                        _scan_state["alive"].append(ip)
                if ok:
                    res = add_device_by_ip(ip)
                    with _scan_lock:
                        if res["success"]:
                            _scan_state["found"].append(res["device"])
                        else:
                            _scan_state["failed"].append({"ip": ip, "error": res["error"]})
    except Exception as e:
        logger.error(f"Network scan error: {e}")
        with _scan_lock:
            _scan_state["error"] = f"Scan error: {e}"
    finally:
        with _scan_lock:
            _scan_state["running"] = False
        globals_._scan_thread = None


def _get_all_subnet_hosts():
    """Collects all /24 subnets from all useful interfaces."""
    all_hosts = []
    seen = set()
    try:
        import psutil
    except ImportError:
        ip = _get_local_ip()
        if _is_useful_interface(ip):
            for h in _get_subnet_hosts(ip):
                if h not in seen:
                    seen.add(h)
                    all_hosts.append(h)
        return all_hosts

    for iface, addr_list in psutil.net_if_addrs().items():
        for addr in addr_list:
            if addr.family == socket.AF_INET and _is_useful_interface(addr.address):
                for h in _get_subnet_hosts(addr.address):
                    if h not in seen:
                        seen.add(h)
                        all_hosts.append(h)

    return all_hosts


def _get_subnet_hosts(ip_label):
    """Returns list of /24 subnet IPs for the given IP."""
    try:
        net = ipaddress.ip_network(f"{ip_label}/24", strict=False)
        return [str(h) for h in net.hosts()]
    except ValueError:
        return []


def _get_local_ip():
    """Local IPv4 address of the laptop."""
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]
    except Exception:
        return "127.0.0.1"
    finally:
        s.close()


def _get_local_subnet():
    """Local /24 subnet address range (list of strings)."""
    ip = _get_local_ip()
    if ip.startswith("127."):
        return []
    try:
        net = ipaddress.ip_network(f"{ip}/24", strict=False)
        return [str(h) for h in net.hosts()]
    except ValueError:
        return []


def _host_pingable(ip):
    """Checks host liveness via system ping (Windows/Unix)."""
    if os.name == "nt":
        args = ["ping", "-n", "1", "-w", str(PING_TIMEOUT_MS), ip]
    else:
        args = ["ping", "-c", "1", "-W", str(max(1, PING_TIMEOUT_MS // 1000)), ip]
    try:
        r = subprocess.run(
            args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
            timeout=(PING_TIMEOUT_MS / 1000) + 1,
        )
        return r.returncode == 0
    except Exception:
        return False


def _http_get(ip, path, timeout=HTTP_TIMEOUT):
    """Simple GET to device; returns (status, text) or (None, '')."""
    try:
        with urllib.request.urlopen(f"http://{ip}{path}", timeout=timeout) as resp:
            return resp.status, resp.read().decode("utf-8", errors="replace")
    except Exception:
        return None, ""


def _parse_devlist(text):
    """Extract info from devlist.json (JSON array of IoTManager objects)."""
    try:
        data = json.loads(text)
    except (json.JSONDecodeError, ValueError):
        return None
    items = data if isinstance(data, list) else [data]
    for it in items:
        if isinstance(it, dict) and ("name" in it or "wg" in it):
            return {
                "name": str(it.get("name", "")),
                "wg": str(it.get("wg", "")),
                "id": str(it.get("id", "")),
                "fv": it.get("fv", ""),
            }
    return None


def identify_device(ip):
    """Accurate identification of IoTManager device by IP.

    Primary sign: /devlist.json response is valid JSON array.
    Fallback: standard ESP FS editor (GET /status with isOk and type).
    Returns dict {kind, name, wg, id, fv} or None.
    """
    status, text = _http_get(ip, "/devlist.json")
    if status == 200:
        info = _parse_devlist(text)
        if info:
            return {"kind": "iotmanager", **info}
    status, text = _http_get(ip, "/status")
    if status == 200 and '"isOk"' in text and '"type"' in text:
        return {"kind": "esp", "name": "", "wg": "", "id": "", "fv": ""}
    return None


def add_device_by_ip(ip, attempts=1, delay=AP_IDENTITY_DELAY):
    """Add device by IP. Returns result dict.

    Идентичность берётся как у multicast-устройств: имя и id — из `settings.json`
    (`fetch_identity_with_retry` → `_fetch_settings_identity`), папка формируется по
    ключу `<name> <id>` (`ensure_device_folder`) — без дублирующей папки «по IP».
    `/devlist.json` (`identify_device`) — запасной источник имени/id и признак, что
    по адресу действительно IoTManager.

    attempts/delay — сколько раз читать settings.json и с какой паузой. Поток
    подключения к AP модуля передаёт `AP_IDENTITY_ATTEMPTS`: сразу после
    ассоциации устройство может отвечать не с первого раза.
    """
    ip = (ip or "").strip()
    try:
        ipaddress.ip_address(ip)
    except ValueError:
        return {"success": False, "error": "Invalid IP address"}
    ident = identify_device(ip)
    sj_name, sj_id = fetch_identity_with_retry(ip, attempts=attempts, delay=delay)
    ident = ident or {}
    kind = ident.get("kind", "iotmanager")
    name = (sj_name or str(ident.get("name") or "")).strip()
    dev_id = (sj_id or str(ident.get("id") or "")).strip()
    if not name and not dev_id:
        if kind != "esp":
            # У IoTManager имя и id всегда есть (settings.json: name + id=getChipId).
            # Пусто = устройство не откликнулось. Папку «по IP» не заводим: ключ
            # папки — имя+id, как и у устройств, найденных multicast-пакетом.
            return {"success": False,
                    "error": "Device did not report name/id (settings.json) at this IP"}
        name = ip                       # ESP-редактор ФС: имени нет — папка по адресу
    entry = ensure_device_folder(ip, name or ip, dev_id)
    return {"success": True, "device": {
        "key": entry["key"],
        "ip": ip,
        "name": name or ip,
        "wg": ident.get("wg", ""),
        "id": dev_id,
        "fv": ident.get("fv", ""),
        "kind": kind,
        "folder": entry["folder"],
    }}


# ==================== Fetch Progress ====================
# _fetch_progress импортирован из state/globals.py (см. список импорта выше):
# локальная копия {} перекрывала бы импорт, и routes/devices.py, который берёт
# _fetch_progress отсюда, писал бы прогресс в другой словарь.


# ==================== Utility Functions ====================


def _scan_snapshot():
    """Snapshot of scan state."""
    with _scan_lock:
        return dict(_scan_state)


def _walk_tree(root):
    """Returns (dirs, files) - relative paths to dirs and files in root."""
    dirs, files = [], []
    if not os.path.isdir(root):
        return dirs, files
    for base, subdirs, filenames in os.walk(root):
        for d in subdirs:
            dirs.append(os.path.relpath(os.path.join(base, d), root))
        for fn in filenames:
            files.append(os.path.relpath(os.path.join(base, fn), root))
    dirs.sort()
    files.sort()
    return dirs, files


def _safe_path(root, rel):
    """Safely resolves relative path within root (protection from traversal).

    Returns absolute path or None if path is outside root.
    """
    if not rel:
        return None
    norm = os.path.normpath(rel)
    if norm.startswith("..") or os.path.isabs(norm):
        return None
    root_real = os.path.realpath(root)
    candidate = os.path.realpath(os.path.join(root_real, norm))
    if candidate != root_real and not candidate.startswith(root_real + os.sep):
        return None
    return candidate


def _rmtree_with_retry(folder, attempts=5, delay=0.5):
    """Tries to delete folder multiple times. True on success."""
    for attempt in range(attempts):
        try:
            shutil.rmtree(folder)
            return True
        except OSError as e:
            logger.warning(f"Delete {folder}: attempt {attempt+1}/{attempts} failed: {e}")
            time.sleep(delay)
    return False


def _rmtree_background(folder, max_minutes=10):
    """Background deletion of 'stuck' folder: retries every 5 sec.

    Folder may be blocked by another process (e.g., acrotray.exe holds it
    as working directory). When the block goes away - folder will be deleted.
    """
    deadline = time.time() + max_minutes * 60
    while time.time() < deadline:
        try:
            shutil.rmtree(folder)
            logger.info(f"Device folder deleted (delayed): {folder}")
            return
        except OSError:
            pass
        time.sleep(5)
    logger.error(f"Could not delete device folder in {max_minutes} min: {folder}")
