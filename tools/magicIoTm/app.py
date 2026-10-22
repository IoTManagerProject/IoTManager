"""
MagicIoTm — Конфигуратор прошивок IoTmanager
Web-сервер на Flask для управления проектами и конфигурациями

Точка входа. Все маршруты зарегистрированы через Blueprint'ы в routes/.
"""

import logging
import os
from flask import Flask
from flask_cors import CORS

# ==================== Логирование ====================
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
LOG_FILE = os.path.join(BASE_DIR, 'magicIoTm.log')

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler(LOG_FILE, encoding='utf-8')
    ]
)
logger = logging.getLogger(__name__)

# ==================== Flask ====================
app = Flask(__name__)
CORS(app)
# Не сортировать ключи JSON в ответах jsonify. Иначе конфиг, отданный фронтенду
# (/api/config, /projects/<cat>/<name>/open и т.д.), возвращается обратно через
# /api/config/save (например, перед сборкой) и записывается в myProfile.json
# с алфавитным порядком полей вместо исходного.
app.json.sort_keys = False

# ==================== Регистрация Blueprint'ов ====================
from routes import (  # noqa: E402
    projects_bp,
    config_bp,
    modules_bp,
    build_bp,
    measure_bp,
    platforms_bp,
    tools_bp,
    validation_bp,
    upload_bp,
    ota_bp,
    scenario_bp,
    devices_bp,
)

# Статические маршруты (не /api)
app.add_url_rule('/', 'index', lambda: app.send_static_file('index.html'))
app.add_url_rule('/favicon.png', 'favicon',
                 lambda: app.send_static_file('favicon.png'))

# API Blueprint'ы
app.register_blueprint(projects_bp, url_prefix='/api')
app.register_blueprint(config_bp, url_prefix='/api')
app.register_blueprint(modules_bp, url_prefix='/api')
app.register_blueprint(build_bp, url_prefix='/api')
app.register_blueprint(measure_bp, url_prefix='/api')
app.register_blueprint(platforms_bp, url_prefix='/api')
app.register_blueprint(tools_bp, url_prefix='/api')
app.register_blueprint(validation_bp, url_prefix='/api')
app.register_blueprint(upload_bp, url_prefix='/api')
app.register_blueprint(ota_bp, url_prefix='/api')
app.register_blueprint(scenario_bp, url_prefix='/api')
app.register_blueprint(devices_bp, url_prefix='/api')

# ==================== Инициализация ====================

def init():
    logger.info("=" * 60)
    logger.info("MagicIoTm запускается...")
    logger.info("=" * 60)
    from utils.projects import ensure_dirs
    from core.config import load_platforms, scan_modinfo
    from core.devices import (  # noqa: E402
        _scan_device_folders,
        start_device_listener,
        start_ping_worker,
    )
    from core.wifi import start_wifi_worker
    from core.flasher import startup_check
    from utils import platformio_tools

    ensure_dirs()
    load_platforms()
    scan_modinfo()
    _scan_device_folders()
    start_device_listener()
    start_ping_worker()
    start_wifi_worker()           # поиск AP-сетей модулей (iotm*) в эфире
    startup_check()               # esptool: проверка + автоустановка при отсутствии
    platformio_tools.startup_check()  # PlatformIO: только проверка (установку предлагает UI)
    logger.info("Готов к работе")
    logger.info("=" * 60)


if __name__ == '__main__':
    init()
    logger.info("Сервер: http://127.0.0.1:5005")
    app.run(debug=True, host='127.0.0.1', port=5005, threaded=True, use_reloader=False)
