"""
Инициализация всех маршрутов приложения.

Импортирует все Blueprint из routes/*.py и регистрирует их.
"""

from routes.projects import bp as projects_bp
from routes.config import bp as config_bp
from routes.modules import bp as modules_bp
from routes.build import bp as build_bp
from routes.measure import bp as measure_bp
from routes.platforms import bp as platforms_bp
from routes.tools import bp as tools_bp
from routes.validation import bp as validation_bp
from routes.upload import bp as upload_bp
from routes.ota import bp as ota_bp
from routes.scenario import bp as scenario_bp
from routes.devices import devices_bp

__all__ = [
    "projects_bp",
    "config_bp",
    "modules_bp",
    "build_bp",
    "measure_bp",
    "platforms_bp",
    "tools_bp",
    "validation_bp",
    "upload_bp",
    "ota_bp",
    "scenario_bp",
    "devices_bp",
]
