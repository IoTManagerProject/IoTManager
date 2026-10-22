#!/usr/bin/env python3
"""
Сниффер multicast UDP пакетов IoTManager.

Подключается к multicast группе 239.255.255.255:4210 и выводит в консоль
все приходящие пакеты (сырые байты + попытка разобрать JSON).

Устройства IoTManager рассылают broadcast-презентацию каждые 60 секунд
в формате JSON-массива, например:
    [{"ip":"192.168.1.50","wg":"main","name":"..."},...]

Запуск:
    python tools/udp_multicast_listener.py
    python tools/udp_multicast_listener.py --group 239.255.255.255 --port 4210
"""

import argparse
import json
import socket
import struct
import sys

DEFAULT_GROUP = "239.255.255.255"
DEFAULT_PORT = 4210


def parse_args():
    parser = argparse.ArgumentParser(description="Multicast UDP listener for IoTManager")
    parser.add_argument("--group", default=DEFAULT_GROUP,
                        help="multicast group address (default: %(default)s)")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT,
                        help="UDP port (default: %(default)s)")
    parser.add_argument("--raw", action="store_true",
                        help="show raw bytes in addition to decoded text")
    return parser.parse_args()


def join_multicast(group: str, port: int):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    # разрешить повторное использование адреса/порта (несколько снифферов одновременно)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    if hasattr(socket, "SO_REUSEPORT"):
        try:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
        except OSError:
            pass
    sock.bind(("", port))
    # вступление в multicast группу (для Windows/Unix используется struct без метки интерфейса)
    mreq = socket.inet_aton(group) + socket.inet_aton("0.0.0.0")
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    # таймаут чтения, чтобы можно было прервать Ctrl+C
    sock.settimeout(0.5)
    return sock


def decode_payload(data: bytes) -> str:
    """Пытается декодировать payload в читаемый текст (UTF-8/ASCII)."""
    for enc in ("utf-8", "latin-1"):
        try:
            return data.decode(enc)
        except UnicodeDecodeError:
            continue
    return repr(data)


def main():
    args = parse_args()
    print(f"Listening multicast {args.group}:{args.port} ... (Ctrl+C to stop)",
          file=sys.stderr)

    sock = join_multicast(args.group, args.port)
    try:
        while True:
            try:
                data, addr = sock.recvfrom(65535)
            except socket.timeout:
                continue
            except KeyboardInterrupt:
                break

            text = decode_payload(data)
            print(f"\n=== Packet from {addr[0]}:{addr[1]} "
                  f"({len(data)} bytes) ===")
            if args.raw:
                # показать hex-дамп полезной нагрузки
                print("RAW   :", " ".join(f"{b:02X}" for b in data))
            try:
                obj = json.loads(text)
                print("JSON  :", json.dumps(obj, ensure_ascii=False, indent=2))
            except (json.JSONDecodeError, ValueError):
                print("TEXT  :", text)
    finally:
        sock.close()
        print("\nListener stopped.", file=sys.stderr)


if __name__ == "__main__":
    main()