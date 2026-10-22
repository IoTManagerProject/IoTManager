"""
Measurer — логика замера размера модулей.

Извлечено из app.py для разделения ответственности.
"""

from utils import measure_run

# Re-export
start = measure_run.start
is_running = measure_run.is_running
event_stream = measure_run.event_stream
stop = measure_run.stop
