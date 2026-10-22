"""
Поиск WiFi-точек доступа IoTManager и подключение к ним («живая» антенна панели).

Модуль IoTManager в режиме точки доступа поднимает сеть с именем из параметра
`apssid` (у пользовательских устройств имена начинаются с iotm) и адресом
192.168.4.1 (см. `src/utils/WiFiUtils.cpp`). Панель:

  1. периодическим потоком (`start_wifi_worker`, период WIFI_SCAN_INTERVAL)
     сканирует эфир и, если видит сети с префиксом IOTM_SSID_PREFIX без учёта
     регистра, поднимает флаг alert — фронтенд показывает кнопку «📶» в правом
     углу шапки списка устройств (`tree-devices-title-row`; кнопка всегда видима,
     счётчик новых сетей и подсветка — по флагу alert). Первый скан выполняется
     сразу после старта панели (или при первом запросе состояния,
     `ensure_first_scan`), а не через минуту;
  2. по нажатию на сеть создаёт WiFi-профиль и подключается к ней (`connect`);
     открытая сеть — без пароля, защищённая — WPA2-PSK;
  3. если адрес панели попал в подсеть 192.168.4.0/24, пингует 192.168.4.1 и
     по успешному отклику добавляет устройство (`add_device_by_ip` →
     /devlist.json), после чего оно появляется в дереве устройств по SSE.

Вывод `netsh` локализован (на русской Windows ключи «Сигнал», «Канал»,
«Проверка подлинности»), поэтому разбор устойчив к локали: ключи SSID/BSSID
не переводятся, уровень сигнала ищется по «%», а ключи сигнала/канала и тип
аутентификации — по списку синонимов. Linux-резерв — `nmcli` (только просмотр).

Важно: `netsh wlan show networks` **не инициирует поиск** — он печатает кэш
драйвера, который Windows обновляет сама и нерегулярно. Поэтому перед чтением
вывода панель пытается запустить активное сканирование через WlanScan API
(`wlanapi.dll`, ctypes — без внешних пакетов, `trigger_scan`). Если WlanScan
отказал (драйвер, WlanSvc, политика или запуск без прав — на части машин он
возвращает `ERROR_INVALID_PARAMETER`), панель пишет об этом предупреждение и
читает кэш netsh как есть.
"""

import ipaddress
import json
import logging
import os
import re
import shutil
import socket
import subprocess
import tempfile
import threading
import time
from datetime import datetime, timezone
from xml.sax.saxutils import escape as xml_escape, unescape as xml_unescape
import state.globals as globals_
from state.globals import (
    _wifi_scan_lock, _wifi_scan_state,
    _wifi_connect_lock, _wifi_connect_state,
)
from core.devices import _host_pingable, add_device_by_ip, AP_IDENTITY_ATTEMPTS

logger = logging.getLogger(__name__)

# ==================== Параметры ====================
IOTM_SSID_PREFIX = "iotm"        # префикс имени AP-сети (без учёта регистра)
WIFI_SCAN_INTERVAL = 60.0        # период сканирования эфира, сек
WIFI_FIRST_SCAN_DELAY = 1.0      # пауза перед самым первым сканом в потоке, сек
MIN_RESCAN_GAP = 5.0             # не сканировать эфир чаще, чем раз в N сек
NETSH_TIMEOUT = 8.0              # таймаут команды netsh/nmcli, сек
CONNECT_WAIT = 25.0              # ожидание ассоциации с сетью, сек
WLAN_FAIL_EVENT = 8002           # событие Windows «подключение не удалось» (журнал WLAN)
WLAN_FAIL_POLL = 1.5             # период опроса журнала WLAN во время ожидания, сек
PING_ATTEMPTS = 3                # число попыток пинга AP-адреса
AP_GATEWAY_IP = "192.168.4.1"    # адрес точки доступа прошивки
AP_SUBNET = ipaddress.ip_network("192.168.4.0/24")
SEEN_LIMIT = 60                  # максимум «показанных» имён в истории
MAX_SSID_LEN = 32                # предельная длина SSID по 802.11

# Однократный первый скан после старта: его выполняет либо поток сканирования,
# либо первый запрос состояния (см. ensure_first_scan)
_first_scan_lock = threading.Lock()

# ==================== Активное сканирование эфира (WlanScan API) ====================
# `netsh wlan show networks` не инициирует поиск — печатает кэш драйвера. Активный
# поиск запускается WlanScan из wlanapi.dll. Работаем через ctypes, чтобы не тянуть
# в прошивку панели внешние пакеты (как netsh, так и WlanScan требуют службу WlanSvc).
SCAN_SETTLE = 2.5              # пауза после запуска сканирования, сек
SCAN_API_MIN_GAP = 8.0         # не чаще одного WlanScan в N сек (драйверу нужно время)
SCAN_API_GIVE_UP = 3           # после N отказов WlanScan больше не пробуем
_scan_api_lock = threading.Lock()
_scan_api_ts = 0.0
_scan_api_fails = 0


def _scan_api_available():
    """Загружается ли wlanapi.dll с нужными точками входа. False — иначе."""
    if os.name != "nt":
        return False
    try:
        import ctypes
        dll = ctypes.WinDLL("wlanapi.dll")
        return all(hasattr(dll, n) for n in
                   ("WlanOpenHandle", "WlanCloseHandle", "WlanScan"))
    except Exception as e:                                  # noqa: BLE001
        logger.debug(f"wlanapi.dll недоступна ({e}) — скан только через netsh")
        return False


