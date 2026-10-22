import json, os, re, socket, struct, subprocess, time, threading, ipaddress, urllib.request
from concurrent.futures import ThreadPoolExecutor, as_completed
from flask import request, jsonify, Response
from utils import ws_client, ota

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
STATE_GREEN = "green"
STATE_YELLOW = "yellow"
STATE_RED = "red"
STATE_GREY = "grey"
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DEVICE_DIR_ROOT = os.path.normpath(os.path.join(BASE_DIR, "..", "devices"))
_ssid_cache = {"value": None, "ts": 0.0}
_SSID_TTL = 300