"""
Flash/esptool logic for MagicIoTm.

Re-exports public symbols from utils.flash and utils.esptool_tools
so that other core modules can import from a single place.
"""

from utils.flash import MODES, start, is_running, event_stream
from utils.esptool_tools import (
    status,
    ensure_installed,
    ensure_updated,
    list_esp_ports,
    family_of_env,
    expected_flash_from_env,
    detect_device,
    list_raw_ports,
    startup_check,
)

__all__ = [
    "MODES",
    "start",
    "is_running",
    "event_stream",
    "status",
    "ensure_installed",
    "ensure_updated",
    "list_esp_ports",
    "family_of_env",
    "expected_flash_from_env",
    "detect_device",
    "list_raw_ports",
    "startup_check",
]