def _wlanscan_once():
    """Одна попытка WlanScan по всем подключённым WLAN-интерфейсам. True — успех.

    Отдельная функция, чтобы trigger_scan могла единообразно посчитать отказы:
    ранние return внутри try не должны «обнулять» счётчик неудачных попыток.
    """
    import ctypes
    from ctypes import wintypes

    # Структуры WLAN API: GUID интерфейса + описание + состояние
    class GUID(ctypes.Structure):
        _fields_ = [("Data1", wintypes.DWORD),
                    ("Data2", wintypes.WORD),
                    ("Data3", wintypes.WORD),
                    ("Data4", ctypes.c_ubyte * 8)]

    class WLAN_INTERFACE_INFO(ctypes.Structure):
        _fields_ = [("guid", GUID),
                    ("desc", ctypes.c_wchar * 256),
                    ("state", ctypes.c_int)]

    dll = ctypes.WinDLL("wlanapi.dll")
    client = wintypes.HANDLE()
    version = wintypes.DWORD()
    rc = dll.WlanOpenHandle(2, None, ctypes.byref(version),
                            ctypes.byref(client))           # 2 = XP и выше
    if rc != 0:
        logger.debug(f"WlanOpenHandle вернул {rc}")
        return False
    try:
        count = wintypes.DWORD(0)
        size = wintypes.DWORD(0)
        # Первый вызов с NULL-буфером отдаёт нужный размер; счётчик при этом
        # НЕ заполняется, поэтому берём размер и число слотов считаем сами.
        rc = dll.WlanEnumInterfaces(client, None, ctypes.byref(count),
                                    None, ctypes.byref(size))
        if rc != 0 or size.value <= 0:
            logger.debug(f"WlanEnumInterfaces (размер) вернул {rc}, размер {size.value}")
            return False
        slots = size.value // ctypes.sizeof(WLAN_INTERFACE_INFO) + 1
        buf = (WLAN_INTERFACE_INFO * slots)()
        count = wintypes.DWORD(slots)
        rc = dll.WlanEnumInterfaces(client, None, ctypes.byref(count),
                                    ctypes.byref(buf), ctypes.byref(size))
        if rc != 0:
            logger.debug(f"WlanEnumInterfaces вернул {rc}")
            return False
        started = False
        for i in range(min(count.value, slots)):
            if buf[i].state != 1:            # 1 = wlan_interface_state_connected
                continue
            guid_str = "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}" % (
                buf[i].guid.Data1, buf[i].guid.Data2, buf[i].guid.Data3,
                *buf[i].guid.Data4)
            rc = dll.WlanScan(client, None, ctypes.c_wchar_p(guid_str),
                              None, None, None)
            if rc == 0:
                started = True
            else:
                logger.debug(f"WlanScan вернул {rc}")
        return started
    finally:
        dll.WlanCloseHandle(client, None)


def trigger_scan(force=False):
    """Запускает активное сканирование эфира (WlanScan). True — поиск инициирован.

    force=True игнорирует паузу между сканированиями. False, если поиск недоступен
    (не Windows, нет wlanapi, драйвер/WlanSvc отказал — тогда netsh покажет кэш как
    есть). После SCAN_API_GIVE_UP отказов попытки прекращаются, чтобы не тратить
    время на заведомо бесполезные вызовы.
    """
    global _scan_api_ts, _scan_api_fails
    if _scan_api_fails >= SCAN_API_GIVE_UP or not _scan_api_available():
        return False
    with _scan_api_lock:
        if _scan_api_fails >= SCAN_API_GIVE_UP:
            return False
        now = time.time()
        if not force and (now - _scan_api_ts) < SCAN_API_MIN_GAP:
            return False
        _scan_api_ts = now
        try:
            started = _wlanscan_once()
        except Exception as e:                                  # noqa: BLE001
            logger.debug(f"Активный скан не удался: {e}")
            started = False
        if started:
            _scan_api_fails = 0
        else:
            _scan_api_fails += 1
            if _scan_api_fails >= SCAN_API_GIVE_UP:
                logger.warning("Активный скан эфира (WlanScan) недоступен "
                               f"({_scan_api_fails} попытки) — список сетей берётся "
                               "из кэша netsh")
    if started:
        time.sleep(SCAN_SETTLE)        # даём драйверу обновить кэш перед чтением
    return started


# ==================== Разбор вывода netsh ====================
_NET_KEY_RE = re.compile(r"^\s*(SSID|BSSID)\s*\d*\s*[:=]\s*(.*)$")
_SIGNAL_KEY_RE = re.compile(r"^\s*(?:signal|\u0441\u0438\u0433\u043d\u0430\u043b|sinyal)"
                            r"\s*[:=]\s*(?P<num>\d{1,3})\s*%", re.I)
# Запасной вариант: неизвестный ключ, значение которого — «NN%» (строка
# «Использование канала: 0 (0%)» под неё не попадает из-за скобок).
_SIGNAL_FALLBACK_RE = re.compile(r"^\s*[^:=%]{1,40}:\s*(?P<num>\d{1,3})\s*%\s*$")
_CHANNEL_KEY_RE = re.compile(r"^\s*(?:channel|\u043a\u0430\u043d\u0430\u043b|kanal|kana\u0142)"
                             r"\s*[:=]\s*(?P<num>\d+)", re.I)
_AUTH_KEY_RE = re.compile(r"^\s*(authentication|проверка подлинности)\s*[:=]\s*(.+)$", re.I)
_SECURED_RE = re.compile(r"wpa|wep|802\.?1x|802\.11i", re.I)
_IFACE_KEY_RE = re.compile(r"^\s*(SSID|Profile|Профиль)\s*[:=]\s*(.*)$")

# Вывод netsh в Windows идёт в OEM-кодировке консоли (для ru — cp866)
_OEM_CP = None
if os.name == "nt":
    try:
        import ctypes
        _OEM_CP = "cp" + str(ctypes.windll.kernel32.GetOEMCP())
    except Exception:
        _OEM_CP = "cp866"


def _decode(raw):
    """Декодирует вывод консоли: UTF-8 → OEM-кодировка Windows → cp1251."""
    if not raw:
        return ""
    if raw[:2] in (b"\xff\xfe", b"\xfe\xff"):
        try:
            return raw.decode("utf-16")
        except UnicodeDecodeError:
            pass
    for enc in ("utf-8", _OEM_CP, "cp1251"):
        if not enc:
            continue
        try:
            return raw.decode(enc)
        except UnicodeDecodeError:
            continue
    return raw.decode("utf-8", errors="replace")


