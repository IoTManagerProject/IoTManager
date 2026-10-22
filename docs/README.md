# Документация IoTManager

Индекс документации репозитория. Организована по модели **Diátaxis** — разные типы документов для разных задач.

| Раздел | Тип | Назначение |
|---|---|---|
| [architecture.md](architecture.md) | Explanation | Как устроена система: прошивка, панель, данные |
| [guides/build-firmware.md](guides/build-firmware.md) | How-to | Сборка прошивки и запуск панели |
| [guides/use-magiciotm.md](guides/use-magiciotm.md) | How-to | Работа с веб-панелью magicIoTm |
| [guides/add-module.md](guides/add-module.md) | How-to | Добавление нового модуля прошивки |
| [reference/magiciotm-api.md](reference/magiciotm-api.md) | Reference | Справочник API панели |
| [reference/platforms.md](reference/platforms.md) | Reference | Поддерживаемые платформы и окружения |
| [reference/data-formats.md](reference/data-formats.md) | Reference | Форматы данных: `data_svelte`, `myProfile.json` |
| [reference/device-status-fsm.md](reference/device-status-fsm.md) | Reference | Конечный автомат статусов устройств |
| [decisions/](decisions/) | Explanation | Архитектурные решения (ADR) |
| | [articles/](articles/) | Article | Рекламные и разъясняющие статьи |

Для AI-ассистентов есть отдельный контракт: [`AGENTS.md`](../AGENTS.md) (карта, команды, правила, риски).

## Как вести документацию

- Меняете API или поведение — обновите соответствующий справочник (`reference/`) и `CHANGELOG.md`.
- Крупное архитектурное решение — добавьте ADR в `docs/decisions/` (шаблон: `NNNN-kratkoe-nazvanie.md`, статус «предложено/принято/устарело»).
- Факт должен жить в одном месте; остальные файлы ссылаются на него.

## Roadmap

### Ближайшие задачи (из истории репозитория)

- [ ] Тесты для magicIoTm (юнит-тесты FSM уже есть: `test_device_status_fsm.py`)
- [ ] Автоматизация CI/CD сборки прошивок (`.github/workflows/build_iotm.yml` — только ручной запуск)
- [ ] Безопасность панели: аутентификация, закрыть path traversal и XSS, `debug=False`
- [ ] Таймауты subprocess (build/measure), атомарная запись, prune `_devices`/`_fetch_progress`
- [ ] Рефакторинг `index.html` (декомпозиция, CSP, вынос ресурсов)
- [ ] Документация: скриншоты панели, гайд по созданию собственного модуля

> Полный список технического долга (с приоритетами) — в секции «Известные проблемы и риски» файла `AGENTS.md`.