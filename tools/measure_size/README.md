# measure_size

Скрипт замеряет прирост размера прошивки (flash) для каждого модуля и пишет в `about.size` в modinfo.json (КБ по env).

## Среда (venv)

```bash
cd measure_size
python3 -m venv venv
source venv/bin/activate   # Linux/macOS
# или: venv\Scripts\activate  # Windows
python measure.py --dry-run
```

## Запуск из корня IoTManager

```bash
# с виртуальным окружением из папки measure_size
measure_size/venv/bin/python measure_size/measure.py --dry-run
measure_size/venv/bin/python measure_size/measure.py --env esp32_4mb
```

Или из папки measure_size (после активации venv):

```bash
cd measure_size && source venv/bin/activate && python measure.py --dry-run
```