def _run(cmd, timeout=NETSH_TIMEOUT, combine=False):
    """Выполняет команду БЕЗ оболочки: возвращает (код возврата, текст вывода).

    cmd — список аргументов либо готовая строка командной строки (когда нужны
    кавыки вокруг значения, как у `netsh wlan connect name="..."`).
    combine=True — stderr добавляется к выводу (для текстов сообщений об ошибках).
    """
    try:
        p = subprocess.run(cmd, capture_output=True, timeout=timeout)
        out = _decode(p.stdout or b"")
        if combine and p.stderr:
            out += _decode(p.stderr or b"")
        return p.returncode, out.strip()
    except subprocess.TimeoutExpired:
        logger.warning(f"Таймаут команды: {cmd}")
        return -1, ""
    except Exception as e:
        logger.warning(f"Команда не выполнена ({cmd}): {e}")
        return -1, str(e)


def wifi_backend():
    """'netsh' (Windows), 'nmcli' (Linux) либо None — WiFi недоступен."""
    if os.name == "nt":
        return "netsh" if shutil.which("netsh") else None
    if shutil.which("nmcli"):
        return "nmcli"
    return None


def parse_netsh_networks(text):
    """Разбор `netsh wlan show networks mode=bssid` → список словарей сетей."""
    nets = []
    cur = None
    for line in (text or "").splitlines():
        m = _NET_KEY_RE.match(line)
        if m:
            key, val = m.group(1).upper(), m.group(2).strip()
            if key == "SSID":
                cur = {"ssid": val, "signal": None, "channel": None,
                       "secured": False, "auth": "", "bssid": ""}
                nets.append(cur)
            elif cur is not None and val:
                cur["bssid"] = val
            continue
        if cur is None:
            continue
        if cur["signal"] is None:
            m = _SIGNAL_KEY_RE.match(line) or _SIGNAL_FALLBACK_RE.match(line)
            if m:
                cur["signal"] = max(0, min(100, int(m.group("num"))))
                continue
        if cur["channel"] is None:
            m = _CHANNEL_KEY_RE.match(line)
            if m:
                cur["channel"] = int(m.group("num"))
                continue
        if not cur["auth"]:
            m = _AUTH_KEY_RE.match(line)
            if m:
                cur["auth"] = m.group(2).strip()
                cur["secured"] = bool(_SECURED_RE.search(cur["auth"]))
    return [n for n in nets if n["ssid"]]


def _dedupe_by_ssid(nets):
    """По одной записи на SSID (с наибольшим сигналом), по убыванию сигнала."""
    best = {}
    for n in nets:
        prev = best.get(n["ssid"])
        if prev is None or (n["signal"] or 0) > (prev["signal"] or 0):
            best[n["ssid"]] = n
    return sorted(best.values(), key=lambda x: -(x["signal"] or 0))


def filter_iotm(networks):
    """Сети, имя которых начинается с IOTM_SSID_PREFIX (без учёта регистра)."""
    return [n for n in networks if n["ssid"].lower().startswith(IOTM_SSID_PREFIX)]


def order_networks(networks):
    """Все видимые сети, но AP-сети модулей (префикс iotm) — впереди остальных.

    Порядок внутри каждой группы — по убыванию сигнала (так их отдаёт
    `_dedupe_by_ssid`). Каждой записи добавляется флаг `iotm`, чтобы фронтенд мог
    пометить точки доступа модулей, не пересчитывая префикс заново.
    """
    out = []
    for n in networks:
        item = dict(n)
        item["iotm"] = n["ssid"].lower().startswith(IOTM_SSID_PREFIX)
        out.append(item)
    out.sort(key=lambda x: (not x["iotm"], -(x["signal"] or 0)))
    return out


def scan_networks(force_scan=False):
    """Видимые WiFi-сети: (список словарей, error). Пустой список — не ошибка.

    Перед чтением вывода netsh запускается активный поиск (WlanScan), иначе
    команда покажет устаревший кэш драйвера. force_scan=True — по требованию
    пользователя (кнопка «Обновить»), игнорирует паузу между сканированиями.
    """
    backend = wifi_backend()
    if backend == "netsh":
        trigger_scan(force=force_scan)
        rc, out = _run(["netsh", "wlan", "show", "networks", "mode=bssid"])
        if rc != 0:
            return [], ("netsh не вернул список сетей — нет WiFi-адаптера "
                        "или не запущена служба автонастройки WLAN")
        nets = parse_netsh_networks(out)
        if not nets:
            # На части сборок Windows режим mode=bssid отдаёт пусто — пробуем базовый вывод
            rc, out = _run(["netsh", "wlan", "show", "networks"])
            nets = parse_netsh_networks(out) if rc == 0 else []
        return _dedupe_by_ssid(nets), None
    if backend == "nmcli":
        rc, out = _run(["nmcli", "-t", "-f", "SSID,SIGNAL,CHANNEL,SECURITY",
                        "dev", "wifi", "list"])
        if rc != 0:
            return [], "nmcli вернул ошибку — NetworkManager недоступен"
        nets = []
        for line in out.splitlines():
            parts = line.split("\\:")      # в terse-режиме разделитель экранирован
            if len(parts) < 2:
                continue
            ssid = parts[0].strip()
            if not ssid:
                continue
            secured = bool(len(parts) > 3 and parts[3].strip())
            nets.append({
                "ssid": ssid,
                "signal": int(parts[1]) if parts[1].strip().isdigit() else None,
                "channel": int(parts[2]) if len(parts) > 2 and parts[2].strip().isdigit() else None,
                "secured": secured,
                "auth": parts[3].strip() if len(parts) > 3 else "",
                "bssid": "",
            })
        return _dedupe_by_ssid(nets), None
    return [], "В этой ОС управление WiFi не поддерживается (нужен Windows с netsh)"


