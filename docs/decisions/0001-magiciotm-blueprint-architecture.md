# ADR-0001: Декомпозиция app.py magicIoTm на blueprint'ы

- **Статус**: принято (коммит `f28e7cf1`, 2026-09-09)
- **Дата**: 2026-09-09
- **Контекст**

  `app.py` панели magicIoTm вырос в монолит (~1700 строк): Flask-маршруты,
  сетевой слой (multicast, ping, HTTP), глобальное состояние и фоновые
  потоки/SSE в одном файле. Аудит (авг 2026) отметил:
  - гонки данных по глобальному состоянию при `threaded=True` (нет единой блокировки);
  - сложность навигации и тестирования;
  - рекомендацию декомпозиции на модули `state/devices/network/metrics/catalog` и blueprints.

- **Решение**

  Разделить панель на слои:

  ```
  app.py      — точка входа: Flask, CORS, регистрация Blueprint'ов, init()
  state/      — глобальное состояние (globals.py: проект, кэши, устройства, блокировки)
  core/       — бизнес-логика (config, devices, builder, measurer, flasher, ota)
  routes/     — Flask Blueprint'ы (12: projects, config, modules, build, measure,
                platforms, tools, validation, upload, ota, scenario, devices)
  utils/      — реализация (subprocess, WS/HTTP-клиенты, esptool)
  static/     — фронтенд (index.html, favicon.png)
  ```

  HTTP-маршруты группируются по Blueprint'ам, сетевая логика устройств — в `core/devices.py`.

- **Следствия**

  - Плюсы: каждый маршрут легко найти; `core/devices.py` получил чистую функцию FSM
    `_next_state(...)` с юнит-тестами (`test_device_status_fsm.py`); снижена связанность.
  - Минусы: часть глобального состояния осталась в `state/globals.py` — гонки полностью
    не устранены, нужна единая блокировка (см. «Известные проблемы» в `AGENTS.md`).
  - Справочник API: [`docs/reference/magiciotm-api.md`](../reference/magiciotm-api.md).