def current_connection():
    """Текущее WiFi-подключение панели: {ssid, profile} (пустые — не подключены)."""
    res = {"ssid": "", "profile": ""}
    backend = wifi_backend()
    if backend == "netsh":
        rc, out = _run(["netsh", "wlan", "show", "interfaces"])
        if rc != 0:
            return res
        for line in out.splitlines():
            m = _IFACE_KEY_RE.match(line)
            if not m:
                continue
            key, val = m.group(1).lower(), m.group(2).strip()
            if key == "ssid" and not res["ssid"]:
                res["ssid"] = val
            elif key in ("profile", "профиль") and not res["profile"]:
                res["profile"] = val
    elif backend == "nmcli":
        rc, out = _run(["nmcli", "-t", "-f", "ACTIVE,SSID", "dev", "wifi", "list"])
        if rc == 0:
            for line in out.splitlines():
                parts = line.split("\\:")
                if len(parts) >= 2 and parts[0].strip().lower() == "yes":
                    res["ssid"] = parts[1].strip()
                    break
    return res


def local_ipv4():
    """IPv4-адреса всех интерфейсов панели (для проверки подсети модуля)."""
    try:
        import psutil
        addrs = []
        for addr_list in psutil.net_if_addrs().values():
            for addr in addr_list:
                if addr.family == socket.AF_INET:
                    addrs.append(addr.address)
        return addrs
    except Exception:                     # нет psutil/доступа к интерфейсам
        from core.devices import _get_local_ip
        return [_get_local_ip()]


# ==================== Подключение к сети ====================

_PROFILE_XML = """<?xml version="1.0"?>
<WLANProfile xmlns="http://www.microsoft.com/networking/WLAN/profile/v1">
    <name>{name}</name>
    <SSIDConfig>
        <SSID>
{hex_section}            <name>{name}</name>
        </SSID>
    </SSIDConfig>
    <connectionType>ESS</connectionType>
    <connectionMode>manual</connectionMode>
    <MSM>
        <security>
            <authEncryption>
                <authentication>{auth}</authentication>
                <encryption>{enc}</encryption>
                <useOneX>false</useOneX>
            </authEncryption>
{key_section}
        </security>
    </MSM>
</WLANProfile>
"""

# Современная схема WPA2-PSK: ключ задаётся элементом <sharedKey> (keyType
# passPhrase). Старый <keyM><key .../></keyM> для WPA2PSK недействителен —
# netsh отвергает профиль: «Недопустимый профиль для данной схемы. The network
# connection profile is corrupted» (0x80001).
_KEY_XML = """            <sharedKey>
                <keyType>passPhrase</keyType>
                <protected>false</protected>
                <keyMaterial>{password}</keyMaterial>
            </sharedKey>"""


def ssid_is_safe(ssid):
    """SSID можно передать в командную строку netsh (без кавычек и CR/LF)."""
    if not ssid or len(ssid) > MAX_SSID_LEN:
        return False
    if '"' in ssid or ssid != ssid.strip():
        return False
    return all(32 <= ord(c) and ord(c) != 127 for c in ssid)


def _write_profile(path, ssid, password):
    """Записывает XML WiFi-профиля (UTF-8 для ASCII, UTF-16 с BOM иначе)."""
    if password:
        auth, enc, key = "WPA2PSK", "AES", _KEY_XML.format(password=xml_escape(password))
    else:
        auth, enc, key = "open", "none", ""
    # Не-ASCII SSID Windows требует задать ещё и байтами в <hex> (802.11),
    # иначе профиль отвергается как некорректный.
    hex_section = ""
    if not ssid.isascii():
        hex_section = f"            <hex>{ssid.encode('utf-8').hex().upper()}</hex>\n"
    xml = _PROFILE_XML.format(name=xml_escape(ssid), auth=auth, enc=enc,
                              hex_section=hex_section, key_section=key)
    data = xml.encode("utf-8") if (ssid + password).isascii() else xml.encode("utf-16")
    with open(path, "wb") as f:
        f.write(data)


# ==================== Журнал WLAN: раннее распознавание сбоя подключения ========

# Событие 8002 («подключение не удалось») Windows пишет в журнал
# Microsoft-Windows-WLAN-AutoConfig/Operational сразу после отката ассоциации —
# обычно за 2–5 секунд, а не через CONNECT_WAIT. Опрос wevtutil во время ожидания
# позволяет показать причину (нет сети, неверный пароль) немедленно.
WLAN_LOG = "Microsoft-Windows-WLAN-AutoConfig/Operational"
_EVT_BLOCK_RE = re.compile(r"<Event\b.*?</Event>", re.S)
_EVT_DATA_RE = re.compile(r"<Data Name='([^']+)'>(.*?)</Data>", re.S)


def _decode_evt(raw):
    """Декодирует вывод wevtutil: UTF-8 → ANSI-кодировка Windows (ACP) → cp1251/cp866.

    В отличие от `_decode` (netsh печатает в OEM-кодировке), вывод wevtutil
    локализованные значения отдаёт в кодировке ANSI (ACP) — на русской Windows
    это cp1251, где OEM-декодирование cp866 даёт кракозябры.
    """
    if not raw:
        return ""
    if raw[:2] in (b"\xff\xfe", b"\xfe\xff"):
        try:
            return raw.decode("utf-16")
        except UnicodeDecodeError:
            pass
    encs = ["utf-8"]
    if os.name == "nt":
        try:
            import ctypes
            encs.append("cp" + str(ctypes.windll.kernel32.GetACP()))
        except Exception:
            pass
    for enc in encs + ["cp1251", "cp866"]:
        try:
            return raw.decode(enc)
        except (UnicodeDecodeError, LookupError):
            continue
    return raw.decode("utf-8", errors="replace")


def _wlan_query_xml(since_ts, count=8):
    """Последние count событий 8002 журнала WLAN, случившихся после since_ts.

    Возвращает (rc, текст XML) либо (-1, '') — журнал недоступен
    (нет wevtutil / права / не Windows): тогда ранний выход просто не работает.
    """
    if os.name != "nt" or not shutil.which("wevtutil"):
        return -1, ""
    # Формат SystemTime — ISO-8601 UTC, дробная часть в 100-нс (7 цифр) + «Z»
    iso = (datetime.fromtimestamp(since_ts, timezone.utc)
           .strftime("%Y-%m-%dT%H:%M:%S.%f") + "0Z")
    q = (f"*[System[(EventID={WLAN_FAIL_EVENT}) and "
         f"TimeCreated[@SystemTime>='{iso}']]]")
    try:
        p = subprocess.run(["wevtutil", "qe", WLAN_LOG, "/q:" + q,
                            "/c:" + str(count), "/f:xml", "/rd:true"],
                           capture_output=True, timeout=NETSH_TIMEOUT)
    except subprocess.TimeoutExpired:
        return -1, ""
    except Exception as e:                          # noqa: BLE001
        logger.debug(f"wevtutil не выполнен: {e}")
        return -1, ""
    return p.returncode, _decode_evt(p.stdout or b"")


def _wlan_connect_failure(since_ts, ssid=None):
    """Причина сбоя подключения к ssid после since_ts (событие 8002) или None.

    ssid=None — берёт любое событие после since_ts. None возвращает и при
    недоступном журнале: отсутствие данных — не ошибка ожидания.
    """
    rc, text = _wlan_query_xml(since_ts)
    if rc != 0 or not text:
        return None
    target = (ssid or "").strip().lower()
    for block in _EVT_BLOCK_RE.findall(text):
        if target:
            data = dict(_EVT_DATA_RE.findall(block))
            ev = ((data.get("SSID") or "").strip() or
                  (data.get("ProfileName") or "").strip())
            if ev.lower() != target:
                continue
        data = dict(_EVT_DATA_RE.findall(block))
        reason = xml_unescape((data.get("FailureReason") or "").strip())
        if reason:
            return reason
        return "Windows сообщает о сбое подключения"
    return None


def _wait_ssid(ssid, timeout=CONNECT_WAIT, since=None):
    """Ждёт, пока текущим WiFi-подключением станет ssid (опрос netsh/nmcli).

    Возвращает (ok, error): ok=False и error — текст причины, если журнал
    Windows уже сообщил о сбое подключения к этой сети (событие 8002); тогда
    ожидание прерывается досрочно, не расходуя весь CONNECT_WAIT. since —
    момент запуска `netsh wlan connect` (time.time()); без since журнал не
    опрашивается (совместимость со старыми вызовами).
    """
    deadline = time.time() + timeout
    next_evt = time.time() + WLAN_FAIL_POLL      # первый опрос не сразу: connect не успел упасть
    while time.time() < deadline:
        if current_connection().get("ssid") == ssid:
            return True, None
        if since is not None and time.time() >= next_evt:
            next_evt = time.time() + WLAN_FAIL_POLL
            reason = _wlan_connect_failure(since, ssid)
            if reason:
                return False, (f"Подключение к «{ssid}» не удалось: "
                               f"{reason}")
        time.sleep(1.0)
    if current_connection().get("ssid") == ssid:
        return True, None
    now = current_connection().get("ssid") or "нет подключения"
    return False, (f"Через {CONNECT_WAIT:.0f} сек активным не стало «{ssid}» "
                   f"(сейчас: {now})")


def connect(ssid, password="", progress=None):
    """Подключается к WiFi-сети: профиль (open или WPA2-PSK) + `netsh wlan connect`.

    progress(text) — необязательный коллбэк стадий для UI.
    """
    def stage(text):
        if progress:
            try:
                progress(text)
            except Exception:               # UI-коллбэк не должен ронять подключение
                pass

    backend = wifi_backend()
    if backend is None:
        return {"success": False, "error": "WiFi в этой ОС недоступен"}
    if backend != "netsh":
        return {"success": False,
                "error": "Автоподключение к WiFi реализовано для Windows (netsh)"}
    if not ssid_is_safe(ssid):
        return {"success": False, "error": f"Некорректное имя сети: {ssid!r}"}

    stage(f"Создание WiFi-профиля «{ssid}»")
    fd, tmp = tempfile.mkstemp(prefix="iotm_wifi_", suffix=".xml")
    os.close(fd)
    try:
        try:
            _write_profile(tmp, ssid, password)
        except Exception as e:
            return {"success": False, "error": f"Не удалось подготовить профиль: {e}"}
        rc, out = _run(f'netsh wlan add profile filename="{tmp}"', combine=True)
        if rc != 0:
            return {"success": False,
                    "error": f"Не удалось добавить WiFi-профиль: {out or 'netsh вернул ошибку'}"}
        stage(f"Ассоциация с «{ssid}»")
        t0 = time.time()                      # точка отсчёта для журнала WLAN (событие 8002)
        rc, out = _run(f'netsh wlan connect name="{ssid}" ssid="{ssid}"', combine=True)
        if rc != 0:
            return {"success": False,
                    "error": f"Не удалось подключиться к «{ssid}»: {out or 'netsh вернул ошибку'}"}
        stage("Ожидание подключения")
        ok, err = _wait_ssid(ssid, since=t0)
        if not ok:
            return {"success": False, "error": err}
        stage(f"Подключено к «{ssid}»")
        return {"success": True, "error": None}
    finally:
        try:
            os.unlink(tmp)
        except OSError:
            pass


# ==================== Поиск модуля в подсети точки доступа ====================


def _in_ap_subnet(addrs):
    """True, если один из адресов панели принадлежит подсети модуля."""
    for a in addrs:
        try:
            if ipaddress.ip_address(a) in AP_SUBNET:
                return True
        except ValueError:
            continue
    return False


def discover_ap_device(progress=None, gateway=AP_GATEWAY_IP, attempts=PING_ATTEMPTS):
    """Проверяет подсеть 192.168.4.0/24, пингует AP-адрес и добавляет устройство.

    Возвращает dict {success, in_ap_subnet, ping_ok, local_ips, device, error}.
    """
    def stage(text):
        if progress:
            try:
                progress(text)
            except Exception:
                pass

    addrs = local_ipv4()
    if not _in_ap_subnet(addrs):
        # Только реальные IPv4 (без loopback): полный список всех интерфейсов нечитаем.
        # 169.254.x.x — APIPA, значит DHCP не ответил вообще; перечислять их поимённо
        # незачем — показываем количество.
        others = [a for a in addrs if not a.startswith(("127.", "169.254."))]
        apipa = sum(1 for a in addrs if a.startswith("169.254."))
        shown = ", ".join(others) or "нет"
        if apipa:
            shown += f" (+{apipa} адрес(ов) APIPA 169.254.x.x — DHCP не отвечает)"
        return {"success": False, "in_ap_subnet": False, "ping_ok": False,
                "local_ips": addrs, "device": None,
                "error": f"Панель не получила адрес в подсети модуля {AP_SUBNET}: "
                         f"нужен 192.168.4.x, а у панели — {shown}. "
                         f"Точка доступа отвечает, но адрес по DHCP не выдал — "
                         f"возможно, подключились не к сети модуля"}
    stage(f"Ping {gateway}")
    ping_ok = False
    for i in range(attempts):
        if _host_pingable(gateway):
            ping_ok = True
            break
        if i + 1 < attempts:
            time.sleep(1.0)                 # DHCP/ассоциация могли не успеть
    if not ping_ok:
        return {"success": False, "in_ap_subnet": True, "ping_ok": False,
                "local_ips": addrs, "device": None,
                "error": f"{gateway} не отвечает на ping — модуль в подсети не найден"}
    stage(f"Чтение settings.json ({gateway})")
    res = add_device_by_ip(gateway, attempts=AP_IDENTITY_ATTEMPTS)
    if not res.get("success"):
        return {"success": False, "in_ap_subnet": True, "ping_ok": True,
                "local_ips": addrs, "device": None,
                "error": res.get("error") or f"Устройство по адресу {gateway} не распознано"}
    return {"success": True, "in_ap_subnet": True, "ping_ok": True,
            "local_ips": addrs, "device": res["device"], "error": None}


# ==================== Пароли AP, сохранённые в панели ====================


def stored_passwords():
    """Пароли точек доступа из проектов панели и папок устройств.

    Для модалки ввода пароля (кнопка 📁): проекты — myProfile.json каждого
    проекта (`iotmSettings.apssid/appass`) плюс корневой myProfile.json
    (проект «PlatformIO»); устройства — `settings.json` из RAM/ (fallback FS/)
    уже просканированных папок `tools/magicIoTm/devices/<folder>/`.

    Возвращает {"success": True, "projects": [...], "devices": [...]} —
    каждый элемент {label, ssid, pass}. Ошибка чтения отдельной записи
    записывается в logger и не роняет весь список.
    """
    from utils import projects as projects_util      # лениво: избежать цикла импортов

    projects, devices = [], []
    platformio = None

    # --- Проекты конфигуратора ---
    try:
        tree = projects_util.list_projects()
        for cat, names in sorted(tree.items()):
            for name in sorted(names):
                try:
                    cfg = projects_util.load_project_config(cat, name) or {}
                except (OSError, ValueError) as e:
                    logger.debug(f"Проект {cat}/{name}: myProfile.json не прочитан ({e})")
                    continue
                s = cfg.get("iotmSettings") or {}
                projects.append({"label": f"{cat}/{name}" if cat else name,
                                 "ssid": str(s.get("apssid") or ""),
                                 "pass": str(s.get("appass") or "")})
    except Exception as e:                          # noqa: BLE001
        logger.debug(f"Не удалось получить дерево проектов: {e}")

    # Корневой myProfile.json — проект «PlatformIO» (шаблон новых проектов).
    # Отдельным полем: фронт показывает его первой строкой, ДО группы «Проекты».
    try:
        with open(projects_util.ROOT_CONFIG_FILE, "r", encoding="utf-8") as f:
            pio = json.load(f).get("iotmSettings") or {}
        platformio = {"label": projects_util.PLATFORMIO_PROJECT,
                      "ssid": str(pio.get("apssid") or ""),
                      "pass": str(pio.get("appass") or "")}
    except (OSError, ValueError):
        pass

    # --- Папки устройств на диске ---
    with globals_._device_folders_lock:
        entries = [dict(v) for v in globals_._device_folders.values()]
    for e in sorted(entries, key=lambda x: str(x.get("folder") or "")):
        settings_path = None
        for d in (e.get("ram_dir"), e.get("fs_dir")):   # прошивка пишет в корень FS
            p = os.path.join(d, "settings.json") if d else None
            if p and os.path.isfile(p):
                settings_path = p
                break
        if not settings_path:
            continue
        try:
            with open(settings_path, "r", encoding="utf-8") as f:
                s = json.load(f)
        except (OSError, ValueError) as err:
            logger.debug(f"Устройство {e.get('folder')}: settings.json не прочитан ({err})")
            continue
        # label — имя папки устройства (в _device_folders.folder лежит полный путь)
        folder = str(e.get("folder") or "")
        devices.append({"label": os.path.basename(folder.rstrip("\\/")) or str(e.get("name") or ""),
                        "ssid": str(s.get("apssid") or ""),
                        "pass": str(s.get("appass") or "")})

    return {"success": True, "platformio": platformio,
            "projects": projects, "devices": devices}


# ==================== Домашняя сеть: куда возвращаться из AP модуля ====================


def remember_home(conn=None):
    """Запоминает «домашнюю» сеть панели — текущее подключение, если оно не AP модуля.

    Нужна для возврата после работы в точке доступа модуля (там адреса 192.168.4.x
    и обычная сеть недоступна). Вызывается при каждом скане и перед подключением к
    AP, поэтому «домашней» всегда оказывается последняя обычная сеть панели.
    """
    c = conn if conn is not None else current_connection()
    ssid = (c.get("ssid") or "").strip()
    if not ssid or ssid.lower().startswith(IOTM_SSID_PREFIX):
        return None                      # мы в AP модуля или вне сети — запоминать нечего
    home = {"ssid": ssid, "profile": (c.get("profile") or ssid).strip(), "ts": time.time()}
    with _wifi_scan_lock:
        _wifi_scan_state["home"] = home
    return home


def home_network():
    """Сохранённая «домашняя» сеть: {ssid, profile, ts} (ssid пустой — не сохранена)."""
    with _wifi_scan_lock:
        home = dict(_wifi_scan_state.get("home") or {})
    home["ssid"] = str(home.get("ssid") or "").strip()
    home["profile"] = str(home.get("profile") or home["ssid"]).strip()
    home["ts"] = float(home.get("ts") or 0.0)
    return home


def return_home(progress=None):
    """Возвращает панель в «домашнюю» сеть по сохранённому профилю (пароль не нужен)."""
    def stage(text):
        if progress:
            try:
                progress(text)
            except Exception:
                pass

    if wifi_backend() != "netsh":
        return {"success": False,
                "error": "Возврат в WiFi-сеть реализован для Windows (netsh)"}
    home = home_network()
    ssid, profile = home["ssid"], home["profile"]
    if not ssid:
        return {"success": False,
                "error": "Домашняя сеть неизвестна: панель должна хотя бы раз подключиться "
                         "к обычной сети — WiFi-скан запоминает её автоматически"}
    if not ssid_is_safe(ssid) or not ssid_is_safe(profile):
        return {"success": False, "error": f"Некорректное имя сети: {ssid!r}"}
    stage("Отключение от текущей сети")
    _run(["netsh", "wlan", "disconnect"])
    stage(f"Подключение к «{ssid}»")
    t0 = time.time()                          # точка отсчёта для журнала WLAN (событие 8002)
    rc, out = _run(f'netsh wlan connect name="{profile}" ssid="{ssid}"', combine=True)
    if rc != 0:
        return {"success": False,
                "error": f"Не удалось вернуться в «{ssid}»: {out or 'netsh вернул ошибку'}"}
    stage("Ожидание подключения")
    ok, err = _wait_ssid(ssid, since=t0)
    if not ok:
        return {"success": False, "error": err}
    stage(f"Подключено к «{ssid}»")
    return {"success": True, "error": None, "ssid": ssid}


# ==================== Поток подключения ====================


def _connect_worker(ssid, password, mode="connect"):
    """Тело фонового потока: подключение к AP модуля либо возврат в домашнюю сеть."""
    def progress(text):
        with _wifi_connect_lock:
            _wifi_connect_state["stage"] = text

    err, device = None, None
    try:
        if mode == "return":
            res = return_home(progress=progress)
            if not res.get("success"):
                err = res.get("error") or "Не удалось вернуться в домашнюю сеть"
        else:
            res = connect(ssid, password, progress=progress)
            if not res.get("success"):
                err = res.get("error") or "Не удалось подключиться"
            else:
                progress("Проверка подсети модуля")
                d = discover_ap_device(progress=progress)
                device = d.get("device")
                if not d.get("success"):
                    looks_like_ap = str(ssid or "").lower().startswith(IOTM_SSID_PREFIX)
                    if d.get("in_ap_subnet") is False and not looks_like_ap:
                        # Подключились к обычной сети (не AP модуля): адрес панели вне
                        # подсети 192.168.4.0/24 — это не ошибка, искать устройство негде
                        logger.info(f"Подключение к «{ssid}»: адрес панели вне подсети "
                                    f"модуля {AP_SUBNET} — устройство не добавляется")
                        device = None
                    else:
                        err = d.get("error")
                if device:
                    logger.info(f"Устройство добавлено из AP «{ssid}»: "
                                f"{device.get('name')} ({device.get('ip')})")
    except Exception as e:
        logger.error(f"Ошибка смены WiFi-сети «{ssid}» ({mode}): {e}", exc_info=True)
        err = str(e)
    finally:
        with _wifi_connect_lock:
            _wifi_connect_state.update({
                "running": False, "ssid": ssid, "stage": "",
                "error": err, "result": device,
            })
        globals_._wifi_connect_thread = None
        try:
            scan_once()                     # снимок должен отражать новую сеть
        except Exception:
            pass


def _start_wifi_thread(ssid, mode, password=""):
    """Общий запуск фоновой смены сети. False — поток уже занят."""
    with _wifi_connect_lock:
        if _wifi_connect_state["running"]:
            return False
        _wifi_connect_state.update({"running": True, "ssid": ssid, "mode": mode,
                                    "stage": "Ожидание запуска", "error": None,
                                    "result": None, "started": time.time()})
    globals_._wifi_connect_thread = threading.Thread(
        target=_connect_worker, daemon=True, args=(ssid, password, mode),
        name=f"wifi-{mode}")
    globals_._wifi_connect_thread.start()
    return True


def start_connect(ssid, password=""):
    """Подключение к AP модуля в отдельном потоке. False — смена сети уже идёт.

    Перед переключением запоминается домашняя сеть — из неё потом возвращаются
    кнопкой «🏠 Домашняя сеть».
    """
    remember_home()
    return _start_wifi_thread(ssid, "connect", password)


def start_return():
    """Возврат в «домашнюю» сеть в отдельном потоке. False — смена сети уже идёт."""
    return _start_wifi_thread(home_network()["ssid"], "return")


def connect_state():
    """Снимок состояния смены WiFi-сети (для опроса фронтендом)."""
    with _wifi_connect_lock:
        return dict(_wifi_connect_state)


# ==================== Периодическое сканирование эфира ====================


def scan_once(force=False):
    """Один цикл сканирования: эфир + текущее подключение → снимок состояния."""
    nets, err = scan_networks(force_scan=force)
    iotm = filter_iotm(nets)
    all_nets = order_networks(nets)      # все сети эфира, AP модулей — впереди
    conn_info = current_connection()
    conn = conn_info.get("ssid", "")
    remember_home(conn_info)             # запоминаем обычную сеть — для возврата из AP
    backend = wifi_backend()
    with _wifi_scan_lock:
        seen = set(_wifi_scan_state.get("seen") or [])
        found = {n["ssid"] for n in iotm}
        _wifi_scan_state.update({
            "supported": backend is not None,
            "backend": backend,
            "networks": iotm,
            "all": all_nets,             # все сети эфира (iotm* — впереди) для модалки
            "total": len(nets),          # сетей в эфире всего — для диагностики
            "new": sorted(found - seen),
            "connected": conn,
            "last_scan": time.time(),
            "scans": int(_wifi_scan_state.get("scans") or 0) + 1,
            "error": err,
        })
    if iotm:
        logger.info(f"WiFi-скан: найдено AP-сетей iotm*: {len(iotm)} — "
                    + ", ".join(n["ssid"] for n in iotm[:8]))
    else:
        # Нет AP модулей — важно отличать «модуль выключен/не в эфире» от
        # «скан не работает»: общее число сетей показывает, что эфир виден
        logger.info(f"WiFi-скан: AP-сетей iotm* не найдено "
                    f"(в эфире всего сетей: {len(nets)}"
                    + (f", например: {', '.join(n['ssid'] for n in nets[:4])}"
                       if nets else " — эфир пуст или скан не удался") + ")")
    return iotm


def mark_seen(ssids=None):
    """Снимает счётчик «новых» сетей: пользователь посмотрел список (кнопка «📶»)."""
    with _wifi_scan_lock:
        add = set(ssids) if ssids else {n["ssid"] for n in _wifi_scan_state["networks"]}
        history = list(dict.fromkeys(list(_wifi_scan_state.get("seen") or []) + sorted(add)))
        _wifi_scan_state["seen"] = history[-SEEN_LIMIT:]
        found = {n["ssid"] for n in _wifi_scan_state["networks"]}
        _wifi_scan_state["new"] = sorted(found - set(_wifi_scan_state["seen"]))


def ensure_first_scan():
    """Первый скан эфира сразу после старта панели (идемпотентно, один раз).

    Вызывается из get_state(): первый же запрос состояния (SSE-кадр или
    /devices/wifi) не должен ждать очередного цикла потока — иначе список сетей
    появился бы только через WIFI_SCAN_INTERVAL. Параллельные вызовы (SSE + GET)
    не запускают сканирование дважды — берём неблокирующий замок.
    """
    with _wifi_scan_lock:
        if _wifi_scan_state.get("scans") or _wifi_scan_state.get("last_scan"):
            return                                  # скан уже был
    if not _first_scan_lock.acquire(blocking=False):
        return                                      # скан уже выполняет другой поток
    try:
        with _wifi_scan_lock:
            done = bool(_wifi_scan_state.get("scans")
                        or _wifi_scan_state.get("last_scan"))
        if not done:
            logger.info("WiFi-скан: первый скан при старте панели")
            scan_once()
    finally:
        _first_scan_lock.release()


def get_state():
    """Снимок состояния WiFi-сканера (+ статус подключения) для API и SSE."""
    ensure_first_scan()                             # сети доступны сразу после старта
    with _wifi_scan_lock:
        st = {k: v for k, v in _wifi_scan_state.items() if k != "seen"}
        st["seen_count"] = len(_wifi_scan_state.get("seen") or [])
    st["connect"] = connect_state()
    st["alert"] = bool(st.get("networks"))
    st["new_count"] = len(st.get("new") or [])
    st["interval"] = WIFI_SCAN_INTERVAL
    st["next_scan"] = round(float(st.get("last_scan") or 0) + WIFI_SCAN_INTERVAL, 1)
    return st


def signature():
    """Подпись состояния для SSE-потока: отдаём только при изменениях."""
    st = get_state()
    c = st.get("connect") or {}
    h = st.get("home") or {}
    return (st.get("scans"), tuple(n["ssid"] for n in st.get("networks") or []),
            tuple(n["ssid"] for n in st.get("all") or []),
            tuple(st.get("new") or []), st.get("connected"), st.get("error"),
            st.get("supported"), c.get("running"), c.get("mode"), c.get("stage"),
            c.get("error"), (c.get("result") or {}).get("key"), h.get("ssid"))


def _scan_due():
    """Пора ли сканировать эфир: первый скан — сразу, дальше не чаще MIN_RESCAN_GAP.

    Смена WiFi-сети (connect/return) блокирует скан полностью — ассоциации нельзя
    мешать. Проверка `last_scan == 0` даёт немедленный первый скан и в потоке, если
    состояние ещё никто не запрашивал (первый клиент панели обычно запрашивает его
    раньше — тогда поток пропускает этот цикл, чтобы не сканировать дважды).
    """
    with _wifi_connect_lock:
        if _wifi_connect_state["running"]:
            return False
    with _wifi_scan_lock:
        last = float(_wifi_scan_state.get("last_scan") or 0.0)
    return (time.time() - last) >= MIN_RESCAN_GAP


def _wifi_worker():
    """Фоновый поток: поиск AP-сетей iotm* каждые WIFI_SCAN_INTERVAL секунд."""
    time.sleep(WIFI_FIRST_SCAN_DELAY)        # дать панели подняться (адаптеры, службы)
    while True:
        try:
            if _scan_due():
                scan_once()
        except Exception as e:
            logger.error(f"Ошибка цикла WiFi-сканирования: {e}")
        time.sleep(WIFI_SCAN_INTERVAL)


def start_wifi_worker():
    """Запускает периодическое WiFi-сканирование (идемпотентно)."""
    if globals_._wifi_thread and globals_._wifi_thread.is_alive():
        return
    globals_._wifi_thread = threading.Thread(target=_wifi_worker, daemon=True,
                                             name="wifi-scan-worker")
    globals_._wifi_thread.start()
    logger.info(f"WiFi-скан: поиск AP-сетей с префиксом «{IOTM_SSID_PREFIX}» "
                f"каждые {WIFI_SCAN_INTERVAL:.0f} сек "
                f"(подсистема: {wifi_backend() or 'недоступна'})")
